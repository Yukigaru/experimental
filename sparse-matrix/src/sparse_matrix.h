#pragma once
#include <unordered_map>
#include <cstdlib>
#include <type_traits>
#include "sparse_matrix_aux.h"

template <typename T, unsigned Dim, typename Alloc>
class SparseMatrix;

template <typename T, unsigned Dim, unsigned MaxDim, typename Alloc>
class MatrixSlice;

using aux::Position;

//////////////////////////////////////////////////////////
// Proxy object that refers to a cell in a N-dimension matrix
// It has all necessary operators for smooth using as just a reference to an array cell
template <typename T, unsigned MaxDim, typename Alloc>
class CellProxy {
    using SparseMatrixT = SparseMatrix<T, MaxDim, Alloc>;
    using PositionT = Position<MaxDim>;

public:
    CellProxy& operator =(T const& value) {
        if (value != matrix.getDefault())
            matrix.set(position, value);
        else
            matrix.clear(position);

        return *this;
    }

    CellProxy& operator =(T && value) {
        if (value != matrix.getDefault())
            matrix.set(position, std::forward<T>(value));
        else
            matrix.clear(position);

        return *this;
    }

    CellProxy& operator =(const CellProxy &other) {
	    *this = other.getCopy();
	    return *this;
    }

    explicit inline operator const T&() const {
        return getRef();
    }

    inline operator T() const {
        return getCopy();
    }

    inline bool operator ==(const CellProxy &other) {
        return getRef() == static_cast<T>(other);
    }

    inline bool operator !=(const CellProxy &other) {
        return !operator ==(other);
    }

    inline bool operator ==(const T& other) {
        return getRef() == other;
    }

    inline bool operator !=(const T& other) {
        return !(*this).operator ==(other);
    }

private:
    CellProxy(PositionT _position, SparseMatrixT& _matrix) noexcept :
        position(_position),
        matrix(_matrix)
    {
    }

    inline T getCopy() const {
        return matrix.get(position);
    }

    inline const T& getRef() const {
        return matrix.get(position);
    }

    PositionT position;
    SparseMatrixT &matrix; // ref to parent matrix

    friend class MatrixSlice<T, 1, MaxDim, Alloc>;
};

//////////////////////////////////////////////////////////
// Matrix slice of a certain dimension.
// For example: MatrixSlice<T, 1> is line-like slice of 2d matrix,
// MatrixSlice<T, 2> is a plane-like slice of 3d matrix and so on
template <typename T, unsigned Dim, unsigned MaxDim, typename Alloc>
class MatrixSlice {
    using CellProxyT = CellProxy<T, MaxDim, Alloc>;
    using SparseMatrixT = SparseMatrix<T, MaxDim, Alloc>;
    static constexpr unsigned PDim = MaxDim - Dim;

public:
    // returns cell proxy if it's the last dimension and i-th element is a cell
    template <unsigned D = Dim>
    typename std::enable_if<(D == 1), CellProxyT>::type operator [](size_t lastCoord) const {
        Position<PDim+1> position(coord, lastCoord);
        return CellProxyT(position, matrix);
    }

    // returns a slice of a lower dimension
    template <unsigned D = Dim>
    typename std::enable_if<(D != 1), MatrixSlice<T, Dim-1, MaxDim, Alloc>>::type operator [](size_t lastCoord) const {
        Position<PDim+1> position(coord, lastCoord);
        return MatrixSlice<T, Dim-1, MaxDim, Alloc>(position, matrix);
    }

private:
    MatrixSlice(Position<PDim> _coord, SparseMatrixT& _matrix) :
        coord(_coord),
        matrix(_matrix) {
    }

    Position<PDim> coord;
    SparseMatrixT &matrix;

    friend class SparseMatrix<T, MaxDim, Alloc>;
    friend class MatrixSlice<T, Dim+1, MaxDim, Alloc>;
};

//////////////////////////////////////////////////////////
template <typename T,
        unsigned Dim = 2,
        typename Alloc = std::allocator<std::pair<const Position<Dim>, T>>>
class SparseMatrix {
    static_assert(Dim >= 2, "Only N-dimension matrix allowed where N >= 2");

    using MatrixSliceT = MatrixSlice<T, Dim-1, Dim, Alloc>;
    using PositionT = Position<Dim>;

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;

    static constexpr bool is_nothrow_move = std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value;
    static constexpr bool is_nothrow_copy = std::is_nothrow_copy_constructible<T>::value && std::is_nothrow_copy_assignable<T>::value;

    SparseMatrix() noexcept(std::is_nothrow_default_constructible<T>::value) :
        deflt(aux::default_construct<T>()) {
    }

    explicit SparseMatrix(const T& _deflt) noexcept(std::is_nothrow_copy_constructible<T>::value):
        deflt(_deflt) {
    }

    SparseMatrix(const SparseMatrix& other) noexcept(is_nothrow_copy):
        cells(other.cells),
        deflt(other.deflt) {
    }

    SparseMatrix(SparseMatrix && other) noexcept(is_nothrow_move) {
        cells.swap(other.cells);
        std::swap(deflt, other.deflt);
    }

    SparseMatrix& operator =(const SparseMatrix& other) {
        cells = other.cells;
        deflt = other.deflt;
        return *this;
    }

    SparseMatrix& operator =(SparseMatrix && other) noexcept(is_nothrow_move) {
        cells.swap(other.cells);
        std::swap(deflt, other.deflt);
        return *this;
    }

    MatrixSliceT operator [](size_t firstCoord) {
        return MatrixSliceT(Position<1>({firstCoord}), *this);
    }

    const T& get(PositionT pos) {
        auto it = cells.find(pos);
        if (it != std::end(cells))
            return it->second;
        return deflt;
    }

    void set(PositionT pos, const T& value) {
        cells[pos] = value;
    }

    void set(PositionT pos, T&& rvalue) {
        cells[pos] = rvalue;
    }

    void clear(PositionT pos) {
        auto it = cells.find(pos);
        if (it != std::end(cells))
            cells.erase(it);
    }

    auto begin() noexcept {
        return std::begin(cells);
    }

    const auto begin() const noexcept {
        return std::begin(cells);
    }

    auto end() noexcept {
        return std::end(cells);
    }

    const auto end() const noexcept {
        return std::end(cells);
    }

    size_t size() const noexcept {
        return cells.size();
    }

    bool empty() const noexcept {
        return cells.empty();
    }

    const T& getDefault() const noexcept {
        return deflt;
    }

private:
    T deflt;
    std::unordered_map<PositionT, T, std::hash<PositionT>, std::equal_to<PositionT>, Alloc> cells;
};

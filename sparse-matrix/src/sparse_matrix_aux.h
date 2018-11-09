#pragma once
#include <array>
#include <type_traits>

//////////////////////////////////////////////////////////
// auxillary functionality for SparseMatrix
namespace aux {
    // A template function that default-constructs its type, if possible
    template<typename T>
    T default_construct() {
        return T(); // will fail
    }

    template<typename T>
    typename std::enable_if<std::is_default_constructible<T>::value, std::true_type>::type default_construct() {
        return T();
    }

    // N-dimension position. An array of numbers in its heart.
    template <unsigned Dim = 2>
    struct Position : public std::array<size_t, Dim> {
        Position(std::initializer_list<size_t> _lst) {
            size_t j = 0;
            for (auto i : _lst) {
                (*this)[j] = i;
                if (++j == this->size())
                    break;
            }
        }

        Position(Position<Dim-1> d, size_t lastCoord) {
            for (size_t i = 0; i < d.size(); i++)
                (*this)[i] = d[i];
            (*this)[Dim-1] = lastCoord;
        }
    };
}

// and its hasher function
namespace std {
    template <unsigned D>
    struct hash<aux::Position<D>> {
    private:
        template <class T>
        static void hash_combine(std::size_t & seed, const T & v) {
            std::hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

    public:
        size_t operator()(const aux::Position<D> &val) const noexcept {
            size_t seed = 0;
            for (size_t i = 0; i < val.size(); i++)
                hash_combine(seed, val[i]);
            return seed;
        }
    };
}
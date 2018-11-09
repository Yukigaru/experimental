#include <gtest/gtest.h>
#include <type_traits>
#include <algorithm>
#include "sparse_matrix.h"

TEST(SparseMatrix, Traits)
{
    struct A{
        A() = delete;
        A(int s) {}
    };
    //SparseMatrix<A> m; // compile error, not default constructible
    SparseMatrix<A> m(A(1)); // ok
}

TEST(SparseMatrix, Get)
{
    SparseMatrix<int> m(-1);
    ASSERT_TRUE(m[5][5] == -1 && m[5][5] == -1);

    m[0][0] = 1;
    ASSERT_TRUE(m[0][0] == 1 && m[0][0] == 1);
}

TEST(SparseMatrix, Set)
{
    SparseMatrix<int> m(-1);
    m[5][5] = 5;
    ASSERT_TRUE(m[5][5] == 5);
    ASSERT_TRUE(5 == m[5][5]);
    ASSERT_TRUE(m[5][4] == -1);
    ASSERT_TRUE(m[4][5] == -1);
    ASSERT_TRUE(m.size() == 1);
    ASSERT_TRUE(!m.empty());
}

TEST(SparseMatrix, MultipleSets)
{
    SparseMatrix<int> m(-1);
    ((m[100][100] = 314) = 0) = 217;

    ASSERT_TRUE(m[100][100] == 217);
    ASSERT_TRUE(m.size() == 1);
}

TEST(SparseMatrix, Remove)
{
    SparseMatrix<int> m(-1);
    m[0][0] = 5;
    m[0][1] = 5;
    m[1][0] = 5;
    m[0][0] = -1;
    ASSERT_TRUE(m[0][0] == -1 &&
        m[0][1] == 5 &&
        m[1][0] == 5);
    ASSERT_TRUE(m.size() == 2);
    ASSERT_TRUE(!m.empty());

    m[1][0] = -1;
    m[0][1] = -1;
    ASSERT_TRUE(m.size() == 0);
    ASSERT_TRUE(m.empty());
}

TEST(SparseMatrix, TransferValue)
{
    SparseMatrix<int> m1(0);
    SparseMatrix<int> m2(0);

    m1[1][1] = 5;
    m2[2][2] = m1[1][1];
    ASSERT_TRUE(m2[2][2] == 5);
    ASSERT_TRUE(m1[1][1] == 5);
}

TEST(SparseMatrix, Corner)
{
    SparseMatrix<int> m(0);
    m[-1][-1] = 5;
    ASSERT_TRUE(m[-1][-1] == 5);
    ASSERT_TRUE(m.size() == 1);
}

TEST(SparseMatrix, Many)
{
    constexpr size_t COUNT = 100000;
    SparseMatrix<int> m(-1);
    for (size_t x = 0; x < COUNT; x++)
        m[x][x] = x;

    for (size_t x = 0; x < COUNT; x++) {
        ASSERT_TRUE(m[x][x] == (int) x);
    }
    ASSERT_TRUE(m.size() == COUNT);
}

TEST(SparseMatrix, Iterator)
{
    SparseMatrix<int> m(-1);
    ASSERT_TRUE(m.size() == 0);
    ASSERT_TRUE(m.begin() == m.end());
    auto savedEnd = m.end();

    m[1][1] = 5;
    ASSERT_TRUE(m.begin() != m.end());
    ASSERT_TRUE(m.end() == savedEnd);
    ASSERT_TRUE(m.begin()->second == 5);

    m[2][2] = 10;
    auto i1 = m.begin();
    std::advance(i1, 2);
    ASSERT_TRUE(i1 == m.end());
}

TEST(SparseMatrix, Iterator2)
{
    constexpr size_t COUNT = 10;
    SparseMatrix<int> m(-1);

    std::vector<int> n;
    for (int i = 0; i < COUNT; i++) {
        n.push_back(i);
        m[i][i] = i;
    }

    for (auto i : m)
    {
        auto it = std::find(std::begin(n), std::end(n), i.second);
        ASSERT_TRUE(it != n.end());
        if (it != n.end())
            n.erase(it);
    }
    ASSERT_TRUE(n.empty());
}

TEST(SparseMatrix, NonPOD)
{
    struct A
    {
        A() = default;
        A(int _i) : i(_i) {}

        bool operator ==(const A& other) const { return i == other.i; }
        bool operator !=(const A& other) const { return !(*this == other); }
        int i{-1};
    };

    static_assert(!std::is_pod<A>::value, "");

    A deflt;
    SparseMatrix<A> m(deflt);

    A a(1);
    m[100][100] = a;
    ASSERT_TRUE(m[100][100] == a);
}

TEST(SparseMatrix, Assignment)
{
    SparseMatrix<int> m1(-1);
    m1[1][1] = 5;

    SparseMatrix<int> m2(1);
    m2 = m1;
    ASSERT_TRUE(m2[1][1] == 5);
    ASSERT_TRUE(!m1.empty());
    ASSERT_TRUE(m2.getDefault() == -1);
    ASSERT_TRUE(m1.getDefault() == -1);

    //
    SparseMatrix<int> m3(-1);
    m3[1][1] = 5;
    m3 = SparseMatrix<int>(-1);
    ASSERT_TRUE(m3.empty());
}

TEST(SparseMatrix, Ctors)
{
    SparseMatrix<int> m1(-1);
    m1[1][1] = 5;

    SparseMatrix<int> m2(m1);
    ASSERT_TRUE(m2[1][1] == 5);
    ASSERT_TRUE(m1[1][1] == 5);
    ASSERT_TRUE(m1.getDefault() == -1 && m2.getDefault() == -1);

    //
    SparseMatrix<int> m3((SparseMatrix<int>(-1))); // just check if it compiles
}

TEST(SparseMatrix, 3rdDim)
{
    SparseMatrix<int, 3> m(0);
    m[1][0][0] = 100;
    ASSERT_TRUE(m[1][0][0] == 100);
    ASSERT_TRUE(m.size() == 1 && !m.empty());
    ASSERT_TRUE(m[0][1][0] == 0 && m[0][0][1] == 0);

    m[1][1][1] = 200;
    ASSERT_TRUE(m[1][0][0] == 100 && m[1][1][1] == 200);
}

TEST(SparseMatrix, 4thDim)
{
    SparseMatrix<int, 4> m(0);
    m[1][2][3][4] = 101;
    ASSERT_TRUE(m[1][2][3][4] == 101);
    ASSERT_TRUE(m.size() == 1 && !m.empty());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include "sparse_matrix.h"
#include <iostream>

int main()
{
    SparseMatrix<int> m(0);

    for (int i = 0; i <= 9; i++) {
        m[i][i] = i;
        m[i][9-i] = 9 - i;
    }

    for (int y = 1; y <= 8; y++) {
        for (int x = 1; x <= 8; x++) {
            std::cout << ((x > 1) ? " " : "") << m[x][y];
        }
        std::cout << '\n';
    }

    std::cout << m.size() << '\n';

    for (const auto& it: m)
    {
        std::cout << "value '" << it.second << "' at " << it.first[0] << ", " << it.first[1] << ' ' << '\n';
    }

    return 0;
}
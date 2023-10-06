/**
 * @file Factorize.h
 * @author Arjun Earthperson
 * @date 10/06/2023
 * @brief TODO:: DOCUMENT
 *
 * This file contains the implementation of forward and backward substitution algorithms, which are part of the LU
 * decomposition method for solving a system of linear equations. The LU decomposition method involves decomposing a
 * given matrix (A) into a lower triangular matrix (L) and an upper triangular matrix (U), such that A = LU. Then,
 * forward elimination (forward substitution) and back substitution (backward substitution) are performed to solve the
 * system of linear equations.
 *
 * There are other approaches to performing forward and backward elimination, such as:
 *
 * 1. Gaussian Elimination: Involves performing row operations on an augmented matrix [A|b] to transform it into an
 *                          upper triangular matrix. Then, back substitution is performed to solve the system of
 *                          linear equations. Computational complexity: O(n^3). Numerical stability: Partial pivoting
 *                          can be used to improve stability.
 *
 * 2. Gauss-Jordan Elimination: An extension of Gaussian elimination. Involves performing row operations on an
 *                              augmented matrix [A|b] to transform it into a reduced row echelon form (RREF). The
 *                              resulting matrix directly provides the solution to the system of linear equations.
 *                              Computational complexity: O(n^3). Numerical stability: Similar to Gaussian elimination,
 *                              partial pivoting can be used to improve stability.
 *
 * 3. Cholesky Decomposition: Applicable to symmetric positive-definite matrices. Involves decomposing a given matrix
 *                            (A) into a lower triangular matrix (L) and its transpose (L^T), such that A = LL^T. Then,
 *                            forward and backward substitution are performed to solve the system of linear equations.
 *                            Computational complexity: O(n^3/3). Numerical stability: Stable for symmetric
 *                            positive-definite matrices.
 *
 * 4. QR Decomposition: Decomposes a given matrix (A) into an orthogonal matrix (Q) and an upper triangular
 *                      matrix (R), such that A = QR. Then, the system of linear equations is transformed into
 *                      Rx = Q^Tb, and back substitution is performed to solve the transformed system.
 *                      Computational complexity: O(n^3). Numerical stability: Stable, especially when using Householder
 *                      reflections or Gram-Schmidt orthogonalization.
 *
 * 5. Singular Value Decomposition (SVD): Decomposes a given matrix (A) into three matrices: an orthogonal matrix (U), a
 *                                        diagonal matrix (Σ), and another orthogonal matrix (V^T), such that A = UΣV^T.
 *                                        Then, the system of linear equations is transformed into a new system, and the
 *                                        solution is obtained using the inverse of the diagonal matrix. Computational
 *                                        complexity: O(n^3). Numerical stability: Stable, but computationally expensive.
 */

#ifndef NE591_008_FACTORIZE_H
#define NE591_008_FACTORIZE_H

// TODO:: DOCUMENT
namespace MyFactorizationMethod {

    // TODO:: DOCUMENT
    enum Type {
        METHOD_LU, ////< LU factorization
        METHOD_LUP, ////< LU factorization with pivoting
    };

    // TODO:: DOCUMENT
    const char *TypeKey(MyFactorizationMethod::Type value) {
        static const char *methodTypeKeys[] = {
                "LU",
                "LUP",
        };
        return methodTypeKeys[static_cast<int>(value)];
    }

    // TODO:: DOCUMENT
    template <typename T>
    struct Parameters {
        Parameters() = default;
        explicit Parameters(const size_t n) {
            L = MyBLAS::Matrix<T>(n, n, 1);
            U = MyBLAS::Matrix<T>(n, n, 0);
            P = MyBLAS::Matrix<T>(n, n, 1);
        };
        MyBLAS::Matrix<T> L;
        MyBLAS::Matrix<T> U;
        MyBLAS::Matrix<T> P;
    };

    // TODO:: DOCUMENT
    template <typename T>
    bool passesPreChecks(const MyBLAS::Matrix<T> &A, const MyBLAS::Vector<T> &b) {

        bool passes = true;

        if(!MyBLAS::isSquareMatrix(A)) {
            std::cerr<<"Error: Failed pre-check: coefficient matrix is non-square.\n";
            passes = false;
        }

        if(!MyBLAS::haveEqualRank(A, b)) {
            std::cerr<<"Error: Failed pre-check: coefficient and constant matrix not equal rank.\n";
            passes = false;
        }

        return passes;
    }

    // TODO:: DOCUMENT
    template <typename T>
    bool passesPostChecks(const MyBLAS::Matrix<T> &A, const MyBLAS::Matrix<T> &L, const MyBLAS::Matrix<T> &U, const MyBLAS::Matrix<T> &P) {

        bool passes = true;
        if(!MyBLAS::allDiagonalElementsBelowThreshold(U, 1e-20)) {
            std::cerr<<"Warning: upper triangular matrix leading diagonal contains values very close to 0, matrix maybe non-singular.\n";
            passes = false;
        }

        if(!MyBLAS::isUpperTriangularMatrix(U)) {
            std::cerr<<"Error: Failed post-check: upper triangular matrix not really upper triangular.\n";
            passes = false;
        }

        if(!MyBLAS::isUnitLowerTriangularMatrix(L)) {
            std::cerr<<"Error: Failed post-check: lower triangular matrix not really unit lower triangular.\n";
            passes = false;
        }

        if(!MyBLAS::isPermutationMatrix(P)) {
            std::cerr<<"Error: Failed post-check: assumed permutation matrix not really a permuted.\n";
            passes = false;
        }

        return passes;
    }

}
#endif //NE591_008_FACTORIZE_H

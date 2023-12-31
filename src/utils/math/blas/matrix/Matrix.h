/**
* @file Matrix.h
* @brief Header file for the MyBLAS::Matrix class.
* @author Arjun Earthperson
* @date 09/22/2023
* @details This file contains the declaration of the MyBLAS::Matrix class, which represents a matrix of T type values.
 */

#ifndef NE591_008_MATRIX_H
#define NE591_008_MATRIX_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include <unordered_set>

#include "math/blas/Constants.h"
#include "math/blas/vector/Vector.h"
#include "profiler/ResourceMonitor.h"

namespace MyBLAS {

template <typename T> class Matrix;
template <typename T> class Vector;

/**
 * @class Matrix
 * @brief Class representing a matrix of T type values.
 */
template <typename T = MyBLAS::NumericType> class Matrix {

  protected:
    std::vector<std::vector<T>> data; ///< 2D vector representing the matrix data.

  public:

    // TODO:: Document
    [[nodiscard]] size_t getAllocatedBytes(bool actual = false) const {
        auto nRows = actual ? data.size() : data.capacity();
        if (data.empty()) {
            return 0;
        }
        auto nCols = actual ? data[0].size() : data[0].capacity();

        assert(nRows >= getRows());
        assert(nCols >= getCols());

        size_t byteCount = 0;

        // count all the elements
        byteCount += (nRows * nCols) * (sizeof(T) > 0.0 ? sizeof(T) : 1);

        // count all the row containers
        byteCount += nRows * sizeof(std::vector<T>);

        // Memory used by this object itself.
        byteCount += sizeof(*this);

        return byteCount;
    }

    /**
     * @brief Default constructor. Initializes an empty matrix.
     */
    Matrix() {
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Default destructor.
     */
    ~Matrix() {
        ResourceMonitor<Matrix<T>>::unregisterInstance(this);
    }

    /**
     * @brief Constructor that initializes the matrix with a given 2D vector.
     * @param _data 2D vector to initialize the matrix with.
     */
    explicit Matrix(std::vector<std::vector<T>> &_data) : data(_data) {
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Const Constructor that initializes the matrix with a given 2D vector.
     * @param _data 2D vector to initialize the matrix with.
     */
    explicit Matrix(const std::vector<std::vector<T>> &_data) : data(_data) {
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the matrix with a given 2D vector.
     * @param _data 2D vector to initialize the matrix with.
     */
    explicit Matrix(std::vector<std::vector<T>>&& _data) : data(std::move(_data)) {
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Move constructor.
     * @param other The matrix to move from.
     */
    Matrix(Matrix&& other) noexcept : data(std::move(other.data)) {
        ResourceMonitor<Matrix<T>>::registerInstance(this);
        // Remove the moved-from instance from the tracking set.
    }

    /**
     * @brief Copy constructor.
     * @param other The matrix to copy from.
     */
    Matrix(const Matrix& other) : data(other.data) {
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the matrix with another matrix of a different type.
     * @param other The matrix to initialize from.
     */
    template <typename U>
    explicit Matrix(const Matrix<U>& other) {
        const size_t rows = other.getRows(), cols = other.getCols();
        data = std::vector<std::vector<T>>(rows, std::vector<T>(cols));
        #pragma omp parallel for simd default(none) shared(other)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp simd
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = static_cast<T>(other[i][j]);
            }
        }
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the matrix with another matrix.
     * @param matrix The matrix to initialize from.
     *
     * This constructor allows for initializing a Matrix from another Matrix or a compatible type.
     */
    template <class MatrixType>
    explicit Matrix(MatrixType matrix) {
        const size_t rows = matrix.getRows(), cols = matrix.getCols();
        data = std::vector<std::vector<T>>(rows, std::vector<T>(cols, 0));
        #pragma omp parallel for simd default(none) shared(matrix)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = matrix[i][j];
            }
        }
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Parameterized constructor that initializes the matrix with a given size and initial value.
     * @param _rows Number of rows in the matrix.
     * @param _cols Number of columns in the matrix.
     * @param _initial Initial value for all elements in the matrix.
     */
    Matrix(size_t _rows, size_t _cols, const T _initial = 0)
        : data(_rows, std::vector<T>(_cols, _initial)) {
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the matrix with a given initializer list.
     * @param initList Initializer list to initialize the matrix with.
     */
    Matrix(std::initializer_list<std::initializer_list<T>> initList) {
        data.resize(initList.size());
        size_t i = 0;
        for (const auto &row : initList) {
            data[i].resize(initList.begin()->size());
            size_t j = 0;
            for (const auto &elem : row) {
                data[i][j] = elem;
                ++j;
            }
            ++i;
        }
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the matrix with a given size and a lambda function.
     * @param rows Number of rows in the matrix.
     * @param cols Number of columns in the matrix.
     * @param func Lambda function to generate the elements of the matrix.
     */
    explicit Matrix(const size_t rows, const size_t cols, std::function<T(size_t, size_t)> func) : data(rows, std::vector<T>(cols)) {
        #pragma omp parallel for simd default(none) shared(func)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = func(i, j);
            }
        }
        ResourceMonitor<Matrix<T>>::registerInstance(this);
    }

    /**
     * @brief Copy assignment operator. Used to assign one Matrix to another after their initial construction.
     * @param other The matrix to copy from.
     */
    Matrix& operator=(const Matrix& other) {
        if (this != &other) {
            data = other.data;
            // Update the instances set
            // No need to update the instances set because 'this' already exists
        }
        return *this;
    }

    /**
     * @brief Move assignment operator. This operator is used to transfer the ownership of resources from one Matrix
     * to another without copying the data
     * @param other The matrix to copy from.
     */
    Matrix& operator=(Matrix&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            // No need to update the instances set because 'this' already exists
            // don't erase the moved-from object from the instances set since we are only moving the content, not the
            // instance. Clear the data since we use instance.data to compute allocated memory
            other.data.clear();
        }
        return *this;
    }

    /**
     * @brief Overloaded operator() to access individual elements of the matrix.
     * @param row Row index of the element to access.
     * @param col Column index of the element to access.
     * @return Reference to the element at the given row and column.
     */
    T &operator()(size_t row, size_t col) {
        assert(row < getRows() && col < getCols());
        return data[row][col];
    }

    /**
     * @brief Overloaded operator() const to access individual elements of the matrix.
     * @param row Row index of the element to access.
     * @param col Column index of the element to access.
     * @return Reference to the element at the given row and column.
     */
    const T &operator()(size_t row, size_t col) const {
        assert(row < getRows() && col < getCols());
        return data[row][col];
    }


    /**
     * @brief Overloaded operator[] to access individual rows of the matrix.
     * @param rowNum Index of the row to access.
     * @return Reference to the row at the given index.
     */
    std::vector<T> &operator[](const size_t rowNum) {
        assert(rowNum < getRows());
        return data[rowNum];
    }

    /**
     * @brief Overloaded operator[] to access individual rows of the matrix (const version).
     * @param rowNum Index of the row to access.
     * @return Const reference to the row at the given index.
     */
    const std::vector<T> &operator[](const size_t rowNum) const {
        assert(rowNum < getRows());
        return data[rowNum];
    }

    /**
     * @brief Getter for the matrix data.
     * @return Const reference to the matrix data.
     */
    [[nodiscard]] const std::vector<std::vector<T>> &getData() const { return data; }

    /**
     * @brief Getter for the vector data.
     * @return reference to the vector data.
     */
    [[nodiscard]] std::vector<std::vector<T>> &getData() { return data; }

    /**
     * @brief Getter for the vector data.
     * @return reference to the vector data.
     */
    [[nodiscard]] std::vector<std::vector<T>> &getMutableData() { return data; }

    /**
     * @brief Adds a new row to the end of the matrix.
     * @param row Vector representing the new row to be added.
     */
    void push_back(const std::vector<T> &row) {
        data.push_back(row);
    }

    [[nodiscard]]  /**
     * @brief Getter for the number of rows in the matrix.
     * @return Number of rows in the matrix.
                   */
    inline size_t getRows() const {
        return data.size();
    }

    [[nodiscard]]  /**
     * @brief Getter for the number of columns in the matrix.
     * @return Number of columns in the matrix.
                   */
    inline size_t getCols() const {
        return data.empty() ? 0 : data[0].size();
    }

    /**
     * @brief Static function to get the identity matrix of a given size.
     * @param size Size of the identity matrix.
     * @return Identity matrix of the given size.
     */
    static Matrix eye(const size_t size) {
        Matrix eye(size, size, 0);
        #pragma omp parallel for simd default(none) shared(eye)
        for (size_t i = 0; i < size; ++i) {
            eye[i][i] = 1;
        }
        return eye;
    }

    /**
     * @brief Overloaded operator+ to add two matrices.
     * @param rhs Matrix to add to the current matrix.
     * @return Resultant matrix after addition.
     */
    Matrix operator+(const Matrix &rhs) const {
        assert(getRows() == rhs.getRows());
        assert(getCols() == rhs.getCols());
        const size_t rows = getRows(), cols = getCols();
        Matrix result(rows, cols, 0);
        #pragma omp parallel for simd default(none) shared(result, rhs)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] = this->data[i][j] + rhs.data[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Overloaded operator- to subtract two matrices.
     * @param rhs Matrix to subtract from the current matrix.
     * @return Resultant matrix after subtraction.
     */
    Matrix operator-(const Matrix &rhs) const {
        assert(getRows() == rhs.getRows());
        assert(getCols() == rhs.getCols());
        const size_t rows = getRows(), cols = getCols();
        Matrix result(rows, cols, 0);
        #pragma omp parallel for simd default(none) shared(result, rhs)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] = this->data[i][j] - rhs.data[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Overloaded operator* to multiply two matrices.
     * @param rhs Matrix to multiply with the current matrix.
     * @return Resultant matrix after multiplication.
     */
    Matrix operator*(const Matrix &rhs) const {
        assert(getCols() == rhs.getRows());
        const size_t my_rows = getRows(), my_cols = getCols();
        const size_t rhs_cols = rhs.getCols();
        Matrix result(my_rows, rhs_cols, 0);
        #pragma omp parallel for simd default(none) shared(result, rhs)
        for (size_t i = 0; i < my_rows; ++i) {
            for (size_t j = 0; j < rhs_cols; ++j) {
                #pragma omp ordered simd
                for (size_t k = 0; k < my_cols; ++k) {
                    result[i][j] += this->data[i][k] * rhs.data[k][j];
                }
            }
        }
        return result;
    }

    /**
     * @brief Overloaded operator* to multiply two matrices, where one is of type MatrixType
     * @param rhs Matrix to multiply with the current matrix.
     * @return Resultant matrix after multiplication.
     */
    template <typename MatrixType1, typename MatrixType2>
    MatrixType1 operator*(const MatrixType2 &rhs) const {
        assert(getCols() == rhs.getRows());
        const size_t my_rows = getRows(), my_cols = getCols();
        const size_t rhs_cols = rhs.getCols();
        MatrixType1 result(my_rows, rhs_cols, 0);
        #pragma omp parallel for simd default(none) shared(result, rhs)
        for (size_t i = 0; i < my_rows; ++i) {
            for (size_t j = 0; j < rhs_cols; ++j) {
                #pragma omp ordered simd
                for (size_t k = 0; k < my_cols; ++k) {
                    result[i][j] += this->data[i][k] * rhs.data[k][j];
                }
            }
        }
        return result;
    }

    /**
     * @brief Overloaded operator* to multiply two matrices, where one is of type MatrixType
     * @param rhs Matrix to multiply with the current matrix.
     * @return Resultant matrix after multiplication.
     */
    template <typename MatrixType1, typename MatrixType2>
    MatrixType1 operator*(const MatrixType1 &rhs) const {
        assert(getCols() == rhs.getRows());
        const size_t my_rows = getRows(), my_cols = getCols();
        const size_t rhs_cols = rhs.getCols();
        MatrixType1 result(my_rows, rhs_cols, 0);
        #pragma omp parallel for simd default(none) shared(result, rhs)
        for (size_t i = 0; i < my_rows; ++i) {
            for (size_t j = 0; j < rhs_cols; ++j) {
                #pragma omp ordered simd
                for (size_t k = 0; k < my_cols; ++k) {
                    result[i][j] += this->data[i][k] * rhs.data[k][j];
                }
            }
        }
        return result;
    }

    /**
     * @brief Overloaded operator* to multiply a matrix with a vector.
     * @param rhs Vector to multiply with the current matrix.
     * @return Resultant vector after multiplication.
     */
    Vector<T> operator*(const Vector<T> &rhs) const {
        assert(getCols() == rhs.size());
        const size_t my_rows = getRows();
        const size_t my_cols = getCols();
        Vector<T> result(my_rows, 0);
        #pragma omp parallel for simd default(none) shared(result, rhs)
        for (size_t i = 0; i < my_rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < my_cols; ++j) {
                result[i] += this->data[i][j] * rhs[j];
            }
        }
        return result;
    }

    /**
     * @brief Sets a submatrix within the current matrix.
     * @param rowStart Starting row index for the submatrix.
     * @param colStart Starting column index for the submatrix.
     * @param subMatrix The submatrix to set.
     */
    void setSubMatrix(const size_t rowStart, const size_t colStart, const Matrix<T> &subMatrix) {
        const size_t subRows = subMatrix.getRows(), subCols = subMatrix.getCols();
        assert(rowStart + subRows <= getRows());
        assert(colStart + subCols <= getCols());
        #pragma omp parallel for simd default(none) shared(subMatrix)
        for (size_t i = 0; i < subRows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < subCols; ++j) {
                data[rowStart + i][colStart + j] = subMatrix[i][j];
            }
        }
    }

    /**
     * @brief Extracts a submatrix from the current matrix.
     * @param rowStart Starting row index for the submatrix.
     * @param colStart Starting column index for the submatrix.
     * @param subRows Number of rows in the submatrix.
     * @param subCols Number of columns in the submatrix.
     * @return The extracted submatrix.
     */
    Matrix<T> subMatrix(const size_t rowStart, const size_t colStart, const size_t subRows, const size_t subCols) const {
        assert(rowStart + subRows <= getRows());
        assert(colStart + subCols <= getCols());
        Matrix<T> subMatrix(subRows, subCols, 0);
        #pragma omp parallel for simd default(none) shared(subMatrix)
        for (size_t i = 0; i < subRows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < subCols; ++j) {
                subMatrix[i][j] = data[rowStart + i][colStart + j];
            }
        }

        return subMatrix;
    }

    /**
     * @brief Overloaded operator== to check for equality between two matrices.
     * @param rhs Matrix to compare with the current matrix.
     * @return True if the matrices are equal, false otherwise.
     *
     * @note This operator checks for exact equality of each element in the matrices.
     *       However, when dealing with floating-point numbers, due to the way they are
     *       represented in computers, two numbers that are theoretically equal may not
     *       be exactly equal when represented in a computer. Therefore, when dealing with
     *       floating-point numbers, it might be more appropriate to check if the difference
     *       between the matrices is below a certain tolerance, rather than checking for exact equality.
     */
    bool operator==(const Matrix &rhs) const {
        const size_t my_rows = getRows(), my_cols = getCols(), rhs_rows = rhs.getRows(), rhs_cols = rhs.getCols();
        if (my_rows != rhs_rows || my_cols != rhs_cols) {
            return false;
        }
        for (size_t i = 0; i < my_rows; ++i) {
            for (size_t j = 0; j < my_cols; ++j) {
                if (this->data[i][j] != rhs.data[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * @brief Overloaded operator* to multiply the matrix by a scalar.
     * @param scalar Scalar to multiply the matrix with.
     * @return Resultant matrix after multiplication.
     */
    Matrix operator*(const T &scalar) const {
        const size_t rows = getRows(), cols = getCols();
        Matrix result(rows, cols, 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] = this->data[i][j] * scalar;
            }
        }
        return result;
    }

    /**
     * @brief Friend function to overload the operator* for scalar * matrix.
     * @param scalar Scalar to multiply the matrix with.
     * @param m Matrix to be multiplied.
     * @return Resultant matrix after multiplication.
     */
    friend Matrix operator*(const T &scalar, const Matrix &m) {
        return m * scalar;
    }

    /**
     * @brief Overloaded operator/ to divide the matrix by a scalar.
     * @param scalar Scalar to divide the matrix by.
     * @return Resultant matrix after division.
     */
    Matrix operator/(const T &scalar) const {
        assert(scalar != 0);
        const size_t rows = getRows(), cols = getCols();
        Matrix result(rows, cols, 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] = this->data[i][j] / scalar;
            }
        }
        return result;
    }

    /**
     * @brief Overloaded operator+ to add a scalar to the matrix.
     * @param scalar Scalar to add to the matrix.
     * @return Resultant matrix after addition.
     */
    Matrix operator+(const T &scalar) const {
        const size_t rows = getRows(), cols = getCols();
        Matrix result(rows, cols, 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] = this->data[i][j] + scalar;
            }
        }
        return result;
    }

    /**
     * @brief Overloaded operator- to subtract a scalar from the matrix.
     * @param scalar Scalar to subtract from the matrix.
     * @return Resultant matrix after subtraction.
     */
    Matrix operator-(const T &scalar) const {
        const size_t rows = getRows(), cols = getCols();
        Matrix result(rows, cols, 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < rows; ++i) {
            #pragma omp ordered simd
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] = this->data[i][j] - scalar;
            }
        }
        return result;
    }

    /**
     * @brief Swaps two rows in the matrix.
     * @param row1 Index of the first row to swap.
     * @param row2 Index of the second row to swap.
     */
    void swapRows(size_t row1, size_t row2) {
        assert(row1 < getRows() && row2 < getRows());
        std::swap(data[row1], data[row2]);
    }

    /**
     * @brief Overloaded operator<< to print the matrix to an output stream.
     * @param os Output stream to print the matrix to.
     * @param m Matrix to print.
     * @return Reference to the output stream.
     */
    friend std::ostream &operator<<(std::ostream &os, const Matrix &m) {
        const auto width = static_cast<int>(std::cout.precision() + static_cast<std::streamsize>(10));
        const size_t rows = m.getRows(), cols = m.getCols();
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                os << std::setw(width) << std::setfill(' ') << std::scientific << static_cast<long double>(m[i][j]);
            }
            os << '\n';
        }
        return os;
    }

    /**
     * @brief Computes the element-wise product of two matrices.
     * @param A First Vector
     * @param B First Second
     * @return A new vector with the results.
     */
    static Matrix<T> elementwiseProduct(const Matrix<T> &A, const Matrix<T> &B) {
        assert(A.getRows() == B.getRows());
        assert(A.getCols() == B.getCols());
        const size_t a_rows = A.getRows(), b_cols = B.getCols();
        MyBLAS::Matrix<T> result(a_rows, b_cols);
        #pragma omp parallel for simd default(none) shared(result, A, B)
        for (size_t row = 0; row < a_rows; row++) {
            #pragma omp ordered simd
            for (size_t col = 0; col < b_cols; col++) {
                result[row][col] = A[row][col] * B[row][col];
            }
        }
        return result;
    }
};

/**
 * @brief Perform forward substitution
 *
 * This function performs forward substitution, which is used in solving a system of linear equations
 * after the system matrix has been decomposed into a lower triangular matrix (L) and an upper triangular matrix (U).
 * The forward substitution algorithm iterates through each row of the lower triangular matrix (L) and computes
 * the corresponding element in the intermediate result vector (y) by subtracting the sum of the product of the
 * current row elements and the corresponding elements in the intermediate result vector (y) from the corresponding
 * element in the input vector (b).
 *
 * @tparam T Using templates for data type consistency in computation.
 * @param L A lower triangular matrix.
 * @param b A vector in the system of linear equations Ax = b.
 * @return The result vector after performing forward substitution.
 */
template <template<typename> class M, template<typename> class V, typename T> V<T> forwardSubstitution(const M<T> &L, const V<T> &b) {
    static_assert(std::is_same<V<T>, MyBLAS::Vector<T>>::value, "M must be a MyBLAS::Matrix type");
    const auto n = b.size();
    auto y = V<T>(n);
    for (size_t row = 0; row < n; row++) {
        T sum = 0;
        for (size_t col = 0; col < row; col++) {
            sum += L[row][col] * y[col];
        }
        y[row] = (b[row] - sum);
    }
    return y;
}

/**
 * @brief Perform backward substitution
 *
 * This function performs backward substitution, which is used in solving a system of linear equations
 * after the system matrix has been decomposed into a lower triangular matrix (L) and an upper triangular matrix (U),
 * and forward substitution has been performed. The backward substitution algorithm iterates through each row of the
 * upper triangular matrix (U) in reverse order and computes the corresponding element in the solution vector (x) by
 * subtracting the sum of the product of the current row elements and the corresponding elements in the solution vector
 * (x) from the corresponding element in the intermediate result vector (y) and then dividing by the diagonal element of
 * the current row in the upper triangular matrix (U).
 *
 * @tparam T Using templates for data type consistency in computation.
 * @param U An upper triangular matrix.
 * @param y The result vector after performing forward substitution.
 * @return The solution vector after performing backward substitution.
 */
template <template<typename> class M, template<typename> class V, typename T> V<T> backwardSubstitution(const M<T> &U, const V<T> &y) {
    static_assert(std::is_same<M<T>, MyBLAS::Matrix<T>>::value, "M must be a MyBLAS::Matrix type");
    static_assert(std::is_same<V<T>, MyBLAS::Vector<T>>::value, "M must be a MyBLAS::Matrix type");
    const auto n = y.size();
    auto x = V<T>(n, static_cast<T>(0));
    for (int64_t i = n - 1; i >= 0; i--) {
        T sum = 0;
        for (auto j = static_cast<size_t>(i + 1); j < n; j++) {
            sum += U[i][j] * x[j];
        }
        x[i] = (y[i] - sum) / U[i][i];
    }
    return x;
}

/**
 * @brief Casts the elements of a given matrix to a specified type.
 *
 * This function creates a new matrix with the same dimensions as the input matrix and fills it with the
 * elements of the input matrix cast to the specified type.
 *
 * @tparam T The data type to cast the elements to.
 * @tparam T2 The original data type of the elements in the input matrix.
 * @param input The input matrix whose elements are to be cast.
 * @return A new matrix with the elements of the input matrix cast to the specified type.
 */
template <typename T, typename T2> static MyBLAS::Matrix<T> cast(const MyBLAS::Matrix<T2> &input) {
    MyBLAS::Matrix<T> output = Matrix<T>(input.getRows(), input.getCols(), 0);

    for (size_t row = 0; row < output.getRows(); row++) {
        for (size_t col = 0; col < output.getCols(); col++) {
            output[row][col] = static_cast<T>(input[row][col]);
        }
    }
    return output;
}

/**
 * @brief Casts the elements of a given vector to a specified type.
 *
 * This function creates a new vector with the same size as the input vector and fills it with the
 * elements of the input vector cast to the specified type.
 *
 * @tparam T The data type to cast the elements to.
 * @tparam T2 The original data type of the elements in the input vector.
 * @param input The input vector whose elements are to be cast.
 * @return A new vector with the elements of the input vector cast to the specified type.
 */
template <typename T, typename T2> static MyBLAS::Vector<T> cast(const MyBLAS::Vector<T2> &input) {
    MyBLAS::Vector<T> output = Vector<T>(input.size(), 0);
    for (size_t idx = 0; idx < output.size(); idx++) {
        output[idx] = static_cast<T>(input[idx]);
    }
    return output;
}
} // namespace MyBLAS

#endif // NE591_008_MATRIX_H

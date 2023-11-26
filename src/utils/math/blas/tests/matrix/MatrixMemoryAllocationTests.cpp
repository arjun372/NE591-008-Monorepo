/**
* @file MatrixMemoryAllocationTests.cpp
* @author Arjun Earthperson
* @date 10/13/2023
*
* @brief This file contains unit tests for the MyBLAS::Matrix class.
 */

#include "math/blas/matrix/Matrix.h"
#include <gtest/gtest.h>

#define TOLERANCE 1e-14
#define FLOAT_TYPE long double

namespace MyBLAS {

// Define a list of types to run the tests with
typedef ::testing::Types<int, float, double, long double> NumericTypes;
TYPED_TEST_SUITE(MatrixMemoryAllocationTests, NumericTypes);

template <typename T>
class MatrixMemoryAllocationTests : public ::testing::Test {
  protected:
    // Helper function to estimate the memory usage of a matrix with given dimensions.
    static size_t estimateMatrixMemoryUsage(size_t rows, size_t cols) {
        size_t memory = sizeof(Matrix<T>); // Memory for the Matrix object itself.
        memory += sizeof(std::vector<std::vector<T>>) + rows * sizeof(std::vector<T>); // Memory for the vector of vectors control structure.
        memory += rows * (sizeof(std::vector<T>) + cols * sizeof(T)); // Memory for the rows and their elements.
        return memory;
    }
};

TYPED_TEST(MatrixMemoryAllocationTests, ConstructorTest) {
    using MatrixType = Matrix<TypeParam>;
    const size_t rows = 5;
    const size_t cols = 5;
    const size_t matrixMemory = this->estimateMatrixMemoryUsage(rows, cols);

    // Create a matrix and check memory usage
    {
        MatrixType matrix(rows, cols);
        size_t memoryUsage = MatrixType::getTotalBytes();
        EXPECT_GT(memoryUsage, 0u);
        EXPECT_NEAR(static_cast<double>(memoryUsage), static_cast<double>(matrixMemory), static_cast<double>(matrixMemory) * 0.1f); // Allow 10% tolerance
    }

    // After matrix goes out of scope, memory usage should decrease
    {
        size_t memoryUsage = MatrixType::getTotalBytes();
        EXPECT_EQ(memoryUsage, 0u);
    }

    // Create multiple matrices and check memory usage
    {
        MatrixType matrix1(rows, cols);
        MatrixType matrix2(rows, cols);
        size_t memoryUsage = MatrixType::getTotalBytes();
        EXPECT_GT(memoryUsage, 0u);
        EXPECT_NEAR(static_cast<double>(memoryUsage), static_cast<double>(2 * matrixMemory), 2 * static_cast<double>(matrixMemory) * 0.1f); // Allow 10% tolerance
    }

    // After matrices go out of scope, memory usage should be zero
    {
        size_t memoryUsage = MatrixType::getTotalBytes();
        EXPECT_EQ(memoryUsage, 0u);
    }
}

} // namespace MyBLAS

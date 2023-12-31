/**
* @file Vector.h
* @brief Header file for the MyBLAS::Vector class.
* @author Arjun Earthperson
* @date 09/22/2023
* @details This file contains the declaration of the MyBLAS::Vector class, which represents a vector of T values.
*/

#ifndef NE591_008_VECTOR_H
#define NE591_008_VECTOR_H

#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <vector>

#include "LazyVector.h" // Include LazyVector for constructor and conversion from LazyVector
#include "profiler/ResourceMonitor.h"

namespace MyBLAS {

template <typename T> class LazyVector;

/**
* @class Vector
* @brief Class representing a vector of T values.
* This class represents a vector of elements of type T. It provides various operations for working with vectors.
* @tparam T The data type of the vector elements.
*/
template <typename T> class Vector {

  protected:
    std::vector<T> data; ///< Vector representing the vector data.
    bool isRow;          ///< Boolean indicating whether the vector is a row vector.

  public:

    // TODO:: DOCUMENT
    [[nodiscard]] size_t getAllocatedBytes(bool actual = false) const {
        auto nItems = actual ? data.size() : data.capacity();

        assert(nItems >= size());

        size_t byteCount = 0;

        // count all the elements
        byteCount += nItems * (sizeof(T) > 0.0 ? sizeof(T) : 1);

        // Memory used by this object itself.
        byteCount += sizeof(*this);

        return byteCount;
    }

    /**
     * @brief Default constructor. Initializes an empty column vector.
     */
    Vector() : isRow(false) {
        // ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    // TODO:: DOCUMENT
    ~Vector() {
        ResourceMonitor<Vector<T>>::unregisterInstance(this);
    }

    // TODO:: DOCUMENT
    Vector(const Vector& other) : data(other.data), isRow(other.isRow) {
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the vector with a given vector and row/column indicator.
     * @param _data Vector to initialize the vector with.
     * @param _isRow Boolean indicating whether the vector is a row vector.
     */
    explicit Vector(std::vector<T> &_data, bool _isRow) : data(_data), isRow(_isRow) {
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the vector with a given vector and row/column indicator.
     * @param _data Vector to initialize the vector with.
     * @param _isRow Boolean indicating whether the vector is a row vector.
     */
    explicit Vector(std::vector<T> _data, bool _isRow) : data(std::move(_data)), isRow(_isRow) {
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the vector with a given vector. The vector is assumed to be a column vector.
     * @param _data Vector to initialize the vector with.
     */
    explicit Vector(std::vector<T> &_data) : data(_data), isRow(false) {
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the vector with a given vector. The vector is assumed to be a column vector.
     * @param _data Vector to initialize the vector with.
     */
    explicit Vector(std::vector<T> _data) : data(_data), isRow(false) {
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
     * @brief Parameterized constructor that initializes the vector with a given size, initial value, and row/column
     * indicator.
     * @param size Size of the vector.
     * @param initial Initial value for all elements in the vector.
     * @param _isRow Boolean indicating whether the vector is a row vector.
     */
    explicit Vector(size_t size, const T initial = 0, bool _isRow = false) : data(size, initial), isRow(_isRow) {
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the vector with a given size and a lambda function.
     * @param size Size of the vector.
     * @param func Lambda function to generate the elements of the vector.
     */
    explicit Vector(const size_t size, std::function<T(size_t)> func) : data(size), isRow(false) {
        #pragma omp parallel for simd default(none) shared(func)
        for (size_t i = 0; i < size; i++) {
            data[i] = func(i);
        }
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
     * @brief Constructor that initializes the vector with an initializer list.
     * @param initList Initializer list to initialize the vector with.
     * @param _isRow Boolean indicating whether the vector is a row vector (default is false).
     */
    Vector(std::initializer_list<T> initList, bool _isRow = false) : data(initList), isRow(_isRow) {
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
    * @brief Constructor that initializes the vector with a LazyVector and a row/column indicator.
    * @param vector The LazyVector to initialize the vector with.
    * @param _isRow Boolean indicating whether the vector is a row vector.
     */
    explicit Vector(LazyVector<T> &vector, bool _isRow = false): data(vector.size()), isRow(_isRow) {
        #pragma omp parallel for simd default(none) shared(vector)
        for (size_t i = 0; i < vector.size(); i++) {
            data[i] = vector[i];
        }
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    /**
    * @brief Constructor that initializes the vector with a LazyVector and a row/column indicator.
    * @param vector The LazyVector to initialize the vector with.
    * @param _isRow Boolean indicating whether the vector is a row vector.
     */
    explicit Vector(LazyVector<T> vector, bool _isRow = false): data(vector.size()), isRow(_isRow) {
        #pragma omp parallel for simd default(none) shared(vector)
        for (size_t i = 0; i < vector.size(); i++) {
            data[i] = vector[i];
        }
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    // Constructor that initializes the vector with another vector of a different type.
    template <typename U>
    explicit Vector(const Vector<U>& other) {
        data.resize(other.size());
        #pragma omp parallel for simd default(none) shared(other)
        for (size_t i = 0; i < other.size(); ++i) {
            data[i] = static_cast<T>(other[i]);
        }
        isRow = other.isRowVector(); // Assuming Vector<U> has a method isRowVector()
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    // Constructor that initializes the vector with another vector or a compatible type.
    template <class VectorType>
    explicit Vector(VectorType vector) {
        data.resize(vector.size());
        #pragma omp parallel for simd default(none) shared(data, vector)
        for (size_t i = 0; i < vector.size(); ++i) {
            data[i] = vector[i];
        }
        isRow = vector.isRowVector(); // Assuming VectorType has a method isRowVector()
        ResourceMonitor<Vector<T>>::registerInstance(this);
    }

    // Copy assignment operator
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            data = other.data;
            isRow = other.isRow;
        }
        return *this;
    }

    // Move assignment operator
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            isRow = other.isRow;
            // Since we've moved from 'other', we may want to reset its state as appropriate
            other.isRow = false; // or true, depending on what you consider a default state
        }
        return *this;
    }

    /**
     * @brief Getter for the vector data.
     * @return Const reference to the vector data.
     */
    [[nodiscard]] const std::vector<T> &getData() const { return data; }

    /**
     * @brief Getter for the vector data.
     * @return reference to the vector data.
     */
    [[nodiscard]] std::vector<T>& getData() { return data; }

    /**
     * @brief Getter for the size of the vector.
     * @return Size of the vector.
     */
    [[nodiscard]] size_t size() const { return data.size(); }

    /**
     * @brief Returns whether this vector is a row vector.
     * @return true if row vector, false otherwise
     */
    [[nodiscard]] bool isRowVector() const {
        return isRow;
    }

    /**
     * @brief Overloaded operator[] to access individual elements of the vector.
     * @param idx Index of the element to access.
     * @return Reference to the element at the given index.
     */
    T &operator[](const size_t idx) { return data[idx]; }

    /**
     * @brief Overloaded operator[] to access individual elements of the vector (const version).
     * @param idx Index of the element to access.
     * @return Const reference to the element at the given index.
     */
    const T &operator[](const size_t idx) const { return data[idx]; }

    /**
     * @brief Overloaded operator== to check for equality between two vectors.
     * @param rhs Vector to compare with the current vector.
     * @return Boolean indicating whether the two vectors are equal.
     *
     * @note When dealing with floating-point numbers, due to precision issues, two vectors that are theoretically equal
     * may not be exactly equal. In such cases, it might be more appropriate to check if the difference between the
     * vectors is below a certain tolerance, rather than checking for exact equality.
     */
    bool operator==(const Vector &rhs) const {
        if (data.size() != rhs.size()) {
            return false;
        }
        for (size_t i = 0; i < size(); ++i) {
            if (this->data[i] != rhs.data[i]) {
                return false;
            }
        }
        return true;
    }

    /**
    * @brief Overloaded operator!= to check for inequality between two vectors.
    * @param rhs Vector to compare with the current vector.
    * @return Boolean indicating whether the two vectors are not equal.
    */
    bool operator!=(const Vector &rhs) const {
        return !(*this==rhs);
    }

    /**
     * @brief Overloaded operator+ to add two vectors.
     * @param rhs Vector to add to the current vector.
     * @return Resultant vector after addition.
     */
    Vector operator+(const Vector &rhs) const {
        assert(data.size() == rhs.size());
        Vector result(size(), 0);
        #pragma omp parallel for simd default(none) shared(result, rhs)
        for (size_t i = 0; i < size(); ++i) {
            result[i] = this->data[i] + rhs.data[i];
        }
        return result;
    }

    /**
     * @brief Overloaded operator- to subtract two vectors.
     * @param rhs Vector to subtract from the current vector.
     * @return Resultant vector after subtraction.
     */
    Vector operator-(const Vector &rhs) const {
        assert(data.size() == rhs.size());
        Vector result(size(), 0);
        #pragma omp parallel for simd default(none) shared(rhs, result)
        for (size_t i = 0; i < size(); ++i) {
            result[i] = this->data[i] - rhs.data[i];
        }
        return result;
    }

    /**
     * @brief Overloaded operator* to multiply two vectors (dot product).
     * @param rhs Vector to multiply with the current vector.
     * @return Resultant scalar after multiplication.
     */
    T operator*(const Vector &rhs) const {
        assert(data.size() == rhs.size());
        T result = 0;
        #pragma omp parallel for simd reduction(+:result) default(none) shared(rhs)
        for (size_t i = 0; i < size(); ++i) {
            result += this->data[i] * rhs.data[i];
        }
        return result;
    }

    /**
     * @brief Overloaded operator/ perform element-wise division between two vectors.
     * @param rhs Vector to subtract from the current vector.
     * @return Resultant vector after subtraction.
     */
    Vector operator/(const Vector &divisor) const {
        assert(data.size() == divisor.size());
        Vector result(size(), 0);
        for (size_t i = 0; i < size(); ++i) {
            if (divisor.data[i] == static_cast<T>(0)) {
                throw std::invalid_argument("attempting to divide by zero at index: "+std::to_string(i));
            }
            result[i] = this->data[i] / divisor.data[i];
        }
        return result;
    }

    /**
     * @brief Overloaded operator/ perform element-wise division between two vectors.
     * @param rhs Vector to subtract from the current vector.
     * @return Resultant vector after subtraction.
     */
    Vector& operator/=(const Vector &divisor) const {
        assert(data.size() == divisor.size());
        for (size_t i = 0; i < size(); ++i) {
            if (divisor.data[i] == static_cast<T>(0)) {
                throw std::invalid_argument("attempting to divide by zero at index: "+std::to_string(i));
            }
            this->data[i] = this->data[i] / divisor.data[i];
        }
        return *this;
    }

    /**
     * @brief Overloaded operator* to multiply a vector with a scalar.
     * @param scalar Scalar to multiply with the vector.
     * @return Resultant vector after multiplication.
     */
    Vector<T> operator*(const T &scalar) const {
        Vector<T> result(size(), 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < size(); ++i) {
            result[i] = data[i] * scalar;
        }
        return result;
    }

    /**
     * @brief Overloaded operator/ to divide a vector by a scalar.
     * @param scalar Scalar to divide the vector by.
     * @return Resultant vector after division.
     */
    Vector<T> operator/(const T &scalar) const {
        assert(scalar != 0);
        Vector<T> result(size(), 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < size(); ++i) {
            result[i] = data[i] / scalar;
        }
        return result;
    }

    /**
     * @brief Overloaded operator+ to add a scalar to a vector.
     * @param scalar Scalar to add to the vector.
     * @return Resultant vector after addition.
     */
    Vector<T> operator+(const T &scalar) const {
        Vector<T> result(size(), 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < size(); ++i) {
            result[i] = data[i] + scalar;
        }
        return result;
    }

    /**
     * @brief Overloaded operator- to subtract a scalar from a vector.
     * @param scalar Scalar to subtract from the vector.
     * @return Resultant vector after subtraction.
     */
    Vector<T> operator-(const T &scalar) const {
        Vector<T> result(size(), 0);
        #pragma omp parallel for simd default(none) shared(result, scalar)
        for (size_t i = 0; i < size(); ++i) {
            result[i] = data[i] - scalar;
        }
        return result;
    }

    /**
     * @brief Overloaded operator<< to print the vector to an output stream.
     * @param os Output stream to print the vector to.
     * @param m Vector to print.
     * @return Reference to the output stream.
     */
    friend std::ostream &operator<<(std::ostream &os, const Vector &m) {
        const auto width = static_cast<int>(std::cout.precision() + static_cast<std::streamsize>(10));
        for (size_t i = 0; i < m.size(); ++i) {
            os << std::setw(width) << std::setfill(' ') << std::scientific << static_cast<long double>(m[i]);
        }
        os << '\n';
        return os;
    }

    /**
     * @brief Computes the element-wise product of two vectors.
     * @param A First Vector
     * @param B First Second
     * @return A new vector with the results.
     */
    static Vector<T> elementwiseProduct(const Vector<T> &A, const Vector<T> &B) {
        assert(A.size() == B.size());
        const auto size = A.size();
        MyBLAS::Vector<T> result(size);
        #pragma omp parallel for simd default(none) shared(result, A, B)
        for (size_t i = 0; i < size; i++) {
            result[i] = A[i] * B[i];
        }
        return result;
    }

  public:
    // Type aliases for iterator and const_iterator
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    // Begin iterator
    iterator begin() { return data.begin(); }
    const_iterator begin() const { return data.begin(); }
    const_iterator cbegin() const { return data.cbegin(); }

    // End iterator
    iterator end() { return data.end(); }
    const_iterator end() const { return data.end(); }
    const_iterator cend() const { return data.cend(); }
};

} // namespace MyBLAS

#endif // NE591_008_VECTOR_H

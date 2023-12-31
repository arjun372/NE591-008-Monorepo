/**
 * @file function.h
 * @author Arjun Earthperson
 * @date 09/08/2023
 * @brief This file contains the declaration of a user-defined function and a function to fill a vector with the values
 * of the user-defined function for each value in another vector.
 */

#pragma once

#include <iostream>
#include <vector>

/**
 * @brief A user-defined function that calculates the value of the function f(x) = e^(x)
 * @tparam T The type of the input and output. This should be a numeric type (e.g., int, float, double).
 * @param x_ The input to the function.
 * @return The value of the function at the input x.
 */
template <typename T> [[maybe_unused]] static inline T user_defined_fx(const T x_) {
    const T fx = std::sin(x_);
    return fx;
}

/**
 * @brief Fills a vector with the values of the user-defined function for each value in another vector.
 * @tparam T The type of the elements in the vectors. This should be a numeric type (e.g., int, float, double).
 * @param x_ A vector of _inputs to the function.
 * @param fx A vector that will be filled with the function's values at the _inputs in the x_ vector.
 */
template <typename T> [[maybe_unused]] static void fill_fx(std::vector<T> &x_, std::vector<T> &fx) {

    fx.clear();
    fx.resize(x_.size());

    // Use std::transform with a lambda function to evaluate fx for each x_ value
    std::transform(x_.begin(), x_.end(), fx.begin(), [](T x) { return user_defined_fx(x); });
}

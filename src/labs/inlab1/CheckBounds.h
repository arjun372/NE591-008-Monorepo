/**
 * @file CheckBounds.h
 * @brief This file contains the declaration of functions for checking the validity of input parameters.
 * @date 08/30/2023
 * @author Arjun Earthperson
 */

#pragma once

#include <iostream>
#include <boost/program_options.hpp>

#include "utils/CommandLine.h"

/**
 * @brief Checks if the absolute value of the input is greater than or equal to 1.
 *
 * @param value The input value to be checked.
 * @return true If the absolute value of the input is greater than or equal to 1.
 * @return false If the absolute value of the input is less than 1.
 */
static bool failsCheck1(double_t value) {
    const double_t max = 1.0f;
    if (abs(value) >= max) {
        std::cerr <<"Error: " << "abs(x) is greater than " << std::setprecision (19) << max << "\n";
        return true;
    }
    return false;
}

/**
 * @brief Checks if the input value is less than or equal to 0.
 *
 * @param value The input value to be checked.
 * @return true If the input value is less than or equal to 0.
 * @return false If the input value is greater than 0.
 */
static bool failsCheck2(double_t value) {
    const double_t min = 0.0f;
    if (value <= min) {
        std::cerr <<"Error: " << "error threshold is not greater than " << std::setprecision (19) << min << "\n";
        return true;
    }
    return false;
}

/**
 * @brief Checks if the input value is a positive natural number.
 *
 * @param value The input value to be checked.
 * @return true If the input value is not a positive natural number.
 * @return false If the input value is a positive natural number.
 */
static bool failsCheck3(double_t value) {
    const double_t min = 0;
    auto error = false;
    if (value <= min) {
        std::cerr <<"Error: " << "number of iterations should be a positive number\n";
        error = true;
    }
    if (ceil(value) != floor(value)) {
        std::cerr <<"Error: " << "number of iterations should be a natural number\n";
        error = true;
    }

    if (value == 0) {
        std::cerr <<"Error: " << "number of iterations cannot be zero\n";
        error = true;
    }

    return error;
}

/**
 * @brief Performs input checks on the values of angle, convergence-threshold, and iterations.
 *
 * If the input value fails the check, the function will prompt the user to enter a new value.
 * The function will continue to prompt the user until a valid value is entered.
 *
 * @param values A map containing the input values to be checked.
 */
static void performInputChecks(boost::program_options::variables_map &values) {

    while(values["angle"].empty() || failsCheck1(values["angle"].as<double_t>())) {
        std::cout<<"Enter a value for the angle x [radian]: ";
        double_t input;
        std::cin >> input;
        replace(values, "angle", input);
    }

    while(values["convergence-threshold"].empty() || failsCheck2(values["convergence-threshold"].as<double_t>())) {
        std::cout<<"Enter a value for the stopping criterion e [e > 0]: ";
        double_t input;
        std::cin >> input;
        replace(values, "convergence-threshold", input);
    }

    while(values["iterations"].empty() || failsCheck3(values["iterations"].as<double_t>())) {
        std::cout<<"Enter maximum number of iterations (natural number): ";
        double_t input;
        std::cin >> input;
        replace(values, "iterations", input);
    }
}

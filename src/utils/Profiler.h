/**
 * @file Profiler.h
 * @author Arjun Earthperson
 * @date 08/30/2023
 * @brief This file contains helper classes and functions for profiling and random number generation.
*/

/*
 * MIT License
 *
 * Copyright (c) 2023 Luigi Capogrosso, Luca Geretti,
 *                    Marco cristani, Franco Fummi, and Tiziano Villa.
 *
 * Copyright (c) 2023 Arjun Earthperson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <iomanip>
#include <iostream>
#include <functional>

#include "Stopwatch.h"

/**
 * @brief A struct that provides a static method for generating random double numbers within a given range.
 */
struct Randomiser
{
    static double get(double min, double max)
    {
        // TODO: rand() has limited randomness; use C++11 random library.
        return ((max - min) * rand() / RAND_MAX + min);
    }
};

/**
 * @brief Initializes the random number generator with the current time as the seed.
 * @return Always returns true.
 */
inline bool _init_randomiser()
{
    srand(time(nullptr));

    return true;
}

/**
 * @brief A global constant that ensures the random number generator is initialized when the program starts.
 */
static const bool init_randomiser = _init_randomiser();

/**
 * @brief A class that provides methods for profiling the execution time of functions.
 */
class Profiler
{
public:
    /**
     * @brief Constructs a new Profiler object.
     * @param num_tries The number of times to execute the function for profiling.
     */
    Profiler(int num_tries) : _num_tries(num_tries) { }

    /**
     * @brief Returns the number of times the function will be executed for profiling.
     * @return The number of tries.
     */
    int num_tries() const { return _num_tries; }

    /**
     * @brief Returns a reference to the Randomiser object.
     * @return A const reference to the Randomiser object.
     */
    [[nodiscard]] Randomiser const& rnd() const { return _rnd; }

    /**
     * @brief Profiles the execution time of a function on average.
     * @param msg The message to print before the profiling result.
     * @param function The function to profile.
     * @param num_tries The number of times to execute the function for profiling.
     */
    void profile_on_average(std::string msg,
                            std::function<void(int)> function,
                            int num_tries)
    {
        _ussw.restart();
        for (int i = 0; i < num_tries; ++i) function(i);
        _ussw.click();
        std::cout << msg << " completed in "
                  << ((double)_ussw.duration().count())/num_tries*1000
                  << " us on average" << std::endl;
    }

    /**
     * @brief Profiles the total execution time of a function.
     * @param msg The message to print before the profiling result.
     * @param function The function to profile.
     * @param num_tries The number of times to execute the function for profiling.
     */
    void profile_on_total(std::string msg,
                          std::function<void(int)> function,
                          int num_tries)
    {
        _mssw.restart();
        for (int i = 0; i < num_tries; ++i) function(i);
        _mssw.click();
        std::cout << msg << " completed in "
                  << ((double)_mssw.duration().count())
                  << " ms on total" << std::endl;
    }

    /**
     * @brief Profiles the execution time of a function on average using the number of tries specified in the constructor.
     * @param msg The message to print before the profiling result.
     * @param function The function to profile.
     */
    void profile(std::string msg, std::function<void(int)> function)
    {
        profile_on_average(msg, function, _num_tries);
    }

private:
    Stopwatch<Microseconds> _ussw; ///< A Stopwatch object for measuring time in microseconds.
    Stopwatch<Milliseconds> _mssw; ///< A Stopwatch object for measuring time in milliseconds.
    Randomiser _rnd; ///< A Randomiser object for generating random numbers.
    const int _num_tries; ///< The number of times to execute the function for profiling.
};
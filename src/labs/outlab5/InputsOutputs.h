/**
 * @file InputsOutputs.h
 * @author Arjun Earthperson
 * @date 09/29/2023
 * @brief This file contains the input and output definitions for this project.
 */

#ifndef NE591_008_OUTLAB_5_INPUTOUTPUTS_H
#define NE591_008_OUTLAB_5_INPUTOUTPUTS_H

#include <utility>

#include "math/blas/Matrix.h"
#include "math/blas/Vector.h"
#include "json.hpp"

/**
    * @struct Input
    * @brief Struct representing the input matrices for the BLAS library.
    */
typedef struct Input {
    Input() = default;

    size_t n = 0; ///< Size of the matrices.
    MyBLAS::Matrix coefficients{}; ///< LU matrix.
    MyBLAS::Vector constants{}; ///< Vector of constants.

    /**
    * @brief Converts the input parameters to a JSON object.
    * @param jsonMap A reference to the JSON object to store the input parameters.
    */
    void toJSON(nlohmann::json &jsonMap) const {
        jsonMap["n"] = n;
        jsonMap["coefficients"] = coefficients.getData();
        jsonMap["constants"] = constants.getData();
    }
} InputMatrices;

typedef struct Map {
    Map() = default;

    MyBLAS::Matrix L;
    MyBLAS::Matrix U;
    MyBLAS::Matrix P;

    /**
    * @brief Converts the input parameters to a JSON object.
    * @param jsonMap A reference to the JSON object to store the input parameters.
    */
    void toJSON(nlohmann::json &jsonMap) const {
        jsonMap["lower"] = L.getData();
        jsonMap["upper"] = U.getData();
        jsonMap["permutation"] = P.getData();
    }
} IntermediateResults;

/**
 * @struct Output
 * @brief Struct representing the output vector for the BLAS library.
 */
typedef struct Output {
    Output() = default;

    MyBLAS::Vector solution; ///< Output vector.
    MyBLAS::Vector residual;

    /**
     * @brief Converts the output vector to a JSON object.
     * @param jsonMap A reference to the JSON object to store the output vector.
     */
    void toJSON(nlohmann::json &jsonMap) const {
        jsonMap["solution"] = solution.getData();
        jsonMap["residual"] = residual.getData();
        jsonMap["max_residual"] = MyBLAS::max<long double>(residual);
    }
} Results;

#endif //NE591_008_OUTLAB_5_INPUTOUTPUTS_H

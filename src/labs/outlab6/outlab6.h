/**
 * @file outlab6.cpp
 * @author Arjun Earthperson
 * @date 10/06/2023
 * @brief This file contains the declaration for the OutLab6 class.
 *
 */

#ifndef NE591_008_OUTLAB6_H
#define NE591_008_OUTLAB6_H

#include <variant>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <boost/program_options.hpp>

#include "InputsOutputs.h"
#include "Parser.h"

#include "Project.h"
#include "CommandLine.h"

#include "math/blas/MyBLAS.h"
#include "math/blas/Matrix.h"
#include "math/factorization/LU.h"

#include "json.hpp"
#include "Compute.h"


/**
 * @class OutLab6
 * @brief This class is a child of the Project class
 * @details The class takes in command line arguments and uses them to solve the system of equations.
 */
class OutLab6 : public Project<OutLab6Inputs, Parser, OutLab6Outputs> {

public:
    /**
     * @brief Constructor for the outlab6 class
     * @param args Command line arguments
     */
    explicit OutLab6(CommandLineArgs args) : Project(args) {}

protected:
    /**
     * @brief This function builds the header information for the project.
     * @return HeaderInfo object containing project information
     */
    HeaderInfo buildHeaderInfo() override {
        Canvas canvas;
        auto x = 0.13;
        auto y = -0.66;
        auto iterations = 80;
        canvas.x_start = -0.007514104707;
        canvas.x_stop = 0.075446744304;
        canvas.y_start = 0.825578589953;
        canvas.y_stop = 0.883651184261;
        canvas.tone_map.growth_rate = 0.25;
        printJuliaSet<__float128>(canvas, x, y, iterations); //"o█■"
        std::cout<<"Julia set at ("<<x<<","<<y<<"), "<<iterations<<" iterations\n";
        return {
                .ProjectName = "NE591: OutLab 06",
                .ProjectDescription = "Solving a system of linear equations using iterative methods",
                .SubmissionDate = "10/06/2023",
                .StudentName = "Arjun Earthperson",
                .HeaderArt = " ",
        };
    }

    // TODO:: Document
    static void printResults(OutLab6Outputs &results) {
        std::cout<<"\ttotal iterations          : "<<(results.solution.iterations)<<std::endl;
        std::cout<<"\tconverged                 : "<<(results.solution.converged ? "Yes" : "No")<<std::endl;
        std::cout<<"\titerative error           : "<<(results.solution.iterative_error)<<std::endl;
        std::cout<<"\tabsolute maximum residual : "<<(results.solution.getMaxResidual(results.inputs.input.coefficients, results.inputs.input.constants))<<std::endl;
        std::cout<<"\texecution time (ns)       : "<<(results.execution_time)<<std::endl;
        std::cout<<"\texecution time (ms)       : "<<(results.execution_time/1.0e6)<<std::endl;
        std::cout<<"\texecution time (s)        : "<<(results.execution_time/1.0e9)<<std::endl;
    }

    // TODO:: DOCUMENT
    void run(OutLab6Outputs &outputs, OutLab6Inputs &inputs, boost::program_options::variables_map &values) override {

        /**
         * TODO:: Document
        **/

        size_t n = 8;
        auto A = MyBLAS::generateOrthogonalMatrix<long double>(n);
        auto x = MyBLAS::generateVector<long double>(n);
        auto b = A*x;
        outputs.inputs.input.coefficients = A;
        inputs.input.coefficients = A;
        outputs.inputs.input.constants = b;
        outputs.inputs.input.relaxation_factor = 1.1110;
        inputs.input.relaxation_factor = 1.1110;
        inputs.input.constants = b;
        outputs.inputs.input.n = n;
        inputs.input.n = n;

        nlohmann::json results;
        inputs.toJSON(results["inputs"]);

        if (inputs.methods.count(MyFactorizationMethod::Type::METHOD_LUP)) {
            OutLab6Outputs runResults(inputs);
            Compute::usingLUP(runResults, inputs);
            runResults.toJSON(results["outputs"][MyFactorizationMethod::TypeKey(MyFactorizationMethod::Type::METHOD_LUP)]);
            Parser::printLine();
            std::cout<<"LUP Factorization Results"<<std::endl;
            Parser::printLine();
            printResults(runResults);
        }

        if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_POINT_JACOBI)) {
            OutLab6Outputs runResults(inputs);
            Compute::usingPointJacobi(runResults, inputs);
            runResults.toJSON(results["outputs"][MyRelaxationMethod::TypeKey(MyRelaxationMethod::Type::METHOD_POINT_JACOBI)]);
            Parser::printLine();
            std::cout<<"Point Jacobi Method Results"<<std::endl;
            Parser::printLine();
            printResults(runResults);
        }

        if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_GAUSS_SEIDEL)) {
            OutLab6Outputs runResults(inputs);
            Compute::usingGaussSeidel(runResults, inputs);
            runResults.toJSON(results["outputs"][MyRelaxationMethod::TypeKey(MyRelaxationMethod::Type::METHOD_GAUSS_SEIDEL)]);
            Parser::printLine();
            std::cout<<"Gauss-Seidel Method Results"<<std::endl;
            Parser::printLine();
            printResults(runResults);
        }

        if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_SOR)) {
            OutLab6Outputs runResults(inputs);
            Compute::usingSOR(runResults, inputs);
            runResults.toJSON(results["outputs"][MyRelaxationMethod::TypeKey(MyRelaxationMethod::Type::METHOD_SOR)]);
            Parser::printLine();
            std::cout<<"SOR Method Results"<<std::endl;
            Parser::printLine();
            printResults(runResults);
        }

        if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_SORJ)) {
            OutLab6Outputs runResults(inputs);
            Compute::usingJacobiSOR(runResults, inputs);
            runResults.toJSON(results["outputs"][MyRelaxationMethod::TypeKey(MyRelaxationMethod::Type::METHOD_SORJ)]);
            Parser::printLine();
            std::cout<<"SOR Point Jacobi Method Results"<<std::endl;
            Parser::printLine();
            printResults(runResults);
        }

        if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_SSOR)) {
            OutLab6Outputs runResults(inputs);
            Compute::usingSymmetricSOR(runResults, inputs);
            runResults.toJSON(results["outputs"][MyRelaxationMethod::TypeKey(MyRelaxationMethod::Type::METHOD_SSOR)]);
            Parser::printLine();
            std::cout<<"Symmetric SOR Method Results"<<std::endl;
            Parser::printLine();
            printResults(runResults);
        }

        Parser::printLine();
        writeJSON(values["output-json"].as<std::string>(), results);
    }

};

#endif // NE591_008_OUTLAB6_H

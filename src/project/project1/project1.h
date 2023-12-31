/**
 * @file project1.cpp
 * @author Arjun Earthperson
 * @date 09/29/2023
 * @brief This file contains the declaration for the Project1 class.
 *
 */

#ifndef NE591_008_PROJECT1_PROJECT1_H
#define NE591_008_PROJECT1_PROJECT1_H

#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <variant>

#include "InputsOutputs.h"
#include "Parser.h"

#include "CommandLine.h"
#include "Project.h"

#include "math/blas/BLAS.h"
#include "math/blas/matrix/Matrix.h"
#include "math/factorization/LU.h"
#include "utils/math/Stats.h"

#include "Compute.h"
#include "json.hpp"

/**
 * @class Project1
 * @brief This class is a child of the Project class and is used to solve a system of linear equations using forward and
 * back substitution.
 * @details The class takes in command line arguments and uses them to solve the system of equations.
 */
class Project1 : public Project<SolverInputs, Parser, SolverOutputs> {

  public:
    /**
     * @brief Constructor for the project1 class
     * @param args Command line arguments
     */
    explicit Project1(CommandLineArgs args) : Project(args) {}

  protected:
    /**
     * @brief This function builds the header information for the project.
     * @return HeaderInfo object containing project information
     */
    HeaderInfo buildHeaderInfo() override {
        Canvas canvas;
        auto x = -0.172;
        auto y = -0.66;
        size_t iterations = 2000;
        canvas.x_start = -0.007514104707;
        canvas.x_stop = 0.075446744304;
        canvas.y_start = 0.825578589953;
        canvas.y_stop = 0.883651184261;
        printJuliaSet<__float128>(canvas, x, y, iterations); //"o█■"
        return {
            .ProjectName = "NE591: Project Milestone 1",
            .ProjectDescription = "Serial Neutron Diffusion Code",
            .SubmissionDate = "09/29/2023",
            .StudentName = "Arjun Earthperson",
            .HeaderArt = " ",
        };
    }

    /**
     * @brief This function runs the project.
     * @details It solves the system of linear equations using forward and back substitution.
     * @param output The output vector
     * @param inputs The input matrices
     * @param values The variable map
     */
    void run(SolverOutputs &output, SolverInputs &inputs, boost::program_options::variables_map &values) override {

        /**
            1. Read input parameters: Read the values of 𝑎, 𝑏, 𝑚, 𝑛, 𝐷, and Σₐ from an input file. Also,
            read the non-uniformly distributed fixed source 𝑞(𝑖,𝑗) for 𝑖 = 1, … , 𝑚 and 𝑗 = 1, … , 𝑛 from the input
        file.

            2. Calculate mesh spacings: Compute the mesh spacings 𝛿 and 𝛾 using the formulas 𝛿 = 𝑎/(𝑚+1) and 𝛾 =
        𝑏/(𝑛+1).

            3. Initialize the matrix and right-hand-side vector: Create an 𝑚×𝑛 matrix A and an 𝑚×𝑛 right-hand-side
            vector B. Initialize all elements of A and B to zero.

            4. Fill the matrix A and vector B: Loop through all the nodes 𝑖 = 1, … , 𝑚 and 𝑗 = 1, … , 𝑛, and fill the
            matrix A and vector B using the given equation:

                   −D((φ(i+1,j) - 2φ(i,j) + φ(i-1,j))/δ² + (φ(i,j+1) - 2φ(i,j) + φ(i,j-1))/γ²) + Σₐφ(i,j) = q(i,j)

            Note that the boundary conditions are already taken into account as the fluxes at the boundaries are zero.

            5. Solve the linear system: Solve the linear system A * 𝜙 = B using LU factorization with pivoting.
            The solution vector 𝜙 will contain the scalar fluxes at each node 𝑖, 𝑗, where 𝑖 = 1, … , 𝑚 and 𝑗 = 1, … , 𝑛.

            6. Output the results: Write the scalar fluxes 𝜙(𝑖,𝑗) for 𝑖 = 1, … , 𝑚 and 𝑗 = 1, … , 𝑛 to an output file.
        **/

        auto profiler1 = getProfiler([&]() {
            // Your arbitrary block of code here
            int sum = 0;
            for (int i = 0; i < 1000000; ++i) {
                sum += i;
            }
        }, 100, 0, "profiler1"); // Run the block 100 times

        profiler1.run();

        const bool quietMode = values.count("quiet");

        std::vector<Stopwatch<Nanoseconds>> clocks = std::vector<Stopwatch<Nanoseconds>>(5);
        std::vector<long double> durations = std::vector<long double>(5);
        nlohmann::json profiler;
        IntermediateResults intermediates;


        auto profiler2 = getProfiler([&]() {
            intermediates = initialize_diffusion_matrix_and_vector<long double>(inputs.m, inputs.n);
            naive_fill_diffusion_matrix_and_vector(inputs, intermediates);
        }, 100, 0, "profiler1"); // Run the block 100 times

        clocks[0].restart();
        profiler2.run();
        clocks[0].click();
        // Step 1. Initialize matrices

        durations[0] = static_cast<long double>(clocks[0].duration().count()) / 100.0;
        profiler["exclusive"]["initialize_and_fill"] = durations[0];
        profiler["cumulative"]["initialize_and_fill"] = durations[0];
        std::cout<<profiler2;

        if (!quietMode) {
            Parser::printLine();
            std::cout << "Diffusion Matrix A: \n";
            Parser::printLine();
            std::cout << intermediates.diffusion_matrix_A;
            Parser::printLine();
            std::cout << "Right Hand Side Vector B: \n";
            Parser::printLine();
            std::cout << intermediates.right_hand_side_vector_B;
        }

        durations[1] = static_cast<long double>(clocks[1].duration().count());
        profiler["exclusive"]["lup_factorize"] = durations[1];
        profiler["cumulative"]["lup_factorize"] = durations[1] + durations[0];

        MyBLAS::Vector<long double> b, Pb, y, phi;
        clocks[2].restart();
        {
            b = intermediates.right_hand_side_vector_B;
            Pb = intermediates.P * intermediates.right_hand_side_vector_B;
            y = MyBLAS::forwardSubstitution(intermediates.L, Pb);
            phi = MyBLAS::backwardSubstitution(intermediates.U, y);
        }
        clocks[2].click();
        durations[2] = static_cast<long double>(clocks[2].duration().count());
        profiler["exclusive"]["fb_substitution"] = durations[2];
        profiler["cumulative"]["fb_substitution"] = durations[2] + durations[1] + durations[0];

        profiler["exclusive"]["solve_linear_system"] = durations[2] + durations[1];
        profiler["cumulative"]["solve_linear_system"] = durations[2] + durations[1] + durations[0];

        MyBLAS::Vector<long double> b_prime, r;
        long double maxResidual;
        clocks[3].restart();
        {
            // compute fluxes
            fill_fluxes(phi, inputs, output);
            b_prime = intermediates.diffusion_matrix_A * phi;
            r = b - b_prime;
            output.residual = r;
            output.solution = phi;

            maxResidual = MyBLAS::Stats::max<long double>(MyBLAS::Stats::abs(r));
        }
        clocks[3].click();
        durations[3] = static_cast<long double>(clocks[3].duration().count());
        profiler["exclusive"]["residuals"] = durations[3];
        profiler["cumulative"]["residuals"] = durations[3] + durations[2] + durations[1] + durations[0];

        if (!values.count("quiet")) {
            Parser::printLine();
            std::cout << "Lower Triangular Matrix (L):\n";
            Parser::printLine();
            std::cout << intermediates.L;
            Parser::printLine();
            std::cout << "Upper Triangular Matrix (U):\n";
            Parser::printLine();
            std::cout << intermediates.U;
            Parser::printLine();
            std::cout << "Permutation Matrix (P):\n";
            Parser::printLine();
            std::cout << intermediates.P;
            Parser::printLine();
            std::cout << "Permuted constants (Pb = P * b):\n";
            Parser::printLine();
            std::cout << Pb;
            Parser::printLine();
            std::cout << "Intermediate vector (y), where (Ly = Pb):\n";
            Parser::printLine();
            std::cout << y;
            Parser::printLine();
            std::cout << "Solution vector (x, phi), where (Ux = y):\n";
            Parser::printLine();
            std::cout << phi;
            Parser::printLine();
            Parser::printLine();
            std::cout << "Computed flux 𝜙(𝑖,𝑗): \n";
            Parser::printLine();
            std::cout << output.fluxes;
            Parser::printLine();
            std::cout << "Residual vector (r = b - Ax) :\n";
            Parser::printLine();
            std::cout << r;
            Parser::printLine();
            std::cout << "Max Residual abs(r): ";
            std::cout << std::setprecision(max_precision) << maxResidual << std::endl;
            Parser::printLine();
        }

        clocks[4].restart();
        {
            // write json
            if (values.count("output-results-json")) {
                nlohmann::json results;
                inputs.toJSON(results["inputs"]);
                output.toJSON(results["outputs"]);
                writeJSON(values["output-results-json"].as<std::string>(), results);
            }

            if (values.count("output-flux-csv")) {
                writeCSVMatrixNoHeaders(values["output-flux-csv"].as<std::string>(), output.fluxes);
            }
        }
        clocks[4].click();
        durations[4] = static_cast<long double>(clocks[4].duration().count());
        profiler["exclusive"]["post-process"] = durations[4];
        profiler["cumulative"]["post-process"] =
            durations[4] + durations[3] + durations[2] + durations[1] + durations[0];
        profiler["total"] = durations[4] + durations[3] + durations[2] + durations[1] + durations[0];
        profiler["problem_size"] = inputs.m * inputs.n;

        writeJSON("profile_" + std::to_string(inputs.m) + "x" + std::to_string(inputs.n) + ".json", profiler);
    }
};

#endif // NE591_008_PROJECT1_PROJECT1_H

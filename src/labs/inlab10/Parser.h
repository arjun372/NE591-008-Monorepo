/**
 * @file Parser.h
 * @author Arjun Earthperson
 * @date 10/20/2023
 * @brief This file contains the Parser class which is responsible for parsing command line arguments and validating
 * user inputs.
 */

#ifndef NE591_008_INLAB10_PARSER_H
#define NE591_008_INLAB10_PARSER_H

#include "math/blas/system/Circuit.h"

#include "InputsOutputs.h"
#include "CheckBounds.h"
#include "FileParser.h"
#include "Helpers.h"

/**
 * @class Parser
 * @brief This class is responsible for parsing command line arguments and validating user inputs.
 *
 * The Parser class extends the CommandLine class template with InLab10Inputs as the template argument.
 * It provides methods to build input arguments, print input arguments, perform checks on input arguments,
 * and build inputs based on the parsed arguments.
 */
class Parser : public CommandLine<InLab10Inputs> {

  public:
    /**
     * @brief Constructor that initializes the Parser object with header information and command line arguments.
     *
     * @param headerInfo A constant reference to a HeaderInfo object containing the header information.
     * @param args A constant reference to a CommandLineArgs object containing the command line arguments.
     */
    explicit Parser(const HeaderInfo &headerInfo, const CommandLineArgs &args) : CommandLine<InLab10Inputs>(headerInfo, args) {}

    /**
     * @brief Default constructor for the Parser class.
     *
     * This constructor is explicitly defined to allow creating Parser objects without any initialization.
     */
    explicit Parser() = default;

  protected:
    /**
     * @brief This function builds the input options for the program.
     *
     * @return A boost::program_options::options_description object containing the description of the input options.
     */
    void buildInputArguments(boost::program_options::options_description &values) override {

        boost::program_options::options_description solverOptions("Solver Options");
        solverOptions.add_options()
            ("stopping-criterion,e", "= stopping criterion [+ve R]")
            ("max-iterations,k", "= maximum iterations [k ∈ ℕ]");

        boost::program_options::options_description fileOptions("File I/O Options");
        fileOptions.add_options()(
            "input-json,i", boost::program_options::value<std::string>(), "= input JSON containing n")(
            "output-json,o", boost::program_options::value<std::string>(), "= path for the output JSON");

        values.add(solverOptions);
        values.add(fileOptions);
    }

    /**
     * @brief This function prints the input values.
     *
     * @param vm A boost::program_options::variables_map object containing the input values to be printed.
     *
     */
    void printInputArguments(boost::program_options::variables_map &vm) override {
        // list the parameters
        CommandLine::printLine();
        std::cout << std::setw(44) << "Inputs\n";
        CommandLine::printLine();
        std::cout << "\tInput JSON,              i: " << vm["input-json"].as<std::string>() << std::endl;
        std::cout << "\tOutput JSON,             o: " << vm["output-json"].as<std::string>() << std::endl;
        std::cout << "\tStopping Criterion,      e: " << vm["stopping-criterion"].as<long double>() << std::endl;
        std::cout << "\tMaximum Iterations,      k: " << vm["max-iterations"].as<long double>() << std::endl;
        CommandLine::printLine();
    }

    /**
     * @brief This function performs checks on the input parameters and prompts the user to enter valid inputs if the
     * current inputs are invalid.
     *
     * @param map A boost::program_options::variables_map object containing the input values to be checked.
     */
    /**
     * @brief This function performs checks on the input parameters and prompts the user to enter valid inputs if the
     * current inputs are invalid.
     *
     * @param map A boost::program_options::variables_map object containing the input values to be checked.
     */
    void performInputArgumentsCheck(boost::program_options::variables_map &map) override {

        std::string input;

        // Check if input file path is provided
        if (!map.count("input-json") || map["input-json"].empty() ||
            !doesFileExist(map["input-json"].as<std::string>())) {
            while (!map.count("input-json") || map["input-json"].empty() ||
                   !doesFileExist(map["input-json"].as<std::string>())) {
                std::cerr << "Error: No input JSON filepath provided.\n" << std::endl;
                std::cout << "Enter input file path (file extension is .json): ";
                std::cin >> input;
                try {
                    replace(map, "input-json", input);
                } catch (const std::exception &) {
                    continue;
                }
            }
        }

        // Check if output file path is provided and writable
        if (!map.count("output-json") || map["output-json"].empty() ||
            !isFileWritable(map["output-json"].as<std::string>())) {
            while (!map.count("output-json") || map["output-json"].empty() ||
                   !isFileWritable(map["output-json"].as<std::string>())) {
                std::cerr << "Error: No output JSON filepath provided.\n" << std::endl;
                std::cout << "Enter output file path (file extension is .json): ";
                std::cin >> input;
                try {
                    replace(map, "output-json", input);
                } catch (const std::exception &) {
                    continue;
                }
            }
        }

        // read the input json and populate the variables_map
        nlohmann::json inputMap;
        try {
            readJSON(map["input-json"].as<std::string>(), inputMap);
        } catch (...) {
            // initialize input map if no file was read
        }

        std::vector<std::function<bool(long double)>> checks;

        // add checks for parameters e, k
        checks.clear();
        checks.emplace_back([](long double value) { return failsNaturalNumberCheck(value); });
        performChecksAndUpdateInput<long double>("max-iterations", inputMap, map, checks);

        checks.clear();
        checks.emplace_back([](long double value) { return failsPositiveNumberCheck(value); });
        performChecksAndUpdateInput<long double>("stopping-criterion", inputMap, map, checks);
    }

    /**
     * @brief Builds the inputs
     */
    void buildInputs(InLab10Inputs &input, boost::program_options::variables_map &values) override {

        // first, read the input file into a json map
        nlohmann::json inputMap;
        readJSON(values["input-json"].as<std::string>(), inputMap);

        // read the output json path
        // TODO:: this should be wrapped in some path validated file write checker
        input.outputJSON = values["output-json"].as<std::string>();

        // read the constants
        MyBLAS::Vector<long double> constants = MyBLAS::Vector(std::vector<long double>(inputMap["constants"]));
        input.constants = MyBLAS::Vector(constants);

        // read the coefficient matrix
        input.coefficients = MyBLAS::Matrix(std::vector<std::vector<long double>>(inputMap["coefficients"]));

        if (!MyBLAS::isSquareMatrix(input.coefficients)) {
            std::cerr << "Error: Input coefficients matrix A not square, aborting.\n";
            exit(-1);
        } else if (!values.count("quiet")) {
            std::cout << "Input coefficients matrix A is square.\n";
        }

        if (!MyBLAS::isSymmetricMatrix(input.coefficients)) {
            std::cerr << "Warning: Input coefficients matrix A is not symmetric.\n";
        } else if (!values.count("quiet")) {
            std::cout << "Input coefficients matrix A is symmetric.\n";
        }

        if (!MyBLAS::isPositiveDefiniteMatrix(input.coefficients)) {
            std::cerr << "Warning: Input coefficients matrix A is not positive definite.\n";
        } else if (!values.count("quiet")) {
            std::cout << "Input coefficients matrix A is positive definite.\n";
        }

        if (input.coefficients.getRows() != input.constants.size()) {
            std::cerr << "Error: Input constants vector 'b' not order n, aborting.\n";
            exit(-1);
        }

        input.n = input.coefficients.getRows();
        input.threshold = values["stopping-criterion"].as<long double>();
        input.max_iterations = static_cast<size_t>(values["max-iterations"].as<long double>());

        // print the matrices since we are in verbose mode.
        if (!values.count("quiet")) {
            const auto precision = getCurrentPrecision();
            printLine();
            std::cout << "Coefficient Matrix (A):\n";
            printLine();
            std::cout << std::setprecision(static_cast<int>(precision)) << input.coefficients;
            printLine();
            std::cout << "Constants Vector (b):\n";
            printLine();
            std::cout << std::setprecision(static_cast<int>(precision)) << input.constants;
        }
    }
};

#endif // NE591_008_INLAB10_PARSER_H

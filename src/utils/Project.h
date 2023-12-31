/**
 * @file Project.h
 * @author Arjun Earthperson
 * @date 08/30/2023
 * @brief This file contains the definition of the Project class template.
 *        The Project class is a generic class that represents a project that can be executed.
 */

#ifndef NE591_008_PROJECT_H
#define NE591_008_PROJECT_H

#include "CommandLine.h"
#include "utils/profiler/Profiler.h"
#include <boost/program_options.hpp>
#include <utility>

/**
 * @class Project
 * @brief A generic class that represents a project that can be executed.
 * @tparam InputType The type of the input data.
 * @tparam CommandLineParserType The type of the command line parser.
 * @tparam OutputType The type of the output data.
 */
template <typename InputType, typename CommandLineParserType, typename OutputType> class Project {

  public:
    /**
     * @brief A static assertion to ensure that CommandLineParserType is a derived class of CommandLine.
     */
    static_assert(std::is_base_of<CommandLine<InputType>, CommandLineParserType>::value,
                  "CommandLineParserType must be a derived class of CommandLine");

    /**
     * @brief Constructor that initializes the command line arguments.
     * @param args The command line arguments.
     */
    explicit Project(CommandLineArgs args) { cmdArgs = args; }

    /**
     * @brief Default virtual destructor.
     */
    virtual ~Project() = default;

    virtual /**
     * @brief Executes the project.
     */
    void execute() {
        initialize();
        timedRun();
        finalize();
    }

    virtual /**
     * @brief Returns the terminal.
     * @return The terminal.
     */
    CommandLineParserType getTerminal() const { return terminal; }

    /**
     * @brief Returns the input data.
     * @return A reference to the input data.
     */
    InputType &getInputs() {
        return this->inputs;
    }

  protected:
    /**
     * @brief Holds the command line arguments.
     */
    CommandLineArgs cmdArgs{};

    /**
     * @brief An instance of CommandLineParserType which is used to parse command line arguments.
     */
    CommandLineParserType terminal;

    /**
     * @brief Holds the output data of the project.
     */
    OutputType _outputs;

    virtual /**
     * @brief Initializes the project.
     */
    void initialize() {
        terminal = CommandLineParserType(buildHeaderInfo(), cmdArgs);
        terminal.getInputs();
        _outputs = OutputType();
    }

    /**
     * @brief Finalizes the project.
     * This is a virtual function and can be overridden by derived classes to provide specific finalization steps.
     */
    virtual void finalize() {

    }

    virtual /**
     * @brief Executes the project in a timed manner.
     * This method calls preRun, run, and postRun methods in sequence.
     */
    void timedRun() {
        preRun(_outputs, terminal.getInputs(), terminal.getArguments());

        run(_outputs, terminal.getInputs(), terminal.getArguments());

        postRun(_outputs, terminal.getInputs(), terminal.getArguments());
    }

  protected:
    /**
     * @brief Builds the header information.
     * @return The header information.
     */
    virtual HeaderInfo buildHeaderInfo() = 0;

    /**
     * @brief A hook for performing actions before the run.
     * @param output The output data.
     * @param input The input data.
     * @param values The variable map.
     */
    virtual void preRun(OutputType &output, InputType &input, boost::program_options::variables_map &values) {}

    /**
     * @brief Executes the project.
     * @param output The output data.
     * @param input The input data.
     * @param values The variable map.
     */
    virtual void run(OutputType &output, InputType &input, boost::program_options::variables_map &values) {}

    /**
     * @brief A hook for performing actions after the run.
     * @param output The output data.
     * @param input The input data.
     * @param values The variable map.
     */
    virtual void postRun(OutputType &output, InputType &input, boost::program_options::variables_map &values) {}

    /**
     * @brief Creates a Profiler object with the given function, number of samples, timeout, and description.
     * @tparam Func The type of the function to be profiled.
     * @param func The function to be profiled.
     * @param samples The number of samples to be taken.
     * @param timeout The maximum time for the profiling.
     * @param description The description of the profiling.
     * @return A Profiler object.
     */
    template <typename Func>
    auto getProfiler(Func func, size_t samples, long double timeout = 0, std::string description = "") {
        return Profiler(func, samples, timeout, description);
    }

    /**
     * @brief Creates a Profiler object with the given function, number of samples, and description.
     * @tparam Func The type of the function to be profiled.
     * @param func The function to be profiled.
     * @param samples The number of samples to be taken.
     * @param description The description of the profiling.
     * @return A Profiler object.
     */
    template <typename Func>
    auto getProfiler(Func func, size_t samples, std::string description = "") {
        return Profiler(func, samples, 0, description);
    }

    /**
     * @brief Creates a Profiler object with the given function and description.
     * @tparam Func The type of the function to be profiled.
     * @param func The function to be profiled.
     * @param description The description of the profiling.
     * @return A Profiler object.
     */
    template <typename Func>
    auto getProfiler(Func func, std::string description = "") {
        return ProfilerHelper::InitProfiler(func, getTerminal().getArguments(), description);
    }
};

#endif // NE591_008_PROJECT_H

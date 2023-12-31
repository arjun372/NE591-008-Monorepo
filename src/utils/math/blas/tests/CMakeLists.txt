set(TEST_SOURCES
        main.cpp
        lazy/LazyMatrixTests.cpp
        lazy/LazyVectorTests.cpp
        expression/MatrixExpressionTests.cpp
        expression/VectorExpressionTests.cpp
        matrix/BaseMatrixTests.cpp
        matrix/MatrixPerformanceTests.cpp
        matrix/MatrixMemoryAllocationTests.cpp
        matrix/MatrixUtilityTests.cpp
        vector/BaseVectorTests.cpp
        vector/VectorPerformanceTests.cpp
        vector/VectorMemoryAllocationTests.cpp
        vector/VectorUtilityTests.cpp
        TestedTypes.h
)

if (NOT TARGET blas_tests)
    # Create the test executable
    add_executable(blas_tests ${TEST_SOURCES})
    # Link the test executable with Google Test and your project's library
    target_link_libraries(blas_tests utils gtest gtest_main)
    add_test(NAME blas_tests COMMAND blas_tests)
endif ()

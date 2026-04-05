cmake_minimum_required(VERSION 3.14)
project(FCM_Tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

include_directories(
    ../../src
    ../../src/libs/googletest/googletest/include
)

add_subdirectory(../../src/libs/googletest googletest_build)

include(GoogleTest)

function(add_test_runtime_dependencies target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${target_name}>
            $<TARGET_FILE_DIR:${target_name}>
        COMMAND_EXPAND_LISTS
    )
endfunction()

function(add_activation_function_test target_name test_source)
    add_executable(${target_name}
        ${test_source}
        ../../src/model/activation_functions/activation_function.cpp
        ../../src/model/activation_functions/bivalent.cpp
        ../../src/model/activation_functions/fabric.cpp
        ../../src/model/activation_functions/sigmoid.cpp
        ../../src/model/activation_functions/tanh.cpp
        ../../src/model/activation_functions/threshold_linear.cpp
        ../../src/model/activation_functions/trivalent.cpp
    )

    target_link_libraries(${target_name} PRIVATE gtest gtest_main Qt6::Core)
    add_test_runtime_dependencies(${target_name})
    gtest_discover_tests(${target_name})
endfunction()

function(add_metric_test target_name test_source)
    add_executable(${target_name}
        ${test_source}
        ../../src/model/metrics/fabric.cpp
        ../../src/model/metrics/mae.cpp
        ../../src/model/metrics/mape.cpp
        ../../src/model/metrics/mse.cpp
    )

    target_link_libraries(${target_name} PRIVATE gtest gtest_main Qt6::Core)
    add_test_runtime_dependencies(${target_name})
    gtest_discover_tests(${target_name})
endfunction()

function(add_algorithm_test target_name test_source)
    add_executable(${target_name}
        ${test_source}
        ../../src/model/activation_functions/activation_function.cpp
        ../../src/model/algorithms/fabric.cpp
        ../../src/model/algorithms/standard.cpp
        ../../src/model/algorithms/standard_fuzzy.cpp
        ../../src/model/algorithms/weights_prediction.cpp
        ../../src/model/algorithms/weights_prediction_fuzzy.cpp
        ../../src/model/entities/triangular_fuzzy_value.cpp
    )

    target_link_libraries(${target_name} PRIVATE gtest gtest_main Qt6::Core)
    add_test_runtime_dependencies(${target_name})
    gtest_discover_tests(${target_name})
endfunction()

function(add_entity_test target_name test_source)
    add_executable(${target_name}
        ${test_source}
        ../../src/model/entities/triangular_fuzzy_value.cpp
    )

    target_link_libraries(${target_name} PRIVATE gtest gtest_main Qt6::Core)
    add_test_runtime_dependencies(${target_name})
    gtest_discover_tests(${target_name})
endfunction()

add_activation_function_test(activation_function_test activation_functions/activation_function_test.cpp)
add_activation_function_test(bivalent_test activation_functions/bivalent_test.cpp)
add_activation_function_test(sigmoid_test activation_functions/sigmoid_test.cpp)
add_activation_function_test(tanh_test activation_functions/tanh_test.cpp)
add_activation_function_test(threshold_linear_test activation_functions/threshold_linear_test.cpp)
add_activation_function_test(trivalent_test activation_functions/trivalent_test.cpp)
add_activation_function_test(activation_functions_fabric_test activation_functions/fabric_test.cpp)

add_metric_test(mae_test metrics/mae_test.cpp)
add_metric_test(mape_test metrics/mape_test.cpp)
add_metric_test(mse_test metrics/mse_test.cpp)
add_metric_test(metrics_fabric_test metrics/fabric_test.cpp)

add_algorithm_test(standard_prediction_algorithm_test algorithms/standard_test.cpp)
add_algorithm_test(weights_prediction_algorithm_test algorithms/weights_prediction_test.cpp)
add_algorithm_test(standard_fuzzy_algorithm_test algorithms/standard_fuzzy_test.cpp)
add_algorithm_test(weights_prediction_fuzzy_algorithm_test algorithms/weights_prediction_fuzzy_test.cpp)
add_algorithm_test(algorithms_fabric_test algorithms/fabric_test.cpp)

add_entity_test(triangular_fuzzy_value_test entities/triangular_fuzzy_value_test.cpp)

cmake_minimum_required(VERSION 3.14)
project(FCM_Tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# Пути к вашим заголовкам
include_directories(
    ../../src
    ../../src/libs/googletest/googletest/include
)

# Добавляем исходники GoogleTest
add_subdirectory(../../src/libs/googletest googletest_build)

# Собираем ваш тест
add_executable(threshold_linear_test
    activation_functions/threshold_linear_test.cpp
    ../../src/model/activation_functions/threshold_linear.cpp
)

# Линкуем с gtest и gtest_main
target_link_libraries(threshold_linear_test PRIVATE gtest gtest_main)

# Регистрируем тесты
include(GoogleTest)
gtest_discover_tests(threshold_linear_test)

cmake_minimum_required(VERSION 3.14)
project(FCM_Ui_Tests)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

include_directories(
    ../../src
)

function(add_ui_test target_name test_source)
    add_executable(${target_name}
        ${test_source}
    )

    target_link_libraries(${target_name} PRIVATE Qt6::Core Qt6::Widgets Qt6::Test)

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${target_name}>/platforms
    )

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${target_name}>
            $<TARGET_FILE_DIR:${target_name}>
        COMMAND_EXPAND_LISTS
    )

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:Qt6::Core>/../plugins/platforms/qoffscreen.dll
            $<TARGET_FILE_DIR:Qt6::Core>/../plugins/platforms/qwindows.dll
            $<TARGET_FILE_DIR:${target_name}>/platforms
    )

    add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
    set_tests_properties(${target_name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${target_name}>
    )
endfunction()

add_ui_test(example_qt_ui_test example_qt_ui_test.cpp)

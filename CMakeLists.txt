cmake_minimum_required(VERSION 3.16)

project(FCM VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    add_compile_options(-Wa,-mbig-obj)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Sql PrintSupport)

set(TS_FILES translations/FCM_ru_RU.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/ui/main_window/main_window.cpp
        src/ui/main_window/main_window.h
        src/ui/main_window/main_window.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(FCM
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/ui/graph_editor/node_item.h src/ui/graph_editor/node_item.cpp
        src/ui/graph_editor/edge_item.h src/ui/graph_editor/edge_item.cpp
        src/ui/graph_editor/graph_scene.h src/ui/graph_editor/graph_scene.cpp
        src/ui/graph_editor/graph_view.h src/ui/graph_editor/graph_view.cpp

        src/ui/graph_editor/color_value_adapter/IColorValueAdapter.h
        src/ui/graph_editor/color_value_adapter/color_value_adapter.h src/ui/graph_editor/color_value_adapter/color_value_adapter.cpp
        src/presenter/simulation_presenter.h src/presenter/simulation_presenter.cpp
        src/presenter/prediction_parameters.h
        src/presenter/simulation_parameters.h
        src/model/activation_functions/activation_function.h
        src/model/activation_functions/bivalent.h src/model/activation_functions/bivalent.cpp
        src/model/activation_functions/trivalent.h src/model/activation_functions/trivalent.cpp
        src/model/activation_functions/threshold_linear.h src/model/activation_functions/threshold_linear.cpp
        src/model/activation_functions/sigmoid.h src/model/activation_functions/sigmoid.cpp
        src/model/activation_functions/tanh.h src/model/activation_functions/tanh.cpp
        src/model/activation_functions/fabric.h src/model/activation_functions/fabric.cpp
        src/model/element_type.h
        src/model/algorithms/algorithm.h
        src/model/algorithms/standard.h src/model/algorithms/standard.cpp
        src/model/metrics/metric.h
        src/model/metrics/mse.h src/model/metrics/mse.cpp
        src/model/entities/calculation_fcm.h
        src/model/predictor.h src/model/predictor.cpp
        src/model/entities/term.h
        src/model/fuzzy_logic/fuzzifier.h
        src/model/fuzzy_logic/numeric_fuzzifier.h src/model/fuzzy_logic/numeric_fuzzifier.cpp
        src/model/entities/concept.h
        src/model/entities/weight.h
        src/model/entities/fcm.h

        src/presenter/edit_mode.h
        src/ui/graph_editor/edit_mode.h
        src/model/entities/experiment.h
        src/model/stop_conditions/stop_condition.h
        src/model/stop_conditions/fixed_steps.h src/model/stop_conditions/fixed_steps.cpp
        src/ui/concept_window/concept_window.h src/ui/concept_window/concept_window.cpp src/ui/concept_window/concept_window.ui
        src/ui/weight_window/weight_window.h src/ui/weight_window/weight_window.cpp src/ui/weight_window/weight_window.ui
        src/repository/models_repository.h src/repository/models_repository.cpp
        src/repository/migration_manager.h src/repository/migration_manager.cpp
        src/resources.qrc
        src/ui/load_model_window/load_model_window.h src/ui/load_model_window/load_model_window.cpp src/ui/load_model_window/load_model_window.ui

        src/libs/qcustomplot/qcustomplot.h src/libs/qcustomplot/qcustomplot.cpp
        src/repository/json_repository.h src/repository/json_repository.cpp
        src/model/static_analysis/static_analyzer.h src/model/static_analysis/static_analyzer.cpp
        src/presenter/static_analysis_presenter.h src/presenter/static_analysis_presenter.cpp
        src/presenter/creation_presenter.h src/presenter/creation_presenter.cpp
        src/ui/adjacency_table/adjacency_table_view.h src/ui/adjacency_table/adjacency_table_view.cpp
        src/ui/main_window/term_list/term_list_item.h
        src/model/entities/calculation_weight.h
        src/model/algorithms/weights_prediction.h src/model/algorithms/weights_prediction.cpp
        src/model/algorithms/fabric.h src/model/algorithms/fabric.cpp
        src/model/metrics/mae.h src/model/metrics/mae.cpp
        src/model/metrics/mape.h src/model/metrics/mape.cpp
        src/model/metrics/fabric.h src/model/metrics/fabric.cpp
        src/model/metrics_manager.h src/model/metrics_manager.cpp
        src/model/stop_conditions/fabric.h src/model/stop_conditions/fabric.cpp
        src/model/stop_conditions/static.h src/model/stop_conditions/static.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET FCM APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(FCM SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(FCM
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(FCM PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Sql Qt6::PrintSupport)
target_link_libraries(FCM PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(FCM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.FCM)
endif()
set_target_properties(FCM PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS FCM
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(FCM)
endif()

qt_add_resources(PROJECT_RESOURCES
    src/resources.qrc
)

target_sources(FCM PRIVATE ${PROJECT_RESOURCES})

enable_testing()
add_subdirectory(tests)

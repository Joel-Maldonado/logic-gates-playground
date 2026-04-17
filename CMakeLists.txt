cmake_minimum_required(VERSION 3.14)
project(logic-gates-playground VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(raylib QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

set(PROJECT_SOURCES
    src/app/main.cpp
    src/app/Application.cpp
    src/app/Config.cpp
    src/core/DerivedGates.cpp
    src/core/GateSymbolGeometry.cpp
    src/core/GatePin.cpp
    src/core/InputSource.cpp
    src/core/LogicGate.cpp
    src/core/OutputSink.cpp
    src/core/Wire.cpp
    src/simulation/CircuitSimulator.cpp
    src/ui/CommandPalette.cpp
    src/ui/CommandStack.cpp
    src/ui/DesignTokens.cpp
    src/ui/GateFactory.cpp
    src/ui/GateGeometry.cpp
    src/ui/GatePainter.cpp
    src/ui/InspectorPanel.cpp
    src/ui/InteractionController.cpp
    src/ui/PaletteManager.cpp
    src/ui/SceneRenderer.cpp
    src/ui/Toolbar.cpp
    src/ui/UIManager.cpp
    src/ui/WirePainter.cpp
    src/ui/WireRouter.cpp
    src/ui/commands/AddGateCommand.cpp
    src/ui/commands/AddWireCommand.cpp
    src/ui/commands/DeleteSelectionCommand.cpp
    src/ui/commands/DuplicateSelectionCommand.cpp
    src/ui/commands/MoveGatesCommand.cpp
)

set(PROJECT_HEADERS
    include/app/Application.h
    include/app/Config.h
    include/core/DerivedGates.h
    include/core/GateSymbolGeometry.h
    include/core/GatePin.h
    include/core/InputSource.h
    include/core/LogicGate.h
    include/core/OutputSink.h
    include/core/Wire.h
    include/simulation/CircuitSimulator.h
    include/ui/CommandPalette.h
    include/ui/CommandStack.h
    include/ui/DesignTokens.h
    include/ui/EditorCommand.h
    include/ui/EditorSelection.h
    include/ui/GateFactory.h
    include/ui/GateGeometry.h
    include/ui/GatePainter.h
    include/ui/InspectorPanel.h
    include/ui/InteractionController.h
    include/ui/InteractionHelpers.h
    include/ui/PaletteManager.h
    include/ui/SceneRenderer.h
    include/ui/Toolbar.h
    include/ui/UIManager.h
    include/ui/WirePainter.h
    include/ui/WireRouter.h
    include/ui/commands/AddGateCommand.h
    include/ui/commands/AddWireCommand.h
    include/ui/commands/DeleteSelectionCommand.h
    include/ui/commands/DuplicateSelectionCommand.h
    include/ui/commands/MoveGatesCommand.h
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif()

if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

enable_testing()

set(TEST_SOURCES
    tests/tests.cpp
    src/core/DerivedGates.cpp
    src/core/GateSymbolGeometry.cpp
    src/core/GatePin.cpp
    src/core/InputSource.cpp
    src/core/LogicGate.cpp
    src/core/OutputSink.cpp
    src/core/Wire.cpp
    src/simulation/CircuitSimulator.cpp
    src/ui/CommandStack.cpp
    src/ui/GateFactory.cpp
    src/ui/GateGeometry.cpp
    src/ui/WireRouter.cpp
    src/ui/commands/DuplicateSelectionCommand.cpp
)

add_executable(logic-gates-tests ${TEST_SOURCES})
target_include_directories(logic-gates-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(logic-gates-tests PRIVATE raylib)

if(APPLE)
    target_link_libraries(logic-gates-tests PRIVATE "-framework IOKit")
    target_link_libraries(logic-gates-tests PRIVATE "-framework Cocoa")
    target_link_libraries(logic-gates-tests PRIVATE "-framework OpenGL")
endif()

add_test(NAME logic-gates-tests COMMAND logic-gates-tests)

message(STATUS "Configuration:")
message(STATUS "  C++ Standard: C++17")
message(STATUS "  Project: ${PROJECT_NAME}")
message(STATUS "  Version: ${PROJECT_VERSION}")

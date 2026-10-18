cmake_minimum_required(VERSION 3.20)
project(shell-emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(shellcore
    src/config/ArgsParser.cpp
    src/config/ConfigFileReader.cpp
    src/config/ConfigLoader.cpp
    src/shell/Environment.cpp
    src/shell/PromptBuilder.cpp
    src/shell/Lexer.cpp
    src/shell/Parser.cpp
    src/shell/Shell.cpp
    src/commands/CommandRegistry.cpp
    src/commands/LsCommand.cpp
    src/commands/CdCommand.cpp
    src/commands/ExitCommand.cpp
    src/log/JsonLogger.cpp
    src/script/ScriptRunner.cpp
)

target_include_directories(shellcore PUBLIC src)

add_executable(shell-emulator src/main.cpp)
target_link_libraries(shell-emulator PRIVATE shellcore)

add_executable(shell_tests
    tests/tests.cpp
)

target_include_directories(shell_tests PRIVATE tests)
target_link_libraries(shell_tests PRIVATE shellcore)

if(WIN32)
    target_link_libraries(shellcore PUBLIC ws2_32)
endif()

if(MSVC)
    target_compile_options(shellcore PRIVATE /W4 /permissive-)
    target_compile_options(shell-emulator PRIVATE /W4 /permissive-)
    target_compile_options(shell_tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(shellcore PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(shell-emulator PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(shell_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME shell_tests COMMAND shell_tests)

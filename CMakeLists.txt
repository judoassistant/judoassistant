cmake_minimum_required(VERSION 3.1.0)

project(qutejudo)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

# qutejudo executable
add_executable(qutejudo "src/qutejudo.cpp")

set_property(TARGET qutejudo PROPERTY CXX_STANDARD 17)
set_property(TARGET qutejudo PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(qutejudo Qt5::Widgets)

cmake_minimum_required(VERSION 3.1.0)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()

project(qutejudo)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

include_directories(src)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(src/applications)
add_subdirectory(src/stores)
add_subdirectory(src/widgets)

# qutejudo executable
set(qutejudo_SOURCES ${SRCS} src/qutejudo.cpp)
add_executable(qutejudo ${qutejudo_SOURCES})

set_property(TARGET qutejudo PROPERTY CXX_STANDARD 17)
set_property(TARGET qutejudo PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(qutejudo Qt5::Widgets Qt5::Core)

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

# Setup cereal
find_package(cereal REQUIRED)

# Setup qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets CONFIG REQUIRED)

# Setup boost
find_package(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Add subdirectories
include_directories(src)

add_subdirectory(src)
add_subdirectory(src/actions)
add_subdirectory(src/applications)
add_subdirectory(src/stores)
add_subdirectory(src/widgets)

# qutejudo executable
set(qutejudo_SOURCES ${SRCS} src/qutejudo.cpp)
add_executable(qutejudo ${qutejudo_SOURCES})

set_property(TARGET qutejudo PROPERTY CXX_STANDARD 17)
set_property(TARGET qutejudo PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(qutejudo Qt5::Widgets Qt5::Core ${Boost_LIBRARIES})

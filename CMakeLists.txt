cmake_minimum_required(VERSION 3.10)
project(ProjectExample VERSION 0.1.0 LANGUAGES C CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to the pre-built dependencies
set(DEPENDENCY_DIR ${CMAKE_SOURCE_DIR}/_dependency)
list(INSERT CMAKE_INCLUDE_PATH 0 ${DEPENDENCY_DIR}/include)
list(INSERT CMAKE_LIBRARY_PATH 0 ${DEPENDENCY_DIR}/lib)
list(INSERT CMAKE_LIBRARY_PATH 0 ${DEPENDENCY_DIR}/lib64)
list(INSERT CMAKE_PROGRAM_PATH 0 ${DEPENDENCY_DIR}/bin)
include_directories(SYSTEM ${DEPENDENCY_DIR}/include)
link_directories(
    ${DEPENDENCY_DIR}/lib
    ${DEPENDENCY_DIR}/lib64
)

# Add the cmake directory to the module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


# Find Glog
find_package(Glog REQUIRED)

# Include directories
include_directories(${Glog_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)  # Add src directory to include paths

# Add the executable
add_executable(project_example src/main.cpp)

# Link libraries
target_link_libraries(project_example ${Glog_LIBRARY} pthread)

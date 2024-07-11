cmake_minimum_required(VERSION 3.10)
project(bitcask VERSION 0.1.0 LANGUAGES C CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Enable compressed debug info and other compile options
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")

# # Option to enable compressed debug info
# option(ENABLE_COMPRESSED_DEBUG_INFO "Enable compressed debug information" ON)
# if(ENABLE_COMPRESSED_DEBUG_INFO)
#   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gz")
#   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -gz")
# endif()


# Set the output directory for compiled binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

# Find Gflags
find_package(Gflags REQUIRED)
# Find Glog
find_package(Glog REQUIRED)
# Find Libunwind
find_package(Libunwind REQUIRED)
# Find Googletest
find_package(Googletest REQUIRED)

# Include directories
include_directories(${Gflags_INCLUDE_DIR})
include_directories(${Glog_INCLUDE_DIR})
include_directories(${Googletest_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})  # Add src directory to include paths

# Add the utils library
add_subdirectory(utils)

# Add the db library
add_subdirectory(db)

# Add the static bitcask library
add_library(bitcask STATIC
    $<TARGET_OBJECTS:db_obj>
    $<TARGET_OBJECTS:utils_obj>
)

# Include directories for the bitcask library
target_include_directories(bitcask
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link libraries to bitcask
target_link_libraries(bitcask utils_lib)

# Add the executable
add_executable(example example/main.cpp)

# Include directories for the example executable
target_include_directories(example
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        # ${PROJECT_SOURCE_DIR}/db
)

# Link libraries to the example executable
target_link_libraries(example bitcask fmt glog gflags ${LIBUNWIND_LIBRARIES} pthread)



# Include the test subdirectory
add_subdirectory(db/test)

# Enable testing
enable_testing()

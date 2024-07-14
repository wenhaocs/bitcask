find_path(Benchmark_INCLUDE_DIR NAMES benchmark)
find_library(Benchmark_LIBRARY NAMES libbenchmark.a libbenchmark_main.a)

if(Benchmark_INCLUDE_DIR AND Benchmark_LIBRARY)
    set(Benchmark_FOUND TRUE)
    mark_as_advanced(
        Benchmark_INCLUDE_DIR
        Benchmark_LIBRARY
    )
endif()

if(NOT Benchmark_FOUND)
    message(FATAL_ERROR "Googlebenchmark doesn't exist")
else()
    message(STATUS "Found Googlebenchmark")
endif()


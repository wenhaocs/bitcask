## Clone
   git clone --recurse-submodules https://github.com/wenhaocs/ProjectExample.git
## Prerequisites
cmake 3.10 minimum
gcc and g++
make

Note: Only tested in Ubuntu 20.04.6 with gcc 9.4.0 and g++ 9.4.0.

## Build dependency
   ./build-dependency.sh
   You can build individual dependency as well.
   ```
        --autoconf) build_autoconf ;;
        --autoconf-archive) build_autoconf_archive ;;
        --automake) build_automake ;;
        --libtool) build_libtool ;;
        --libunwind) build_libunwind ;;
        --fmt) build_fmt ;;
        --gflags) build_gflags ;;
        --glog) build_glog ;;
        --gtest) build_gtest ;;
        --benchmark) build_benchmark ;;
        --all) build_all=true ;;
   ```
## Build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

## How to use
Find bin/libbitcask.a in the build folder. Include all files in /include and link libbitcask.a in your cpp project.

## Unit tests
All tests are in db/tests folder. Please refer to the CMakeLists.txt inside about the unit test name, and compile them accordingly. E.g., `make db_impl_test`. Or compile all of them by `make -j<N>`. Unit tests are in bin/test folder.

## Benchmark
Based on google benchmark. Run `make -j<N> benchmark_db` to build the benchmark. Run `./benchmark_db --benchmark_filter=<benchmark name>` to run individual benchmark.

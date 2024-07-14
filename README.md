## Clone
   git clone --recurse-submodules https://github.com/wenhaocs/ProjectExample.git
## Build dependency
   ./build-dependency.sh
## Build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

## How to use
Find bin/libbitcask.a in the build folder. Include all files in /include and link libbitcask.a in your cpp project.

## Unit tests
All tests are in db/tests folder. Please refer to the CMakeLists.txt inside about the unit test name, and compile them accordingly. E.g., `make db_impl_test`. Or compile all of them by `make -j<N>`. Unit tests are in bin/test folder.

## Benchmark
Based on google benchmark. Run `make -j<N> benchmark_db` to build the benchmark. Run `./benchmark_db --benchmark_filter=<benchmark name>` to run individual benchmark.

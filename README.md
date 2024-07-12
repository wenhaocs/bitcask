## Clone
   git clone --recurse-submodules https://github.com/wenhaocs/ProjectExample.git
## Build dependency
   ./build-dependency.sh
## Build
cmake -DCMAKE_BUILD_TYPE=Release ..

make

## Unit tests
All tests are in db/tests folder. Please refer to the CMakeLists.txt inside about the unit test name, and compile them accordingly. E.g., `make db_impl_test`

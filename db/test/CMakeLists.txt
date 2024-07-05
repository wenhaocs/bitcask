# db/test/CMakeLists.txt

# Add the test executable
add_executable(log_record_test LogRecordTest.cpp)
set_target_properties(
    log_record_test
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test
)

# Include directories for the test executable
target_include_directories(log_record_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/db
        ${PROJECT_SOURCE_DIR}/utils
)

# Link libraries to the test executable
target_link_libraries(log_record_test $<TARGET_OBJECTS:db_obj> $<TARGET_OBJECTS:utils_obj> gtest gtest_main fmt glog gflags ${LIBUNWIND_LIBRARIES} pthread)

# Add a test to CTest
add_test(NAME log_record_test COMMAND log_record_test)

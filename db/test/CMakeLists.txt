# db/test/CMakeLists.txt

# log record test
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



# data file test
add_executable(data_file_test DataFileTest.cpp)
set_target_properties(
    data_file_test
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test
)

# Include directories for the test executable
target_include_directories(data_file_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/db
        ${PROJECT_SOURCE_DIR}/utils
)

# Link libraries to the test executable
target_link_libraries(data_file_test $<TARGET_OBJECTS:db_obj> $<TARGET_OBJECTS:utils_obj> gtest gtest_main fmt glog gflags ${LIBUNWIND_LIBRARIES} pthread)

# Add a test to CTest
add_test(NAME data_file_test COMMAND data_file_test)


# hash index test
add_executable(hash_index_test HashIndexTest.cpp)
set_target_properties(
    hash_index_test
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test
)

# Include directories for the test executable
target_include_directories(hash_index_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/db
        ${PROJECT_SOURCE_DIR}/utils
)

# Link libraries to the test executable
target_link_libraries(hash_index_test $<TARGET_OBJECTS:db_obj> $<TARGET_OBJECTS:utils_obj> gtest gtest_main fmt glog gflags ${LIBUNWIND_LIBRARIES} pthread)

# Add a test to CTest
add_test(NAME hash_index_test COMMAND hash_index_test)


# db impl test
add_executable(db_impl_test DBImplTest.cpp)
set_target_properties(
    db_impl_test
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test
)

# Include directories for the test executable
target_include_directories(db_impl_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/db
        ${PROJECT_SOURCE_DIR}/utils
)

# Link libraries to the test executable
target_link_libraries(db_impl_test $<TARGET_OBJECTS:db_obj> $<TARGET_OBJECTS:utils_obj> gtest gtest_main fmt glog gflags ${LIBUNWIND_LIBRARIES} pthread)

# Add a test to CTest
add_test(NAME db_impl_test COMMAND db_impl_test)
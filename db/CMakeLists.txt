add_library(db_obj OBJECT
    DBImpl.cpp
    LogRecord.cpp
    DataFile.cpp
)

# Include directories for the bitcask library
target_include_directories(db_obj
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
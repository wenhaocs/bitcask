add_library(utils_obj OBJECT
    Crc.cpp
    NamedThread.cpp
    TscHelper.cpp
    WallClock.cpp
    Helper.cpp
)

target_include_directories(utils_obj
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
cmake_minimum_required(VERSION 3.10)
INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR})

ADD_EXECUTABLE(trace-reader trace-reader.c)
ADD_EXECUTABLE(model-reader model-reader.c)
# ADD_TARGET_COMPILE_FLAGS("-Wno-format")
TARGET_LINK_LIBRARIES(trace-reader ROSS m)
TARGET_LINK_LIBRARIES(model-reader ROSS m)

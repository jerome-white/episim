cmake_minimum_required(VERSION 3.10)
INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR})

SET(tr_srcs
  trace-reader.c
)

ADD_EXECUTABLE(trace-reader trace-reader.c ${tr_srcs})
# ADD_TARGET_COMPILE_FLAGS("-Wno-format")
TARGET_LINK_LIBRARIES(trace-reader ROSS m)

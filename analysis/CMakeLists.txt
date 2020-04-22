cmake_minimum_required(VERSION 3.10)
INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR})

ADD_EXECUTABLE(event-reader event-reader.c)
ADD_EXECUTABLE(model-reader model-reader.c)
# ADD_TARGET_COMPILE_FLAGS("-Wno-format")
TARGET_LINK_LIBRARIES(event-reader ROSS m)
TARGET_LINK_LIBRARIES(model-reader ROSS m)

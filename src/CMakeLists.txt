cmake_minimum_required(VERSION 3.10)
INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR})

SET(epi_srcs
  main.c
  model.c
  util.c
)

ADD_EXECUTABLE(epi-model main.c ${epi_srcs})
# ADD_TARGET_COMPILE_FLAGS("-Wno-format")
TARGET_LINK_LIBRARIES(epi-model ROSS m)

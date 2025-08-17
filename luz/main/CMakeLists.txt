idf_component_register(
  INCLUDE_DIRS
    "."
  SRCS
    "ble.cc"
    "database.cc"
    "led.cc"
    "luz.cc"
    "protocol.cc"
  REQUIRES
    bt
    nvs_flash
    driver
)

string(REPLACE " -Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
remove_definitions(-Werror)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23 -Wno-error)
remove_definitions(-Werror)

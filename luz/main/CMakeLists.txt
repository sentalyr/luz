idf_component_register(
  INCLUDE_DIRS
    "."
  SRCS
    "ble.cc"
    "buffer.cc"
    "database.cc"
    "led.cc"
    "luz.cc"
    "protocol.cc"
  REQUIRES
    bt
    nvs_flash
    driver
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)

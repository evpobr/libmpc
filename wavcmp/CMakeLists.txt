include_directories(${libmpc_SOURCE_DIR}/libwavformat)
link_directories(${libmpc_BINARY_DIR}/libwavformat)

add_executable(wavcmp wavcmp)

target_link_libraries(wavcmp wavformat_static)

install(TARGETS wavcmp RUNTIME DESTINATION bin)

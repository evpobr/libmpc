include_directories(${libmpc_SOURCE_DIR}/include)

link_directories(${libmpc_BINARY_DIR}/libmpcdec)
link_directories(${libmpc_BINARY_DIR}/libmpcenc)

add_executable(mpcchap mpcchap dictionary iniparser ${libmpc_SOURCE_DIR}/common/tags)

target_link_libraries(mpcchap mpcdec_static)
target_link_libraries(mpcchap mpcenc_static)

if(NOT MSVC)
target_link_libraries(mpcchap m)
endif(NOT MSVC)


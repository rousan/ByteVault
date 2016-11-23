cmake_minimum_required(VERSION 3.6)
project(ByteVault)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/user.c src/main.c src/logger.c src/include/logger.h src/include/utils.h src/utils.c src/include/database.h src/database.c src/include/encryption.h src/encryption.c src/include/user.h src/include/viewer.h src/viewer.c src/include/sqlite/sqlite3.h src/include/constants.h src/include/pragma.h src/include/common.h src/include/commands.h src/commands.c src/command_actions.c src/include/command_actions.h src/include/error_messages.h src/linked_list.c src/include/linked_list.h src/include/resource.h)

add_executable(ByteVault ${SOURCE_FILES})
add_executable(BV ${SOURCE_FILES})

target_link_libraries(ByteVault "${CMAKE_CURRENT_BINARY_DIR}/sqlite3.dll")
target_link_libraries(BV "${CMAKE_CURRENT_BINARY_DIR}/sqlite3.dll")

cmake_minimum_required(VERSION 3.27)
project(IPC_Publish_Subscribe C)

set(CMAKE_C_STANDARD 17)

add_executable(klient inf155965_155845_k.c inf155965_155845_message_types.h)
target_link_libraries(klient)

add_executable(serwer inf155965_155845_s.c inf155965_155845_message_types.h)
target_link_libraries(serwer)
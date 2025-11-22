cmake_minimum_required(VERSION 3.16)
project (amx C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(c/libwav)

add_executable(amx-core
	c/src/main.c
)

target_include_directories(amx-core PRIVATE
	c/include
	c/libwav/include
)

target_link_libraries(amx-core PRIVATE amxwav m)

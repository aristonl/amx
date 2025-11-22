add_library(amxwav STATIC
	src/wav.c
)

target_include_directories(amxwav PUBLIC
	$(CMAKE_CURRENT_SOURCE_DIR}/include
)

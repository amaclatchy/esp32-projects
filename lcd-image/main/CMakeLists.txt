file(GLOB_RECURSE srcs "lcd-image.c" "src/*.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "./include")
file(GLOB_RECURSE srcs "button-interrupt.c" "src/*.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "./include")

file(GLOB_RECURSE srcs "euchre-singleboard-proto.c" "src/*.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "./include")

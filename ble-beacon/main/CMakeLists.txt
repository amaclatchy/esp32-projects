file(GLOB_RECURSE srcs "ble-beacon.c" "src/*.c")

idf_component_register(SRCS "${srcs}"
                       PRIV_REQUIRES bt nvs_flash
                       INCLUDE_DIRS "./include")
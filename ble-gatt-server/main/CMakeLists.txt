file(GLOB_RECURSE srcs "ble-gatt-server.c" "src/*.c")

idf_component_register(SRCS "${srcs}"
                       PRIV_REQUIRES bt nvs_flash driver
                       INCLUDE_DIRS "./include")

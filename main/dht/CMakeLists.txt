set(srcs "DHT11.c")
add_library(dht DHT11.c)
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES)


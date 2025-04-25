idf_component_register(SRCS "SD.c"
                    INCLUDE_DIRS "."
                    REQUIRES
                    driver
                    fatfs
                    vfs
                    )

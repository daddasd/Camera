idf_component_register(SRCS "myES7210.c"
                    INCLUDE_DIRS "."
                    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/managed_components/espressif__es7210/include
                    REQUIRES
                    driver
                    SD # 添加这个路径
                    )

#pragma once

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <errno.h>

#define SD_PIN_CLK 47
#define SD_PIN_CMD 48
#define SD_PIN_D0 21

#define MOUNT_POINT "/sdcard"
#define EXAMPLE_MAX_CHAR_SIZE 128

void SD_Init(void);
void SD_Unmount(void);
esp_err_t SD_Write_Data(const char *path, const char *filename, const char *data, size_t length);
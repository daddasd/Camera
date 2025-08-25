/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-04-22 14:38:52
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-22 18:11:54
 * @FilePath: \boot_key\components\SD\SD.C
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "SD.h"

/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-04-19 20:43:46
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-22 15:50:28
 * @FilePath: \boot_key\main\main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

static const char *TAG = "SD";
// 用于保存已挂载的 SD 卡指针
static sdmmc_card_t *card = NULL;
/**
 * @brief 不覆盖内容的写文件函数
 *
 * @param path  文件路径
 * @param data  写进的内容
 * @return esp_err_t
 */
static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}
/**
 * @brief 读取第一行的内容
 *
 * @param path
 * @return * esp_err_t
 */
static esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}
/**
 * @brief 读取全部文件内容
 *
 * @param path  文件路径
 * @return esp_err_t
 */
static esp_err_t s_example_read_file_all_lines(const char *path)
{
    ESP_LOGI(TAG, "Reading entire file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }

    char line[EXAMPLE_MAX_CHAR_SIZE];
    // 一次读取一行，直到文件末尾
    while (fgets(line, sizeof(line), f) != NULL)
    {
        // 去掉末尾换行（可选）
        char *pos = strchr(line, '\n');
        if (pos)
            *pos = '\0';
        ESP_LOGI(TAG, "Read line: %s", line);
        // 如果想把每行内容累积到一个大缓冲，或者打印到屏幕、存到其它地方，都在这里处理
    }

    fclose(f);
    ESP_LOGI(TAG, "Finished reading file");
    return ESP_OK;
}
/**
 * @brief 初始化SDO并打印SD卡信息
 *
 */
void SD_Init(void)
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.clk = SD_PIN_CLK;
    slot_config.cmd = SD_PIN_CMD;
    slot_config.d0 = SD_PIN_D0;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). ",
                     esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    sdmmc_card_print_info(stdout, card);
}

void SD_Unmount(void)
{
    if (card != NULL)
    {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
        card = NULL;
        ESP_LOGI(TAG, "SD card unmounted");
    }
    else
    {
        ESP_LOGW(TAG, "SD card not mounted, nothing to unmount");
    }
}
// 递归创建目录函数
static esp_err_t create_directory_recursive(const char *dir)
{
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);

    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0; // 去掉最后的 '/'

    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            mkdir(tmp, 0775); // 忽略已存在的错误
            *p = '/';
        }
    }
    mkdir(tmp, 0775); // 创建最后一级目录
    return ESP_OK;
}

esp_err_t SD_Write_Data(const char *path, const char *filename, const char *data, size_t length)
{
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s%s/%s", MOUNT_POINT, path, filename);

    // 创建目录
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "%s%s", MOUNT_POINT, path);
    create_directory_recursive(dir_path);

    // 打开文件写入
    FILE *f = fopen(full_path, "w");
    if (f == NULL)
    {
        ESP_LOGE("SD_WRITE", "Failed to open file for writing: %s", strerror(errno));
        return ESP_FAIL;
    }

    size_t bytes_written = fwrite(data, 1, length, f);
    fclose(f);

    if (bytes_written != length)
    {
        ESP_LOGE("SD_WRITE", "Write failed (expected %d, actual %d)", length, bytes_written);
        return ESP_FAIL;
    }

    ESP_LOGI("SD_WRITE", "File written successfully: %s", full_path);
    return ESP_OK;
}
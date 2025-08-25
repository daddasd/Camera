/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "myES7210.h"

static const char *TAG = "ES7210";
/**
 * @brief ES7210 I2S 初始化
 * 
 * @return i2s_chan_handle_t 
 */
i2s_chan_handle_t es7210_i2s_init(void)
{
    i2s_chan_handle_t i2s_rx_chan = NULL;
    ESP_LOGI(TAG, "Create I2S receive channel");
    i2s_chan_config_t i2s_rx_conf = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&i2s_rx_conf, NULL, &i2s_rx_chan));

    ESP_LOGI(TAG, "Configure I2S receive channel to TDM mode");
    i2s_tdm_config_t i2s_tdm_rx_conf = {
        .slot_cfg = I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_SAMPLE_BITS, I2S_SLOT_MODE_STEREO, I2S_TDM_SLOT_MASK),
        .clk_cfg = {
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .sample_rate_hz = I2S_SAMPLE_RATE,
            .mclk_multiple = I2S_MCLK_MULTIPLE},
        .gpio_cfg = {.mclk = ES7210_I2S_MCK_IO, .bclk = ES7210_I2S_BCK_IO, .ws = ES7210_I2S_WS_IO,
                     .dout = -1, // ES7210 only has ADC capability
                     .din = ES7210_I2S_DI_IO},
    };

    ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(i2s_rx_chan, &i2s_tdm_rx_conf));

    return i2s_rx_chan;
}
void es7210_codec_init(void)
{
    ESP_LOGI(TAG, "Init I2C used to configure ES7210");
    i2c_config_t i2c_conf = {
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = ES7210_I2C_CLK,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM, i2c_conf.mode, 0, 0, 0));

    /* Create ES7210 device handle */
    es7210_dev_handle_t es7210_handle = NULL;
    es7210_i2c_config_t es7210_i2c_conf = {
        .i2c_port = I2C_NUM,
        .i2c_addr = ES7210_I2C_ADDR};
    ESP_ERROR_CHECK(es7210_new_codec(&es7210_i2c_conf, &es7210_handle));

    ESP_LOGI(TAG, "Configure ES7210 codec parameters");
    es7210_codec_config_t codec_conf = {
        .i2s_format = I2S_TDM_FORMAT,
        .mclk_ratio = I2S_MCLK_MULTIPLE,
        .sample_rate_hz = I2S_SAMPLE_RATE,
        .bit_width = (es7210_i2s_bits_t)I2S_SAMPLE_BITS,
        .mic_bias = ES7210_MIC_BIAS,
        .mic_gain = ES7210_MIC_GAIN,
        .flags.tdm_enable = true};
    ESP_ERROR_CHECK(es7210_config_codec(es7210_handle, &codec_conf));
    ESP_ERROR_CHECK(es7210_config_volume(es7210_handle, ES7210_ADC_VOLUME));
}

 esp_err_t record_wav(i2s_chan_handle_t i2s_rx_chan)
{
    ESP_RETURN_ON_FALSE(i2s_rx_chan, ESP_FAIL, TAG, "invalid i2s channel handle pointer");
    esp_err_t ret = ESP_OK;

    uint32_t byte_rate = I2S_SAMPLE_RATE * I2S_CHAN_NUM * I2S_SAMPLE_BITS / 8;
    uint32_t wav_size = byte_rate * EXAMPLE_RECORD_TIME_SEC;

    const wav_header_t wav_header =
        WAV_HEADER_PCM_DEFAULT(wav_size, I2S_SAMPLE_BITS, I2S_SAMPLE_RATE, I2S_CHAN_NUM);

    ESP_LOGI(TAG, "Opening file %s", EXAMPLE_RECORD_FILE_PATH);
    FILE *f = fopen(EXAMPLE_SD_MOUNT_POINT EXAMPLE_RECORD_FILE_PATH, "w");
    ESP_RETURN_ON_FALSE(f, ESP_FAIL, TAG, "error while opening wav file");

    /* Write wav header */
    ESP_GOTO_ON_FALSE(fwrite(&wav_header, sizeof(wav_header_t), 1, f), ESP_FAIL, err,
                      TAG, "error while writing wav header");

    /* Start recording */
    size_t wav_written = 0;
    static int16_t i2s_readraw_buff[4096];
    ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_rx_chan), err, TAG, "error while starting i2s rx channel");
    while (wav_written < wav_size)
    {
        if (wav_written % byte_rate < sizeof(i2s_readraw_buff))
        {
            ESP_LOGI(TAG, "Recording: %" PRIu32 "/%ds", wav_written / byte_rate + 1, EXAMPLE_RECORD_TIME_SEC);
        }
        size_t bytes_read = 0;
        /* Read RAW samples from ES7210 */
        ESP_GOTO_ON_ERROR(i2s_channel_read(i2s_rx_chan, i2s_readraw_buff, sizeof(i2s_readraw_buff), &bytes_read,
                                           pdMS_TO_TICKS(1000)),
                          err, TAG, "error while reading samples from i2s");
        /* Write the samples to the WAV file */
        ESP_GOTO_ON_FALSE(fwrite(i2s_readraw_buff, bytes_read, 1, f), ESP_FAIL, err,
                          TAG, "error while writing samples to wav file");
        wav_written += bytes_read;
    }

err:
    i2s_channel_disable(i2s_rx_chan);
    ESP_LOGI(TAG, "Recording done! Flushing file buffer");
    fclose(f);

    return ret;
}

void RecordAudio(void)
{
    i2s_chan_handle_t i2s_rx_chan = es7210_i2s_init();
    es7210_codec_init();
    SD_Init();
    esp_err_t err = record_wav(i2s_rx_chan);
    SD_Unmount();
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Audio was successfully recorded into " EXAMPLE_RECORD_FILE_PATH
                      ". You can now remove the SD card safely");
    }
    else
    {
        ESP_LOGE(TAG, "Record failed, " EXAMPLE_RECORD_FILE_PATH " on SD card may not be playable.");
    }
}
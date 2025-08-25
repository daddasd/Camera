#pragma once

#include <string.h>
#include "sdkconfig.h"
#include "esp_check.h"
#include "driver/i2s_tdm.h"
#include "driver/i2c.h"
#include "es7210.h"
#include "SD.h"
#include "format_wav.h"
/* I2C port and GPIOs */
#define I2C_NUM (0)
#define I2C_SDA_IO (1)
#define I2C_SCL_IO (2)

/* I2S port and GPIOs */
#define ES7210_I2S_NUM (0)
#define ES7210_I2S_MCK_IO (38)
#define ES7210_I2S_BCK_IO (14)
#define ES7210_I2S_WS_IO (13)
#define ES7210_I2S_DI_IO (12)

/* I2S configurations */
#define I2S_TDM_FORMAT (ES7210_I2S_FMT_I2S)
#define I2S_CHAN_NUM (2)
#define I2S_SAMPLE_RATE (48000)
#define I2S_MCLK_MULTIPLE (I2S_MCLK_MULTIPLE_256)
#define I2S_SAMPLE_BITS (I2S_DATA_BIT_WIDTH_32BIT)
#define I2S_TDM_SLOT_MASK (I2S_TDM_SLOT0 | I2S_TDM_SLOT1)

/* ES7210 configurations */
#define ES7210_I2C_ADDR (0x41)
#define ES7210_I2C_CLK (100000)
#define ES7210_MIC_GAIN (ES7210_MIC_GAIN_30DB)
#define ES7210_MIC_BIAS (ES7210_MIC_BIAS_2V87)
#define ES7210_ADC_VOLUME (0)

/* SD card & recording configurations */
#define EXAMPLE_RECORD_TIME_SEC (10)
#define EXAMPLE_SD_MOUNT_POINT "/sdcard"
#define EXAMPLE_RECORD_FILE_PATH "/xzhi.WAV"

i2s_chan_handle_t es7210_i2s_init(void);
void es7210_codec_init(void);
esp_err_t record_wav(i2s_chan_handle_t i2s_rx_chan);
void RecordAudio(void);

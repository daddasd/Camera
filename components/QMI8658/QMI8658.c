#include "QMI8658.h"

static const char *TAG = "QMI8658";

esp_err_t i2c_master_init(void)
{
    int i2c_master_port = BSP_I2C_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BSP_QMI8658_SDA,
        .scl_io_num = BSP_QMI8658_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = BSP_QMI8658_I2C_FREQ_HZ,
    };
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

esp_err_t QMI8658_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(BSP_I2C_NUM, QMI8658_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t QMI8658_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(BSP_I2C_NUM, QMI8658_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

void QMI8658_Init(void)
{
    uint8_t id;
    uint8_t retry_count = 0;
    i2c_master_init();
    QMI8658_register_read(QMI8658_WHO_AM_I, &id, 1); // 先读一次

    while (id != 0x05 && retry_count < 10) // 用 && 保证正确退出
    {
        vTaskDelay(pdMS_TO_TICKS(1000));                // 延时 1s
        QMI8658_register_read(QMI8658_WHO_AM_I, &id, 1); // 再次读取
        retry_count++;
    }

    if (id == 0x05)
    {
        ESP_LOGI(TAG, "QMI8659 Read ID successfully :%d", id); 
    }
    else
    {
        ESP_LOGI(TAG, "QMI8659 Read ID filed");
    }
    QMI8658_register_write_byte(QMI8658_RESET,0xB0);//复位
    vTaskDelay(pdMS_TO_TICKS(20));
    QMI8658_register_write_byte(QMI8658_CTRL1, 0X40);//CTRL1 设置地址自动增加
    QMI8658_register_write_byte(QMI8658_CTRL7, 0X03);//CTRL7 允许加速度和陀螺仪
    QMI8658_register_write_byte(QMI8658_CTRL2, 0X95);//CTRL2 设置ACC 4G 250HZ
    QMI8658_register_write_byte(QMI8658_CTRL3, 0Xd5);//CTRL3 设置GRY 512 DPS 250HZ
}

void QMI8658_Read_AccGry(t_sQMI8658 *p)
{
    uint8_t status, data_ready = 0;
    int16_t buf[6];
    QMI8658_register_read(QMI8658_STATUS0, &status, 1);
    if(status & 0x03)
        data_ready=1;
    if(data_ready)
    {
        data_ready = 0;
        QMI8658_register_read(QMI8658_AX_L, (uint8_t *)buf, 12);
        p->acc_x = buf[0];
        p->acc_y = buf[1];
        p->acc_z = buf[2];
        p->gry_x = buf[3];
        p->gry_y = buf[4];
        p->gry_z = buf[5];
    }
}
// 温度读取函数
void QMI8658_Read_Temperature(t_sQMI8658 *p)
{
    uint8_t temp_data[2];
    QMI8658_register_read(QMI8658_TEMP_L, temp_data, 2);
    int8_t temp_h = (int8_t)temp_data[1];
    uint8_t temp_l = temp_data[0];
    p->temperature = (float)temp_h + (float)temp_l / 256.0f;
}
// 获取XYZ轴的倾角值
void QMI8658_fetch_angleFromAcc(t_sQMI8658 *p)
{
    float temp;

    QMI8658_Read_AccGry(p); // 读取加速度和陀螺仪的寄存器值
    // 根据寄存器值 计算倾角值 并把弧度转换成角度
    temp = (float)p->acc_x / sqrt(((float)p->acc_y * (float)p->acc_y + (float)p->acc_z * (float)p->acc_z));
    p->AngleX = atan(temp) * 57.29578f; // 180/π=57.29578
    temp = (float)p->acc_y / sqrt(((float)p->acc_x * (float)p->acc_x + (float)p->acc_z * (float)p->acc_z));
    p->AngleY = atan(temp) * 57.29578f; // 180/π=57.29578
    temp = sqrt(((float)p->acc_x * (float)p->acc_x + (float)p->acc_y * (float)p->acc_y)) / (float)p->acc_z;
    p->AngleZ = atan(temp) * 57.29578f; // 180/π=57.29578
}
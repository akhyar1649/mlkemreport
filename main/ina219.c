#include "ina219.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define INA219_REG_CONFIG 0x00
#define INA219_REG_SHUNT  0x01
#define INA219_REG_BUS    0x02
#define INA219_REG_POWER  0x03
#define INA219_REG_CURRENT 0x04
#define INA219_REG_CALIB  0x05

#define INA219_BUS_VOLTAGE_LSB_MV 4.0f

static const char *TAG = "ina219";

static esp_err_t ina219_write_reg(ina219_dev_t *dev, uint8_t reg, uint16_t value) {
    uint8_t buf[3] = { reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF) };
    return i2c_master_write_to_device(dev->port, dev->addr, buf, sizeof(buf), pdMS_TO_TICKS(100));
}

static esp_err_t ina219_read_reg(ina219_dev_t *dev, uint8_t reg, uint16_t *value) {
    uint8_t data[2] = {0};
    esp_err_t err = i2c_master_write_read_device(dev->port, dev->addr, &reg, 1, data, 2, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        return err;
    }
    *value = (uint16_t)((data[0] << 8) | data[1]);
    return ESP_OK;
}

esp_err_t ina219_init(ina219_dev_t *dev) {
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    dev->port = (i2c_port_t)CONFIG_INA219_I2C_PORT;
    dev->addr = (uint8_t)CONFIG_INA219_ADDR;
    dev->shunt_ohms = (float)CONFIG_INA219_SHUNT_MILLIOHM / 1000.0f;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_INA219_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_INA219_I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CONFIG_INA219_I2C_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(dev->port, &conf);
    if (err != ESP_OK) {
        return err;
    }

    err = i2c_driver_install(dev->port, conf.mode, 0, 0, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

#if CONFIG_INA219_ADC_12BIT_8S
    uint16_t adc_code = 0x0B; /* 12-bit, 8-sample (4.26 ms) */
#elif CONFIG_INA219_ADC_12BIT_16S
    uint16_t adc_code = 0x0C; /* 12-bit, 16-sample (8.51 ms) */
#else
    uint16_t adc_code = 0x03; /* 12-bit, 1-sample (532 us) */
#endif

    uint16_t config = 0;
    config |= (1 << 13);
    config |= (3 << 11);
    config |= (adc_code << 7);
    config |= (adc_code << 3);
    config |= 0x07;

    float max_expected_amps = (float)CONFIG_INA219_MAX_CURRENT_MA / 1000.0f;
    dev->current_lsb = max_expected_amps / 32768.0f;
    dev->power_lsb = dev->current_lsb * 20.0f;
    uint16_t cal = (uint16_t)(0.04096f / (dev->current_lsb * dev->shunt_ohms));

    err = ina219_write_reg(dev, INA219_REG_CALIB, cal);
    if (err != ESP_OK) {
        return err;
    }

    err = ina219_write_reg(dev, INA219_REG_CONFIG, config);
    if (err != ESP_OK) {
        return err;
    }

    ESP_LOGI(TAG, "INA219 init: addr=0x%02X shunt=%.3f ohm", dev->addr, (double)dev->shunt_ohms);
    return ESP_OK;
}

esp_err_t ina219_read_bus_voltage_mv(ina219_dev_t *dev, float *bus_mv) {
    uint16_t raw = 0;
    esp_err_t err = ina219_read_reg(dev, INA219_REG_BUS, &raw);
    if (err != ESP_OK) {
        return err;
    }
    uint16_t value = raw >> 3;
    *bus_mv = (float)value * INA219_BUS_VOLTAGE_LSB_MV;
    return ESP_OK;
}

esp_err_t ina219_read_current_ma(ina219_dev_t *dev, float *current_ma) {
    uint16_t raw = 0;
    esp_err_t err = ina219_read_reg(dev, INA219_REG_CURRENT, &raw);
    if (err != ESP_OK) {
        return err;
    }
    int16_t signed_raw = (int16_t)raw;
    *current_ma = (float)signed_raw * dev->current_lsb * 1000.0f;
    return ESP_OK;
}

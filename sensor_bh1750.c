#include <stdio.h>
#include "sensor_bh1750.h"
#include "partition_nvs.h"

#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#define TAG "BH1750"

// configs
static uint32_t _i2c_port;
static uint32_t _i2c_address;
static uint32_t _rst_pin;

// readings
static float _raw_light_intensity;
static float _real_light_intensity;

// outputs for main app
char buffer[100];
char *s;

// about calibration
static char *namespace = "bh1750";
static char *coef0_key = "c0";
static float coef0 = 1.0;

static void update()
{
    uint8_t data_h, data_l;

    //gpio_set_level(_rst_pin, 1);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                            // Command link Create
    i2c_master_start(cmd);                                                   // Start bit
    i2c_master_write_byte(cmd, _i2c_address << 1 | WRITE_BIT, ACK_CHECK_EN); // Write an single byte address
    i2c_master_write_byte(cmd, BH1750_CMD_START, ACK_CHECK_EN);
    i2c_master_stop(cmd); // Stop bit
    esp_err_t err = i2c_master_cmd_begin(_i2c_port, cmd, TICK_DELAY);
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "bh1750_get part1: %s", esp_err_to_name(err));
    }

    vTaskDelay(30 / portTICK_RATE_MS);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, _i2c_address << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data_h, ACK_VAL);
    i2c_master_read_byte(cmd, &data_l, NACK_VAL);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(_i2c_port, cmd, TICK_DELAY);
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "bh1750_get part2: %s", esp_err_to_name(err));
    }

    _raw_light_intensity = (data_h << 8 | data_l) / 1.2; // calibracion
    _real_light_intensity = _raw_light_intensity * coef0;
}

static float getRaw()
{
    return _raw_light_intensity;
}
static float getReal()
{
    return _real_light_intensity;
}
bool check_bh1750()
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, _i2c_address << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, BH1750_CMD_START, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(_i2c_port, cmd, TICK_DELAY);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
        return true;
    else
    {
        ESP_LOGE(TAG, "%s", __func__);
        return false;
    }
}

void init_bh1750(bool _ft, uint32_t *arg)
{
    if (_ft == true)
    {
        _i2c_port = arg[0];
        _i2c_address = arg[1];
        _rst_pin = arg[2];
        gpio_set_level(_rst_pin, 1);

        ESP_LOGI(TAG, "i2c_port:%u || i2c_addrr:%u || rst_pin:%u", _i2c_port, _i2c_address, _rst_pin);
    };

    // Not necesary in this case
}

void print_bh1750()
{
    if (check_bh1750())
        printf("BH1750 : OK");
    else
        printf("BH1750 : Not detected (Light sensor)");
}

char *get_bh1750(int a, bool raw)
{
    update();

    s = " ";
    sprintf(buffer, "%.02f", raw ? getRaw() : getReal());
    s = buffer;
    return s;
}

bool calibrate_bh1750(int a, float *raws, float *instruments)
{
    switch (a)
    {
    case 0: // light intensity
        coef0 = 1.0 * raws[0] / instruments[0];
        break;
    default:
        return false;
    }

    return true;
}

bool save_coefs_bh1750(int a)
{
    if (a == 0)
    {
        int32_t coef0_x1000 = (int32_t)(coef0 * 1000.0);
        esp_err_t err0 = nvs_set_int32(coef0_x1000, namespace, coef0_key);
        bool coefs_saved = (err0 == ESP_OK);
        return coefs_saved;
    }
    else
    {
        return false;
    }
}
bool load_coefs_bh1750(int a)
{
    if (a == 0)
    {
        uint32_t coef0_x1000;
        esp_err_t err = nvs_get_uint32(&coef0_x1000, namespace, coef0_key);
        if (err == ESP_OK)
        {
            coef0 = coef0_x1000 / 1000.0;
            printf("coeficients of bh1750-light => coef[0]:%.3f\n", coef0);
            return true;
        }
        printf("coeficients of bh1750-light => error loading\n");
        return false;
    }
    else
    {
        return false;
    }
}
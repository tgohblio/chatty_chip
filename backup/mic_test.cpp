#include "Arduino.h"
#include <driver/i2s.h>

// you shouldn't need to change these settings
#define SAMPLE_BUFFER_SIZE 512
#define SAMPLE_RATE 8000
// most microphones will probably default to left channel but you may need to tie the L/R pin low
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
// either wire your microphone to the same pins or change these to match your wiring
#define I2S_MIC_SCK GPIO_NUM_32
#define I2S_MIC_WS  GPIO_NUM_25
#define I2S_MIC_SD GPIO_NUM_33
#define I2S_MIC_I2S_PORT I2S_NUM_0

// i2s config for INMP441 mic
i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_MIC_CHANNEL,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

// and don't mess around with this
i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SCK,
    .ws_io_num = I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SD
};

void setup( void )
{
    uint8_t status = ESP_FAIL;
    // we need serial output for the plotter
    Serial.begin(115200);
    // start up the I2S peripheral
    status = i2s_driver_install(I2S_MIC_I2S_PORT, &i2s_config, 0, NULL);
    if( status == ESP_OK )
    {
        Serial.printf("I2S config OK\r\n");
        i2s_set_pin(I2S_MIC_I2S_PORT, &i2s_mic_pins);
    }
    else{
        Serial.printf("I2S config FAIL\r\n");
    }
}

int32_t raw_samples[SAMPLE_BUFFER_SIZE] = { 0 };
size_t bytes_read = 0;
esp_err_t result = ESP_FAIL;
int samples_read = 0;

void loop( void )
{
    // read from the I2S device
    result = i2s_read(I2S_MIC_I2S_PORT, raw_samples, sizeof(raw_samples), &bytes_read, portMAX_DELAY);
    if( result == ESP_OK )
    {
        Serial.printf("bytes_read = %d bytes\r\n", bytes_read);
        samples_read = bytes_read / sizeof(int32_t);
        // dump the samples out to the serial channel.
        for (int i = 0; i < samples_read; i++)
        {
            Serial.printf("[%d] = %ld\n", i, raw_samples[i]);
        }
    }
    else
    {
        Serial.printf("Read error: %d\n", result);
    }
}
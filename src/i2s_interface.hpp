  
/*
 * this file includes all required function to setup and drive the i2s interface
 *
 * Author: Marcel Licence
 */

#ifdef __CDT_PARSER__
#include <cdt.h>
#endif

#include <driver/i2s.h>

/*
 * no dac not tested within this code
 * - it has the purpose to generate a quasy analog signal without a DAC
 */
//#define I2S_NODAC


const i2s_port_t i2s_port_number = I2S_NUM_0;

bool i2s_write_stereo_samples(float *fl_sample, float *fr_sample)
{
    static union sampleTUNT
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleDataU;
    /*
     * using RIGHT_LEFT format
     */
    sampleDataU.ch[0] = int16_t(*fr_sample * 16383.0f); /* some bits missing here */
    sampleDataU.ch[1] = int16_t(*fl_sample * 16383.0f);

    static size_t bytes_written = 0;

    i2s_write(i2s_port_number, (const char *)&sampleDataU.sample, 4, &bytes_written, portMAX_DELAY);

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool i2s_write_mono_samples_buff(float *fsample, const int buffLen)
{
    int16_t sample[SAMPLE_BUFFER_SIZE];

    for (int n = 0; n < buffLen; n++)
    {
        /*
         * using RIGHT_LEFT format
         */
        sample[n] = int16_t(fsample[n] * 16383.0f); /* some bits missing here */
    }

    static size_t bytes_written = 0;

    i2s_write(i2s_port_number, (const char *)&sample[0], 2 * buffLen, &bytes_written, portMAX_DELAY);

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool i2s_write_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen)
{
    static union sampleTUNT
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleDataU[SAMPLE_BUFFER_SIZE];

    for (int n = 0; n < buffLen; n++)
    {
        /*
         * using RIGHT_LEFT format
         */
        sampleDataU[n].ch[0] = int16_t(fl_sample[n] * 16383.0f); /* some bits missing here */
        sampleDataU[n].ch[1] = int16_t(fr_sample[n] * 16383.0f);
    }

    static size_t bytes_written = 0;

    if(i2s_write(i2s_port_number, (const char *)&sampleDataU[0].sample, 4 * buffLen, &bytes_written, portMAX_DELAY) != ESP_OK)
        Serial.println("i2s write error!");

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void i2s_read_stereo_samples(float *fl_sample, float *fr_sample)
{
    static size_t bytes_read = 0;

    static union
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleData;

    i2s_read(i2s_port_number, (char *)&sampleData.sample, 4, &bytes_read, portMAX_DELAY);

    //sampleData.ch[0] &= 0xFFFE;
    //sampleData.ch[1] &= 0;

    /*
     * using RIGHT_LEFT format
     */
    *fr_sample = ((float)sampleData.ch[0] * (5.5f / 65535.0f));
    *fl_sample = ((float)sampleData.ch[1] * (5.5f / 65535.0f));
}

void i2s_read_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen)
{
    static size_t bytes_read = 0;

    static union
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleData[SAMPLE_BUFFER_SIZE];

    i2s_read(i2s_port_number, (char *)&sampleData[0].sample, 4 * buffLen, &bytes_read, portMAX_DELAY);

    //sampleData.ch[0] &= 0xFFFE;
    //sampleData.ch[1] &= 0;

    for (int n = 0; n < buffLen; n++)
    {
        /*
         * using RIGHT_LEFT format
         */
        //fr_sample[n] = ((float)sampleData[n].ch[0] * (5.5f / 65535.0f));
        //fl_sample[n] = ((float)sampleData[n].ch[1] * (5.5f / 65535.0f));
        fr_sample[n] = ((float)sampleData[n].ch[0] / (16383.0f));
        fl_sample[n] = ((float)sampleData[n].ch[1] / (16383.0f));
    }
}

i2s_config_t i2s_configuration =
{
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // | I2S_MODE_DAC_BUILT_IN
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // default interrupt priority
    .dma_buf_count = 2,
    .dma_buf_len = 128,
    // .use_apll = true,
};

i2s_pin_config_t pins =
{
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num =  I2S_WCLK_PIN,
    .data_out_num = I2S_DOUT_PIN,
    .data_in_num = I2S_DIN_PIN
};

void setup_i2s()
{
    Serial.print("Driver install: ");Serial.println(i2s_driver_install(i2s_port_number, &i2s_configuration, 0, NULL));
    Serial.print("Pin set: ");Serial.println(i2s_set_pin(I2S_NUM_0, &pins));
    Serial.print("Rate set: ");Serial.println(i2s_set_sample_rates(i2s_port_number, SAMPLE_RATE));
    Serial.print("I2S start: ");Serial.println(i2s_start(i2s_port_number));
    REG_WRITE(PIN_CTRL, 0xFFFFFFF0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
}
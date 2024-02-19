#ifndef CONFIG_HPP
#define CONFIG_HPP
#endif

// MIDI
#define MIDI_RX_PIN 19
#define NOTE_BEGIN 48

// SD
#define SD_CS 13

// I2C
#define I2C1_SDA 21
#define I2C1_SCL 22
#define I2C1_SPEED 1000000

// I2S
#define I2S_BCLK_PIN 27
#define I2S_WCLK_PIN 26
#define I2S_DOUT_PIN 25
#define I2S_DIN_PIN -1

#define CHANNEL_COUNT   2
#define WORD_SIZE   16
#define I2S1CLK (512*SAMPLE_RATE)
#define BCLK    (SAMPLE_RATE*CHANNEL_COUNT*WORD_SIZE)
#define LRCK    (SAMPLE_RATE*CHANNEL_COUNT)

// Audio general
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE_16BIT
#define SAMPLE_BUFFER_SIZE 64
#define AUDIBLE_LIMIT   (0.25f/32768.0f)
#define NUM_PLAYERS 8
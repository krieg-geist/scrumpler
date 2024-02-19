#include <Arduino.h>
#include <MIDI.h>
#include <Wire.h>

#include "player.hpp"
#include "config.hpp"
#include "i2s_interface.hpp"
#include "midi_note_handler.hpp"

TwoWire I2C1 = TwoWire(0); //I2C1 bus
// TwoWire I2C2 = TwoWire(1); //I2C2 bus might not need this?

TaskHandle_t Core0TaskHnd;

SamplePlayer* player; // sample player
MidiNoteHandler* midi_handler; // midi note dispatch

static float fl_sample[SAMPLE_BUFFER_SIZE]; // raw (mixed) sound data to be sent out the DAC
static float fr_sample[SAMPLE_BUFFER_SIZE]; // these should probably go somewhere else tbh

// other core stuff
inline void Core0TaskLoop()
{
  // TODO: handle inputs etc
  delay(1);
}

inline void Core0TaskSetup()
{
  // TODO: idk maybe this will be useful later
}

inline void audio_task()
{
  memset(fl_sample, 0, sizeof(fl_sample));
  memset(fr_sample, 0, sizeof(fr_sample));

  // load latest buffer from mixer
  player->process(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE);

  // Send to DAC
  // function blocks and returns when sample is put into buffer
  if (i2s_write_stereo_samples_buff(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE))
  {
    ;
  }
}

void CoreTask0(void *parameter)
{
  Core0TaskSetup();

  while (true)
  {
    Core0TaskLoop();
    /* this seems necessary to trigger the watchdog */
    delay(1);
    yield();
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("init...");

  // I2C 
  Wire.begin(I2C1_SDA, I2C1_SCL); // Adjusted for common Arduino I2C setup

  // sdcard
  pinMode(SD_CS, OUTPUT);
   digitalWrite(SD_CS, HIGH);

  // I2S
  setup_i2s();

  // Initialize player
  player = new SamplePlayer();

   // MIDI
  midi_handler = new MidiNoteHandler(player);
  midi_handler->begin(MIDI_CHANNEL_OMNI);

  // Programmatically load NUM_PLAYERS samples
  char samplePath[120];
  for (int i = 0; i < NUM_PLAYERS; i++) {
    snprintf(samplePath, sizeof(samplePath), "/samples/%d.wav", i); // We're gonna want to be able to list and select files but lets just load 1-8 for now
    player->loadWav(i, samplePath);

    // Associate each sample with a MIDI note
    midi_handler->setNoteToListen(NOTE_BEGIN + i, i);
  }

  xTaskCreatePinnedToCore(CoreTask0, "CoreTask0", 8192, NULL, 999, &Core0TaskHnd, 0);
}

void loop()
{
  audio_task();
}

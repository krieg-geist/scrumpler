#ifndef SAMPLE_PLAYER_HPP
#define SAMPLE_PLAYER_HPP

#include <Arduino.h>
#include "config.hpp"

#define SAMPLE_FOLDER "samples"

// Precomputed panning values, L, R
const float pan_lut[2][19] = {
    {1.000, 0.996, 0.985, 0.966, 0.940, 0.906, 0.866, 0.819, 0.766, 0.707, 0.643, 0.574, 0.500, 0.423, 0.342, 0.259, 0.174, 0.087, 0.000},
    {0.000, 0.087, 0.174, 0.259, 0.342, 0.423, 0.500, 0.574, 0.643, 0.707, 0.766, 0.819, 0.866, 0.906, 0.940, 0.966, 0.985, 0.996, 1.000}
};

class SamplePlayer {
public:
    SamplePlayer(); // Constructor
    ~SamplePlayer(); // Destructor
    bool loadWav(uint8_t sampleNum, char* filename);
    bool setPan(uint8_t sampleNum, uint8_t pan);
    bool setVol(uint8_t sampleNum, uint8_t vol);
    bool sampleOn(uint8_t sampleNum, uint8_t velocity);
    void process(float *signal_l, float *signal_r, const int buffLen);
    static void init();

private:
    static uint32_t totalSampleStorageLen;
    static uint8_t sampleCount;
    struct Player {
        bool enabled;
        int32_t pos;
        float decay;
        float decay_sample;
        bool playing;
        float velocity; // 0.0 -> 1.0
        uint8_t pan;    // 0, 9, 18 (L, LR, R)
        char* filename[64];
        uint32_t numSamples;
        int16_t *sampleStorage;
        bool stereo;
    };

    static Player samplePlayers[NUM_PLAYERS];
};

#endif // SAMPLE_PLAYER_H
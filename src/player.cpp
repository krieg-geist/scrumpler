#include "player.hpp"
#include "patch_manager.hpp"

SamplePlayer::Player SamplePlayer::samplePlayers[NUM_PLAYERS];
uint32_t SamplePlayer::totalSampleStorageLen = 0;
uint8_t SamplePlayer::sampleCount = 0;

SamplePlayer::SamplePlayer() {
    psramInit();
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
    
    totalSampleStorageLen = ESP.getFreePsram() / sizeof(int16_t);
    PatchManager_SetDestination(1, 1);
}

SamplePlayer::~SamplePlayer() {
    // Destructor code here, such as freeing dynamic memory if used
}

bool SamplePlayer::loadWav(uint8_t sampleNum, char* filename) {
    if (! PatchManager_PrepareSdCard())
    {
        return false;
    }
    // Ensure the sample number is within bounds
    if (sampleNum >= NUM_PLAYERS) {
        Serial.println("Sample number out of bounds.");
        return false;
    }

    Player* newPatch = &samplePlayers[sampleNum];
    // Ideally reset the newPatch fields to defaults here, omitted for brevity

    auto dataSize = PatchManager_WaveSize(SD_MMC, filename);
    if (dataSize == 0)
    {
        return false;
    }
    Serial.print("Datasize: ");
    Serial.println(dataSize);

    auto freePSRAM = ESP.getFreePsram();
    if (dataSize > freePSRAM) {
        Serial.println("Not enough PSRAM memory for sample storage!");
        return false;
    }

    newPatch->numSamples = dataSize / 2;
    newPatch->sampleStorage = (int16_t*)ps_malloc(dataSize);
    if (newPatch->sampleStorage == NULL) {
        Serial.println("Could not allocate PSRAM!");
        return false;
    }

    auto readWavSamples = PatchManager_LoadWavefile(SD_MMC, filename, newPatch->sampleStorage, dataSize, newPatch->stereo);
    SD_MMC.end();

    if (readWavSamples <= 0) {
        Serial.println("Error reading WAV file.");
        // Free allocated memory if WAV file read fails
        free(newPatch->sampleStorage);
        newPatch->sampleStorage = NULL;
        return false;
    }

    Serial.printf("Read %d samples from %s on SD_MMC\n", readWavSamples, filename);

    // Setup the newPatch properties after successful loading
    newPatch->velocity = 1.0f;
    newPatch->pan = 9; // Assuming mid-pan as default
    // Assuming 'filename' fits within the char* array, consider strncpy for safety
    strncpy((char*)newPatch->filename, filename, sizeof(newPatch->filename) - 1);
    newPatch->enabled = true;
    newPatch->playing = false;
    newPatch->pos = 0;
    newPatch->decay_sample = 0.0f;
    newPatch->decay = 0.0f;

    Serial.println("Successfully initialized sample.");

    return true;
}

bool SamplePlayer::setPan(uint8_t sampleNum, uint8_t pan) {
    if (pan >= 19)
    {
        Serial.println("Invalid pan value");
        return false;
    }
    samplePlayers[sampleNum].pan = pan;
    return true;
}

bool SamplePlayer::setVol(uint8_t sampleNum, uint8_t vol) {
    if (vol >= 16)
    {
        Serial.println("Invalid vol value");
        return false;
    }
    samplePlayers[sampleNum].velocity = (float)vol / 16;
    return true;
}

bool SamplePlayer::sampleOn(uint8_t sampleNum, uint8_t velocity) {
    auto *player = &samplePlayers[sampleNum];
    player->velocity = (float)velocity / 127;
    if(player->enabled)
    {
        if(player->playing)
        {
            player->decay_sample = ((float)player->sampleStorage[player->pos]) / ((float)0x8000) * player->velocity;
            player->pos = 0;
            return true;
        }
        player->playing = true;
        return true;
    }
    return false;
}

void SamplePlayer::process(float *signal_l, float *signal_r, const int buffLen) {
    for (int i = 0; i < sampleCount; i++)
    {
        auto *player = &samplePlayers[i];

        for (int n = 0; n < buffLen; n++)
        {
            float sample_f_l = 0; // Left channel sample
            float sample_f_r = 0; // Right channel sample
            
            if (player->decay_sample != 0.0)
            {
                if (player->decay_sample > AUDIBLE_LIMIT)
                {
                    player->decay_sample *= 0.99;
                    sample_f_l += player->decay_sample;
                    sample_f_r += player->decay_sample; // Apply decay equally to both channels for simplicity
                }
                else
                {
                    player->decay_sample = 0;
                }
            }
            
            if (player->playing)
            {
                if (player->stereo)
                {
                    // For stereo samples, read two consecutive samples for L and R channels
                    sample_f_l = ((float)player->sampleStorage[player->pos]) / ((float)0x8000);
                    sample_f_r = ((float)player->sampleStorage[player->pos + 1]) / ((float)0x8000);
                    player->pos += 2; // Move to the next pair of samples
                }
                else
                {
                    // For mono samples, the same sample is applied to both L and R channels
                    sample_f_l = sample_f_r = ((float)player->sampleStorage[player->pos]) / ((float)0x8000);
                    player->pos += 1; // Move to the next sample
                }

                sample_f_l *= player->velocity;
                sample_f_r *= player->velocity;
                
                if (player->pos >= player->numSamples * (player->stereo ? 2 : 1))
                {
                    player->playing = false;
                    player->decay_sample = (sample_f_l + sample_f_r) / 2; // Average decay for simplicity
                    player->pos = 0;
                }
                
                // Apply panning only for mono samples or handle stereo panning differently

                signal_l[n] += sample_f_l * pan_lut[0][player->pan];
                signal_r[n] += sample_f_r * pan_lut[1][player->pan];
            }
        }
    }
}

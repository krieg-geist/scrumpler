#ifndef MidiNoteHandler_hpp
#define MidiNoteHandler_hpp

#include <MIDI.h>
#include "player.hpp"

class MidiNoteHandler {
public:
    MidiNoteHandler(SamplePlayer* player);
    void begin(int midiChannel = 1);
    void update();
    void setNoteToListen(uint8_t note, uint8_t channel);
    void removeNoteToListen(uint8_t note);

private:
    struct NoteCallback {
        uint8_t note;
        bool listen;
    } noteCallbacks[NUM_PLAYERS];

    SamplePlayer* player; // Pointer to a SamplePlayer instance

    static void handleNoteOnStatic(byte channel, byte note, byte velocity);
    static void handleNoteOffStatic(byte channel, byte note, byte velocity);

    void handleNoteOn(byte channel, byte note, byte velocity);
    void handleNoteOff(byte channel, byte note, byte velocity);
    static MidiNoteHandler* instance;
};

#endif /* MidiNoteHandler_hpp */
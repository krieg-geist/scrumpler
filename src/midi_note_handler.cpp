#include <Arduino.h>

#include "midi_note_handler.hpp"
#include "config.hpp"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, serialMidi);
MidiNoteHandler* MidiNoteHandler::instance = nullptr;

MidiNoteHandler::MidiNoteHandler(SamplePlayer* player) : player(player) {
    instance = this;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        noteCallbacks[i].note = 0;
        noteCallbacks[i].listen = false;
    }

    serialMidi.setHandleNoteOn(MidiNoteHandler::handleNoteOnStatic);
    serialMidi.setHandleNoteOff(MidiNoteHandler::handleNoteOffStatic);
}

void MidiNoteHandler::begin(int midiChannel) {
    // Midi stuff
    Serial1.begin(31250, SERIAL_8N1, MIDI_RX_PIN);
    serialMidi.begin(midiChannel);
}

void MidiNoteHandler::setNoteToListen(uint8_t note, uint8_t channel) {
    // Ensure the channel is within the valid range
    if (channel >= NUM_PLAYERS) {
        Serial.print("Warning: Channel ");
        Serial.print(channel);
        Serial.print(" is invalid. Select a channel between 0 and ");
        Serial.println(NUM_PLAYERS - 1);
        return;
    }

    // Ensure the note is within the valid MIDI range
    if (note > 127) {
        Serial.print("Warning: Note ");
        Serial.print(note);
        Serial.println(" is invalid. Select a note between 0-127.");
        return;
    }

    if (noteCallbacks[channel].listen && noteCallbacks[channel].note != note) {
        Serial.print("Warning: Attempting to overwrite an existing listening note on channel ");
        Serial.print(channel);
        Serial.println(". Operation aborted.");
        return; // Exit to avoid unintentional overwrites
    }

    // Set the note and mark it for listening
    noteCallbacks[channel].note = note;
    noteCallbacks[channel].listen = true;
    Serial.print("Listening set for note ");
    Serial.print(note);
    Serial.print(" on channel ");
    Serial.println(channel); // Confirm operation success
}

void MidiNoteHandler::removeNoteToListen(uint8_t note) {
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (noteCallbacks[i].note == note) {
            noteCallbacks[i].listen = false;
            break;
        }
    }
}

void MidiNoteHandler::handleNoteOnStatic(byte channel, byte note, byte velocity) {
    if (instance) {
        instance->handleNoteOn(channel, note, velocity);
    }
}

void MidiNoteHandler::handleNoteOffStatic(byte channel, byte note, byte velocity) {
    if (instance) {
        instance->handleNoteOff(channel, note, velocity);
    }
}

void MidiNoteHandler::handleNoteOff(byte channel, byte note, byte velocity) {
    // TODO
}

void MidiNoteHandler::handleNoteOn(byte channel, byte note, byte velocity) {
    char buffer[150];
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (noteCallbacks[i].note == note && noteCallbacks[i].listen) {
            // Attempt to play the sample, check if it fails
            if (!player->sampleOn(i)) {
                // If sampleOn returns false, print a message to the console
                Serial.print("Note ");
                Serial.print(note);
                Serial.println("couldn't be played.");
            }
            break;
        }
    }
}

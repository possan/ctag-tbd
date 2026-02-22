#include "MacroTranslator.hpp"
#include "esp_log.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "SynthDefinition.hpp"
#include "TrackDefinition.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


MacroTranslator::MacroTranslator() {
    synthDefinitionModel = nullptr;
    macroSoundDefinitionModel = nullptr;
    macroDeviceDefinitionModel = nullptr;

    for (int i = 0; i < 16; i++) {
        trackToMidiChannel[i] = -1;
        midiChannelToTrack[i] = -1;
        trackBaseCC[i] = 0;
        trackMachineId[i] = "";
        definition[i] = nullptr;
        trackDirty[i] = 0;

        for (int j = 0; j < 16; j++) {
            trackParameterValues[i][j] = 0;
            trackOutputMappingCC[i][j] = -1;
        }
    }
};

MacroTranslator::~MacroTranslator() {
};

void MacroTranslator::SetTrackMachine(const int trackIndex, const std::string &synthID) {
    if (synthID == trackMachineId[trackIndex]) {
        return;
    }

    ESP_LOGI("MacroTranslator", "Track %d machine set to %s",
        trackIndex, synthID.c_str());
    trackMachineId[trackIndex] = synthID;

    SynthDefinition *synthDef =
        synthDefinitionModel->GetSynthDefinition(synthID);

    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Synth definition not found for id %s",
            synthID.c_str());
        return;
    }

    int idx = 0;


    TrackDefinition *trackDef = synthDefinitionModel->GetTrackDefinition(trackIndex);
    if (trackDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Track definition not found for track index %d",
            trackIndex);
        return;
    }

    midiChannelToTrack[trackDef->midiChannel] = trackIndex;
    trackBaseCC[trackIndex] = trackDef->baseCC;

    // for(int k=0; k<16; k++){
    //     synthParameterIdToIndex[trackIndex][k] = -1;
    // }


    idx = 0;
    for(auto par : synthDef->parameters) {
        ESP_LOGI("MacroTranslator", "Processing parameter %s, type %d, cc %d",
            par->id.c_str(), par->type, par->cc);

        if (par->type == SynthParameterType_CC) {
            soundProcessor->handleMidiControlChange(
                trackToMidiChannel[trackIndex],
                trackBaseCC[trackIndex] + par->cc,
                par->defaultValue
            );
            // synthParameterIdToIndex[trackIndex][par->id] = idx;
        }

        idx ++;
    }

    trackDirty[trackIndex] = 1;
}

void MacroTranslator::SetTrackMacroDefinition(const int trackIndex, MacroDeviceDefinition *def) {
    ESP_LOGI("MacroTranslator", "Setting track %d macro definition 0x%08X",
        trackIndex, (uintptr_t)def);
    if (def != nullptr) {
        ESP_LOGI("MacroTranslator", "Macro def: \"%s\" \"%s\" \"%s\"",
            def->id.c_str(), def->name.c_str(), def->synthId.c_str());
    }

    if (definition[trackIndex] != nullptr) {
        delete definition[trackIndex];
        definition[trackIndex] = nullptr;
    }

    if (def == nullptr) {
        SetTrackMachine(trackIndex, "");
        return;
    }

    SynthDefinition *synthDef =
        synthDefinitionModel->GetSynthDefinition(def->synthId);

    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Invalid synth referenceSynth definition not found for id %s",
            def->synthId.c_str());
    }

    TrackDefinition *trackDef = synthDefinitionModel->GetTrackDefinition(trackIndex);
    if (trackDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Track definition not found for track index %d",
            trackIndex);
        return;
    }

    if (synthDef != nullptr) {
        int idx = 0;
        for(auto om : def->outputMappings) {
            int mappedcc = -1;
            for(auto par : synthDef->parameters) {
                if (par->id == om->synthParameterId) {
                    mappedcc = par->cc + trackDef->baseCC;
                    break;
                }
            }

            ESP_LOGI("MacroTranslator", "Macro definition output for synth parameter %s, mapped to cc %d",
                om->synthParameterId.c_str(), mappedcc);

            trackOutputMappingCC[trackIndex][idx] = mappedcc;

            idx ++;
        }
    }

    Document d1;
    d1.SetObject();
    if (def->SerializeJSONInto(d1)) {
        // StringBuffer buffer;
        // Writer<StringBuffer> writer(buffer);
        // d1.Accept(writer);
        MacroDeviceDefinition *defcopy = new MacroDeviceDefinition();
        if (defcopy->DeserializeJSON(d1)) {
            definition[trackIndex] = defcopy;
        } else {
            ESP_LOGE("MacroTranslator", "Failed to deserialize macro definition into JSON");
            delete defcopy;
        }
    } else {
        ESP_LOGE("MacroTranslator", "Failed to serialize macro definition into JSON");
    }

    SetTrackMachine(trackIndex, def->synthId);
}

void MacroTranslator::SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value) {
    if (trackIndex < 0 || trackIndex >= 16) {
        // ESP_LOGE("MacroTranslator", "Track index out of range: %d", trackIndex);
        return;
    }

    if (parameterIndex < 0 || parameterIndex >= 16) {
        // ESP_LOGE("MacroTranslator", "Parameter index out of range: %d", parameterIndex);
        return;
    }

    ESP_LOGI("MacroTranslator", "Track %d, Parameter %d = %d",
        trackIndex, parameterIndex, value);
    trackParameterValues[trackIndex][parameterIndex] = value;
    trackDirty[trackIndex] = 1;
}


void MacroTranslator::SetTrackParametersFromJSON(const int trackIndex, const std::string &parametersJSON) {
    Document d;

    d.Parse(parametersJSON.c_str());
    if (d.HasParseError()) {
        ESP_LOGE("MacroTranslator", "Failed to parse parameters JSON: %s", parametersJSON.c_str());
        return;
    }

    if (!d.IsObject()) {
        ESP_LOGE("MacroTranslator", "Parameters JSON is not an object: %s", parametersJSON.c_str());
        return;
    }

    if (d.HasMember("macro")) {
        std::string macro = d["macro"].GetString(); 
        MacroDeviceDefinition *def =
            macroDeviceDefinitionModel->GetMacroDeviceDefinition(macro);
        ESP_LOGI("MacroTranslator", "Setting track %d macro definition to %s => 0x%08X", trackIndex, macro.c_str(), (uintptr_t)def);
        this->SetTrackMacroDefinition(trackIndex, def);
    }

    if (d.HasMember("machine")) {
        std::string machine = d["machine"].GetString();
        ESP_LOGI("MacroTranslator", "Setting track %d machine to: %s", trackIndex, machine.c_str());
        this->SetTrackMachine(trackIndex, machine);
    }

    if (d.HasMember("parameters")) {
        const Value& params = d["parameters"];
        if (!params.IsArray()) {
            ESP_LOGE("MacroTranslator", "'parameters' member is not an array: %s", parametersJSON.c_str());
            return;
        }

        int values[16] = {};
        for(int k=0; k<16; k++) {
            values[k] = -1;
        }
        int idx = 0;
        for (auto &v : params.GetArray()) {
            if (v.IsInt()) {
                values[idx] = v.GetInt();
                trackParameterValues[trackIndex][idx] = values[idx];
                trackDirty[trackIndex] = 1;
                ESP_LOGI("MacroTranslator", "Set track %d parameter %d to %d",
                    trackIndex, idx, values[idx]);
            }

            idx ++;
        }

        // MacroDeviceDefinition *def = definition[trackIndex];
        // if (def == nullptr) {
        //     ESP_LOGE("MacroTranslator", "No macro definition for track %d", trackIndex);
        //     return;
        // }
    }
}


void MacroTranslator::_parseIncomingMidiMessages(const uint8_t *buf, const size_t len) {
    // if (len > 1) {
    //     ESP_LOGI("MacroTranslator",
    //         "parseIncomingMidiMessages: %02X %02X %02X %02X %02X %02X %02X %02X %02X (%d)",
    //             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], len);
    // }

    if (buf == nullptr || len < 1)  {
        return;
    }

    int left = len;
    int o = 0;

    while(left > 0) {
        uint8_t b0 = buf[o++];
        left --;

        if (b0 == 0) {
            // probably end of event stream
            return;
        }

        uint8_t inputchannel = (b0 & 0x0F);
        uint8_t cmd = (b0 & 0xF0);
        int track = midiChannelToTrack[inputchannel];

        switch(cmd) {
            case 0x80: // note off
            {
                if (left < 2) return; // not enough data

                uint8_t note = buf[o++];
                uint8_t velocity = buf[o++];
                left -= 2;

                soundProcessor->handleMidiNoteOff(track, note, velocity);
                // _handleMidiNoteOff(channel, b1, b2);
                break;
            }
            case 0x90: // note on
            {
                if (left < 2) return; // not enough data

                uint8_t note = buf[o++];
                uint8_t velocity = buf[o++];
                left -= 2;

                if (velocity > 0) {
                    ESP_LOGI("MacroTranslator", "Note on, track %d, note %d, velocity %d",
                        track, note, velocity);
                    soundProcessor->handleMidiNoteOn(track, note, velocity);
                    // _handleMidiNoteOn(channel, note, velocity);
                } else {
                    soundProcessor->handleMidiNoteOff(track, note, velocity);
                    // _handleMidiNoteOff(channel, note, velocity);
                }
                break;
            }
            case 0xA0: // aftertouch
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                // _handleMidiAftertouch(channel, b1, b2);
                break;
            }
            case 0xB0: // control change
            {
                if (left < 2) return; // not enough data

                uint8_t control = buf[o++];
                uint8_t value = buf[o++];
                left -= 2;

                int macrocc = (int)control - trackBaseCC[track];
                macrocc -= 8; // input parameter CC's start at 8 too and...

                // ESP_LOGI("MacroTranslator", "CC, track %d, control %d, value %d",
                //     track, control, value);
                this->SetTrackParameter(track, macrocc, value);

                // }
                // _handleMidiControlChange(channel, cnotrol, b2);
                break;
            }
            case 0xC0: // program change
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++]; // not used?
                left -= 2;
                // _handleMidiPatchChange(channel, b1);
                break;
            }
            case 0xE0: // pitch bend
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                // _handleMidiPitchBend(channel, b2 * 128 + b1);
                break;
            }
            case 0xF0: // system common / real time
                // TODO handle sysex, clock, start, stop, continue, ...
                switch(b0) {
                    case 0xF0: // sysex start
                        return; // just ignore and bail out for now

                    case 0xF8: // timing clock
                    case 0xFA: // start
                    case 0xFB: // continue
                    case 0xFC: // stop
                    case 0xFE: // active sensing
                    case 0xFF: // system reset
                        // ignore for now
                        break;
                }
                break; // fail for now
            default:
                // for anything else, just stop parsing.
                return;
        }
    }
}


void MacroTranslator::TranslateInput(CTAG::SP::ProcessData *pd) {
    if (pd == nullptr)
        return;

    for(int t=0; t<16; t++) {
        if (trackDirty[t]) {
            // First change machines if needed.
            soundProcessor->setTrackMachine(t, trackMachineId[t]);
        }
    }

    _parseIncomingMidiMessages(pd->midi_bytes, pd->midi_bytes_length);

    for(int t=0; t<16; t++) {
        if (trackDirty[t]) {
            trackDirty[t] = 0;

            MacroDeviceDefinition *def = definition[t];

            ESP_LOGI("MacroTranslator", "Track %d is dirty, def=0x%08X", t, (uintptr_t)def);

            if (def != nullptr) {
                ESP_LOGI("MacroTranslator", "Using definition: %s, %s",
                    def->id.c_str(), def->name.c_str());

                int idx = 0;
                for(auto om : def->outputMappings) {
                    int32_t finalvalue = om->startValue;

                    int cc = trackOutputMappingCC[t][idx];
                    for(auto src : om->sources) {
                        int val = trackParameterValues[t][src->parameterIndex];
                        finalvalue += val * src->amount;
                    }

                    if (finalvalue < 0) finalvalue = 0;
                    if (finalvalue > 127) finalvalue = 127;

                    ESP_LOGI("MacroTranslator", "Track %d: setting synth parameter %s to %d (CC %d)",
                        t, om->synthParameterId.c_str(), finalvalue, cc);
                    if (cc != -1) {
                        soundProcessor->handleMidiControlChange(t, cc, finalvalue);
                    }
                    idx ++;
                }
            }
        }
    }
}
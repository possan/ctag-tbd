/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#include "ctagSoundProcessorSineSrc.hpp"
#include <iostream>
#include <cmath>
#include "helpers/ctagFastMath.hpp"


using namespace CTAG::SP;

void ctagSoundProcessorSineSrc::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // init params
    sineSource.SetSampleRate(44100.f);
    sineSource.SetFrequency(1000.f);
    // ad envs
    adEnv.SetSampleRate(44100.f);
    adEnv.SetModeExp();
    pitchEnv.SetSampleRate(44100.f);
    pitchEnv.SetModeExp();
}

void ctagSoundProcessorSineSrc::Process(const ProcessData &data) {
    float deltaCVLoud = (data.cv[cv_loudness] - preCVLoudness) / (float) bufSz; // for linear CV interpolation
    freq = (float) frequency; // frequency
    // cv frequency
    if (cv_frequency != -1) {
        preCVFrequency = 0.3f * data.cv[cv_frequency] + 0.7f * preCVFrequency; // smooth CV
        float fMod = preCVFrequency * 5.f;
        fMod = CTAG::SP::HELPERS::fastpow2(fMod);
        freq *= fMod;
    }
    // eg pitch
    if (enableEG_p == 1 && trig_enableEG_p != -1) {
        if (data.trig[trig_enableEG_p] != prevTrigState_p) {
            prevTrigState_p = data.trig[trig_enableEG_p];
            if (prevTrigState_p == 0) pitchEnv.Trigger();
        }
        pitchEnv.SetLoop(loopEG_p);
        attackVal_p = (float) attack_p / 4095.f * 5.f;
        if (cv_attack_p != -1) {
            attackVal_p = data.cv[cv_attack_p] * data.cv[cv_attack_p];
        }
        decayVal_p = (float) decay_p / 4095.f * 5.f;
        if (cv_decay_p != -1) {
            decayVal_p = data.cv[cv_decay_p] * data.cv[cv_decay_p];
        }
        pitchEnv.SetAttack(attackVal_p);
        pitchEnv.SetDecay(decayVal_p);
    }
    // parameter control
    loud = (float) loudness / 4095.f;
    //  eg loud
    if (enableEG == 1 && trig_enableEG != -1) {
        if (data.trig[trig_enableEG] != prevTrigState) {
            prevTrigState = data.trig[trig_enableEG];
            if (prevTrigState == 0) adEnv.Trigger();
        }
        adEnv.SetLoop(loopEG);
        attackVal = (float) attack / 4095.f * 5.f;
        if (cv_attack != -1) {
            attackVal = data.cv[cv_attack] * data.cv[cv_attack] * 2.f;
        }
        decayVal = (float) decay / 4095.f * 10.f;
        if (cv_decay != -1) {
            decayVal = data.cv[cv_decay] * data.cv[cv_decay] * 4.f;
        }
        adEnv.SetAttack(attackVal);
        adEnv.SetDecay(decayVal);
    }
    // here is the oscillator
    float freqb;
    for (int i = 0; i < this->bufSz; i++) { // iterate all channel samples
        freqb = freq;
        // pitch fm
        if (enableEG_p == 1 && trig_enableEG_p != -1) {
            float val;
            if (cv_amount_p == -1)
                val = CTAG::SP::HELPERS::fasterpow2(pitchEnv.Process() * (float) amount_p / 4095.f * 8.f);
            else
                val = CTAG::SP::HELPERS::fasterpow2(pitchEnv.Process() * data.cv[cv_amount_p] * 8.f);
            freqb *= val;
            if (freqb > 10000.f) freqb = 1000.f;
            if (freqb < 15.f) freqb = 15.f;
        }
        // freq
        sineSource.SetFrequency(freqb);
        // get samples
        data.buf[i * 2 + this->processCh] = sineSource.Process() * loud;
        // apply loud EG
        if (enableEG == 1) {
            data.buf[i * 2 + this->processCh] *= adEnv.Process();
        }
        // apply CV loud control
        if (cv_loudness != -1) {
            data.buf[i * 2 + this->processCh] *=
                    preCVLoudness * preCVLoudness; // linearly interpolate CV data, square for loudness perception
            preCVLoudness += deltaCVLoud;
        }
    }
}

void ctagSoundProcessorSineSrc::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("frequency", [&](const int val) { frequency = val; });
    pMapCv.emplace("frequency", [&](const int val) { cv_frequency = val; });
    pMapPar.emplace("loudness", [&](const int val) { loudness = val; });
    pMapCv.emplace("loudness", [&](const int val) { cv_loudness = val; });
    pMapPar.emplace("enableEG", [&](const int val) { enableEG = val; });
    pMapTrig.emplace("enableEG", [&](const int val) { trig_enableEG = val; });
    pMapPar.emplace("loopEG", [&](const int val) { loopEG = val; });
    pMapTrig.emplace("loopEG", [&](const int val) { trig_loopEG = val; });
    pMapPar.emplace("attack", [&](const int val) { attack = val; });
    pMapCv.emplace("attack", [&](const int val) { cv_attack = val; });
    pMapPar.emplace("decay", [&](const int val) { decay = val; });
    pMapCv.emplace("decay", [&](const int val) { cv_decay = val; });
    pMapPar.emplace("enableEG_p", [&](const int val) { enableEG_p = val; });
    pMapTrig.emplace("enableEG_p", [&](const int val) { trig_enableEG_p = val; });
    pMapPar.emplace("loopEG_p", [&](const int val) { loopEG_p = val; });
    pMapTrig.emplace("loopEG_p", [&](const int val) { trig_loopEG_p = val; });
    pMapPar.emplace("amount_p", [&](const int val) { amount_p = val; });
    pMapCv.emplace("amount_p", [&](const int val) { cv_amount_p = val; });
    pMapPar.emplace("attack_p", [&](const int val) { attack_p = val; });
    pMapCv.emplace("attack_p", [&](const int val) { cv_attack_p = val; });
    pMapPar.emplace("decay_p", [&](const int val) { decay_p = val; });
    pMapCv.emplace("decay_p", [&](const int val) { cv_decay_p = val; });
    isStereo = false;
    id = "SineSrc";
    // sectionCpp0
}

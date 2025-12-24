#pragma once

#include "RackSynth.hpp"
#include "plaits/dsp/drums/analog_bass_drum.h"
#include "plaits/dsp/drums/analog_snare_drum.h"
#include "plaits/dsp/drums/synthetic_bass_drum.h"
#include "plaits/dsp/drums/synthetic_snare_drum.h"
#include "plaits/dsp/drums/hi_hat.h"
#include "braids/analog_oscillator.h"
#include "braids/signature_waveshaper.h"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "filters/ctagDiodeLadderFilter.hpp"
#include "filters/ctagDiodeLadderFilter2.hpp"
#include "filters/ctagDiodeLadderFilter3.hpp"
#include "filters/ctagDiodeLadderFilter4.hpp"
#include "filters/ctagDiodeLadderFilter5.hpp"
#include "filters/ctagFilterBase.hpp"
#include "synthesis/RomplerVoiceMinimal.hpp"
#include "synthesis/Clap.hpp"
#include "synthesis/Rimshot.hpp"
#include "synthesis/FmKick.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"
#include "SimpleComp/SimpleComp.h"
#include "mifx/reverb.h"
#include "polypad/ChordSynth.hpp"

using namespace CTAG::SP;

class RackTBD03 {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float td3_out[BUF_SZ];

	void handleMidiNoteOn(uint8_t note, uint8_t vel);
	void handleMidiNoteOff(uint8_t note, uint8_t vel);
	// void handleMidiCC(uint8_t control, uint8_t value);

private:
    ctagDiodeLadderFilter5 td3_pirkle_zdf_boost; // Pirkle ZDF with boost
    ctagDiodeLadderFilter3 td3_karlson; // Karlson
    ctagDiodeLadderFilter4 td3_blaukraut; // Blaukraut
    ctagDiodeLadderFilter td3_pirkle_zdf; // Pirkle ZDF
    ctagDiodeLadderFilter2 td3_zavalishin; // Zavalishin ZDF
    ctagADEnv td3_adVCA, td3_adVCF;
    braids::MacroOscillator td3_osc;
    braids::SignatureWaveshaper td3_ws;
    uint8_t td3_sync[BUF_SZ] = {0};
    bool td3_pre_trig = false;
    bool td3_isAccent = false;
    float td3_pre_eg_val = 0.f;
    float td3_pre_pitch_val = 0.f;
	int midi_note {0};
	float midi_freq {0.0f};
	bool midi_trig {false};

	atomic<int16_t> td3_sync_trig; //, trig_td3_sync_trig;
	atomic<int16_t> td3_shape; //, cv_td3_shape, tc3_shape;
	atomic<int16_t> td3_param_0; //, cv_td3_param_0, cc_td3_param_0;
	atomic<int16_t> td3_param_1; //, cv_td3_param_1, cc_td3_param_1;
	atomic<int16_t> td3_filter_type; //, cv_td3_filter_type, cc_td3_filter_type;
	atomic<int16_t> td3_cutoff; //, cv_td3_cutoff, cc_td3_cutoff;
	atomic<int16_t> td3_resonance; //, cv_td3_resonance, cc_td3_resonance;
	atomic<int16_t> td3_envelope; //, cv_td3_envelope, cc_td3_envelope;
	atomic<int16_t> td3_saturation; //, cv_td3_saturation, cc_td3_saturation;
	atomic<int16_t> td3_drive; //, cv_td3_drive, cc_td3_drive;
	atomic<int16_t> td3_accent; // trig_td3_accent;
	atomic<int16_t> td3_accent_level; //, cv_td3_accent_level, cc_td3_accent_level;
	atomic<int16_t> td3_slide; // trig_td3_slide;
	atomic<int16_t> td3_slide_level; //, cv_td3_slide_level, cc_td3_slide_level;
	atomic<int16_t> td3_decay_vca; //, cv_td3_decay_vca, cc_td3_decay_vca;
	atomic<int16_t> td3_decay_vcf; //, cv_td3_decay_vcf, cc_td3_decay_vcf;
	atomic<int16_t> td3_p0_amt; //, cv_td3_p0_amt, cc_td3_p0_amt;
	atomic<int16_t> td3_p1_amt; //, cv_td3_p1_amt, cc_td3_p1_amt;
};

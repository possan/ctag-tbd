#pragma once

#include "DrumRackSynth.hpp"
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

class DrumRackTD3 : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
    ctagDiodeLadderFilter5 td3_pirkle_zdf_boost; // Pirkle ZDF with boost
    ctagDiodeLadderFilter3 td3_karlson; // Karlson
    ctagDiodeLadderFilter4 td3_blaukraut; // Blaukraut
    ctagDiodeLadderFilter td3_pirkle_zdf; // Pirkle ZDF
    ctagDiodeLadderFilter2 td3_zavalishin; // Zavalishin ZDF
    ctagADEnv td3_adVCA, td3_adVCF;
    braids::MacroOscillator td3_osc;
    braids::SignatureWaveshaper td3_ws;
    uint8_t td3_sync[32] = {0};
    bool td3_pre_trig = false;
    bool td3_isAccent = false;
    float td3_pre_eg_val = 0.f;
    float td3_pre_pitch_val = 0.f;

	atomic<int16_t> td3_trigger, trig_td3_trigger;
	atomic<int16_t> td3_sync_trig, trig_td3_sync_trig;
	atomic<int16_t> td3_pitch, cv_td3_pitch;
	atomic<int16_t> td3_shape, cv_td3_shape;
	atomic<int16_t> td3_param_0, cv_td3_param_0;
	atomic<int16_t> td3_param_1, cv_td3_param_1;
	atomic<int16_t> td3_filter_type, cv_td3_filter_type;
	atomic<int16_t> td3_cutoff, cv_td3_cutoff;
	atomic<int16_t> td3_resonance, cv_td3_resonance;
	atomic<int16_t> td3_envelope, cv_td3_envelope;
	atomic<int16_t> td3_saturation, cv_td3_saturation;
	atomic<int16_t> td3_drive, cv_td3_drive;
	atomic<int16_t> td3_accent, trig_td3_accent;
	atomic<int16_t> td3_accent_level, cv_td3_accent_level;
	atomic<int16_t> td3_slide, trig_td3_slide;
	atomic<int16_t> td3_slide_level, cv_td3_slide_level;
	atomic<int16_t> td3_decay_vca, cv_td3_decay_vca;
	atomic<int16_t> td3_decay_vcf, cv_td3_decay_vcf;
	atomic<int16_t> td3_p0_amt, cv_td3_p0_amt;
	atomic<int16_t> td3_p1_amt, cv_td3_p1_amt;
	atomic<int16_t> td3_lev, cv_td3_lev;
	atomic<int16_t> td3_pan, cv_td3_pan;
	atomic<int16_t> td3_fx1, cv_td3_fx1;
	atomic<int16_t> td3_fx2, cv_td3_fx2;
};

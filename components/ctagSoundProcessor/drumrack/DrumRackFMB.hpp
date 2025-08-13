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

class DrumRackFMB : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
	CTAG::SYNTHESIS::FmKick fmb;
	float fmb_out[32];
	bool fmb_trig_prev {false};
	atomic<int32_t> fmb_trigger, trig_fmb_trigger;
	atomic<int32_t> fmb_mute, trig_fmb_mute;
	atomic<int32_t> fmb_lev, cv_fmb_lev;
	atomic<int32_t> fmb_pan, cv_fmb_pan;
	atomic<int32_t> fmb_fx1, cv_fmb_fx1;
	atomic<int32_t> fmb_fx2, cv_fmb_fx2;
	atomic<int32_t> fmb_use_ratio_mode, trig_fmb_use_ratio_mode;
	atomic<int32_t> fmb_mod_env_sync, trig_fmb_mod_env_sync;
	atomic<int32_t> fmb_f_b, cv_fmb_f_b;
	atomic<int32_t> fmb_d_b, cv_fmb_d_b;
	atomic<int32_t> fmb_f_m, cv_fmb_f_m;
	atomic<int32_t> fmb_I, cv_fmb_I;
	atomic<int32_t> fmb_d_m, cv_fmb_d_m;
	atomic<int32_t> fmb_b_m, cv_fmb_b_m;
	atomic<int32_t> fmb_A_f, cv_fmb_A_f;
	atomic<int32_t> fmb_d_f, cv_fmb_d_f;
};

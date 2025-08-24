#include "DrumRackSynth.hpp"
#include "DrumRackTBD03.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackTBD03::Init(const DrumRackInitData *initdata) {
    // uint8_t *privatedata = initdata->allocator(1000);

    td3_pirkle_zdf_boost.Init();
    td3_karlson.Init();
    td3_blaukraut.Init();
    td3_pirkle_zdf.Init();
    td3_zavalishin.Init();
    td3_osc.Init();
    td3_osc.set_pitch(100);
    td3_osc.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
    td3_adVCA.SetSampleRate(44100.f / 32);
    td3_adVCA.SetModeExp();
    td3_adVCA.SetAttack(0.f);
    td3_adVCA.SetDecay(0.5f);
    td3_adVCF.SetSampleRate(44100.f / 32);
    td3_adVCF.SetModeExp();
    td3_adVCF.SetAttack(0.f);
    td3_adVCF.SetDecay(0.5f);
    td3_ws.Init(0xcafe);

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ td3_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_td3_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "sync_trig", [&](const int val){ td3_sync_trig = val;});
	initdata->rack->registerTrig(initdata->prefix, "sync_trig", [&](const int val){ trig_td3_sync_trig = val;});
	initdata->rack->registerParam(initdata->prefix, "pitch", [&](const int val){ td3_pitch = val;});
	initdata->rack->registerCv(initdata->prefix, "pitch", [&](const int val){ cv_td3_pitch = val;});
	initdata->rack->registerParam(initdata->prefix, "shape", [&](const int val){ td3_shape = val;});
	initdata->rack->registerCv(initdata->prefix, "shape", [&](const int val){ cv_td3_shape = val;});
	initdata->rack->registerParam(initdata->prefix, "param_0", [&](const int val){ td3_param_0 = val;});
	initdata->rack->registerCv(initdata->prefix, "param_0", [&](const int val){ cv_td3_param_0 = val;});
	initdata->rack->registerParam(initdata->prefix, "param_1", [&](const int val){ td3_param_1 = val;});
	initdata->rack->registerCv(initdata->prefix, "param_1", [&](const int val){ cv_td3_param_1 = val;});
	initdata->rack->registerParam(initdata->prefix, "filter_type", [&](const int val){ td3_filter_type = val;});
	initdata->rack->registerCv(initdata->prefix, "filter_type", [&](const int val){ cv_td3_filter_type = val;});
	initdata->rack->registerParam(initdata->prefix, "cutoff", [&](const int val){ td3_cutoff = val;});
	initdata->rack->registerCv(initdata->prefix, "cutoff", [&](const int val){ cv_td3_cutoff = val;});
	initdata->rack->registerParam(initdata->prefix, "resonance", [&](const int val){ td3_resonance = val;});
	initdata->rack->registerCv(initdata->prefix, "resonance", [&](const int val){ cv_td3_resonance = val;});
	initdata->rack->registerParam(initdata->prefix, "envelope", [&](const int val){ td3_envelope = val;});
	initdata->rack->registerCv(initdata->prefix, "envelope", [&](const int val){ cv_td3_envelope = val;});
	initdata->rack->registerParam(initdata->prefix, "saturation", [&](const int val){ td3_saturation = val;});
	initdata->rack->registerCv(initdata->prefix, "saturation", [&](const int val){ cv_td3_saturation = val;});
	initdata->rack->registerParam(initdata->prefix, "drive", [&](const int val){ td3_drive = val;});
	initdata->rack->registerCv(initdata->prefix, "drive", [&](const int val){ cv_td3_drive = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ td3_accent = val;});
	initdata->rack->registerTrig(initdata->prefix, "accent", [&](const int val){ trig_td3_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "accent_level", [&](const int val){ td3_accent_level = val;});
	initdata->rack->registerCv(initdata->prefix, "accent_level", [&](const int val){ cv_td3_accent_level = val;});
	initdata->rack->registerParam(initdata->prefix, "slide", [&](const int val){ td3_slide = val;});
	initdata->rack->registerTrig(initdata->prefix, "slide", [&](const int val){ trig_td3_slide = val;});
	initdata->rack->registerParam(initdata->prefix, "slide_level", [&](const int val){ td3_slide_level = val;});
	initdata->rack->registerCv(initdata->prefix, "slide_level", [&](const int val){ cv_td3_slide_level = val;});
	initdata->rack->registerParam(initdata->prefix, "decay_vca", [&](const int val){ td3_decay_vca = val;});
	initdata->rack->registerCv(initdata->prefix, "decay_vca", [&](const int val){ cv_td3_decay_vca = val;});
	initdata->rack->registerParam(initdata->prefix, "decay_vcf", [&](const int val){ td3_decay_vcf = val;});
	initdata->rack->registerCv(initdata->prefix, "decay_vcf", [&](const int val){ cv_td3_decay_vcf = val;});
	initdata->rack->registerParam(initdata->prefix, "p0_amt", [&](const int val){ td3_p0_amt = val;});
	initdata->rack->registerCv(initdata->prefix, "p0_amt", [&](const int val){ cv_td3_p0_amt = val;});
	initdata->rack->registerParam(initdata->prefix, "p1_amt", [&](const int val){ td3_p1_amt = val;});
	initdata->rack->registerCv(initdata->prefix, "p1_amt", [&](const int val){ cv_td3_p1_amt = val;});

    this->enabled = false;
}

void DrumRackTBD03::Process(const DrumRackProcessData &data) {
    std::fill_n(td3_out, BUF_SZ, 0.f);

    float dvcf, dvca;
    bool trg;

    if (!this->enabled) {
        return;
    }

    if (trig_td3_trigger != -1) {
        trg = data.trig[trig_td3_trigger] == 1 ? 0 : 1; // negative logic
    } else {
        trg = td3_trigger;
    }

    if (trg && !td3_pre_trig) {
        printf("TBDD3\n");
        td3_isAccent = td3_accent;
        if (trig_td3_accent != -1) {
            td3_isAccent = data.trig[trig_td3_accent] == 0 ? 1 : 0;
        }
        dvcf = td3_decay_vcf / 4095.f * 5.f;
        if (cv_td3_decay_vcf != -1) {
            dvcf = fabsf(data.cv[cv_td3_decay_vcf]) * 5.f;
        }
        // if accent shorten decay of filter eg
        if (td3_isAccent) {
            dvcf = td3_kAccentDecay;
        }
        td3_adVCF.SetDecay(dvcf);
        dvca = td3_decay_vca / 4095.f * 5.f;
        if (cv_td3_decay_vca != -1) {
            dvca = fabsf(data.cv[cv_td3_decay_vca]) * 5.f;
        }
        td3_adVCA.SetDecay(dvca);
        td3_adVCF.Trigger();
        td3_adVCA.Trigger();
        // sync on trigger
        if (td3_sync_trig) td3_sync[0] = 1;
        td3_osc.Strike();
        td3_pre_trig = true;
    } else if (!trg) {
        td3_pre_trig = false;
    }

    float egvalVCA = td3_adVCA.Process();
    // if accent make slightly louder
    if (td3_isAccent) {
        egvalVCA *= td3_kAccentVCAFactor;
    }
    float egvalVCF = td3_adVCF.Process();

    // shape
    int s = td3_shape;
    if (cv_td3_shape != -1) {
        s = fabsf(data.cv[cv_td3_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    td3_osc.set_shape(ms);

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2];
    parameters[0] = td3_param_0;
    parameters[1] = td3_param_1;
    if (cv_td3_param_0 != -1) {
        parameters[0] = static_cast<int16_t>(fabsf(data.cv[cv_td3_param_0] * 32767));
    }
    if (cv_td3_param_1 != -1) {
        parameters[1] = static_cast<int16_t>(fabsf(data.cv[cv_td3_param_1] * 32767));
    }
    int32_t mod_amt[2];
    mod_amt[0] = td3_p0_amt;
    mod_amt[1] = td3_p1_amt;
    int32_t mod[2];
    if (cv_td3_p0_amt != -1) {
        mod[0] = static_cast<int32_t >(data.cv[cv_td3_p0_amt] * 65535.f);
    } else {
        mod[0] = static_cast<int32_t >(egvalVCF * 65535.f);
    }
    if (cv_td3_p1_amt != -1) {
        mod[1] = static_cast<int32_t >(data.cv[cv_td3_p1_amt] * 65535.f);
    } else {
        mod[1] = static_cast<int32_t >(egvalVCF * 65535.f);
    }
    for (int i = 0; i < 2; ++i) {
        int32_t value = parameters[i];
        value += (mod[i] * mod_amt[i]) / 8192;
        CONSTRAIN(value, 0, 32767);
        parameters[i] = value;
    }
    td3_osc.set_parameters(parameters[0], parameters[1]);

    // pitch calculation and quantization
    MK_BOOL_PAR(isSlide, td3_slide)
    MK_FLT_PAR_ABS(fSlideLevel, td3_slide_level, 4095.f, 0.099f)
    fSlideLevel += 0.9f;
    int32_t ipitch = td3_pitch;
    if (cv_td3_pitch != -1) {
        float fPitch = data.cv[cv_td3_pitch] * 12.f * 5.f; // five octaves
        if(isSlide){
            fPitch = fSlideLevel * td3_pre_pitch_val + (1.f - fSlideLevel) * fPitch;
        }
        td3_pre_pitch_val = fPitch;
        ipitch += static_cast<int32_t>(fPitch * 128.f);
    }
    CONSTRAIN(ipitch, 0, 16383);
    td3_osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[BUF_SZ];
    td3_osc.Render(td3_sync, buffer, BUF_SZ);

    // apply filter and EGs
    int ftype = td3_filter_type;
    if (cv_td3_filter_type != -1) {
        ftype = static_cast<int>(fabsf(data.cv[cv_td3_filter_type]) * 5.f);
    }
    CONSTRAIN(ftype, 0, 4)
    float c = td3_cutoff / 4095.f;
    if (cv_td3_cutoff != -1) {
        c = fabsf(data.cv[cv_td3_cutoff]);
    }
    c *= 27000.f;
    c -= 5000.f;
    float fenv = td3_envelope / 4095.f;
    if (cv_td3_envelope != -1) {
        fenv = fabsf(data.cv[cv_td3_envelope]);
    }
    c += fenv * egvalVCF * 22000.f;
    // if accent add to VCF envelope
    float facclev = td3_accent_level / 4095.f;
    if (cv_td3_accent_level != -1) {
        facclev = fabsf(data.cv[cv_td3_accent_level]);
    }
    if (td3_isAccent) {
        c += facclev * egvalVCF * 22000.f;
    }

    float r = td3_resonance / 4095.f;
    if (cv_td3_resonance != -1) {
        r = fabsf(data.cv[cv_td3_resonance]);
    }

    int32_t signature = td3_saturation;
    if (cv_td3_saturation != -1) {
        signature = static_cast<int32_t>(fabsf(data.cv[cv_td3_saturation]) * 65535.f);
    }
    CONSTRAIN(signature, 0, 65535)

    float dri = td3_drive / 4095.f * 30.f;
    if (cv_td3_drive != -1) {
        dri = fabsf(data.cv[cv_td3_drive]) * 30.f;
    }

    CONSTRAIN(c, 20.f, 22000.f)
    CONSTRAIN(r, 0.f, 1.f)
    CONSTRAIN(dri, 1.f, 30.f)
    ctagFilterBase *filter = &td3_pirkle_zdf_boost;
    switch(ftype){
        case 0:
            filter = &td3_pirkle_zdf_boost;
            break;
        case 1:
            filter = &td3_karlson;
            break;
        case 2:
            filter = &td3_blaukraut;
            break;
        case 3:
            filter = &td3_pirkle_zdf;
            break;
        case 4:
            filter = &td3_zavalishin;
            break;
    }
    filter->SetCutoff(c);
    filter->SetResonance(r);
    filter->SetGain(dri);

    for (int i = 0; i < BUF_SZ; i++) {
        float eg = td3_pre_eg_val +
                   (egvalVCA - td3_pre_eg_val) / (float) BUF_SZ * i; // linear fade from previous eg value to avoid glitches
        // apply non linearity to filter input
        int16_t warped = td3_ws.Transform(buffer[i]);
        buffer[i] = stmlib::Mix(buffer[i], warped, signature);
        // filter, EG and clip
        const float div = 3.0518509476E-5f;

        float f = stmlib::SoftClip(eg * filter->Process(buffer[i] * div));
        td3_out[i] = f;
    }
    td3_pre_eg_val = egvalVCA;
    // sync on trigger
    td3_sync[0] = 0;
}

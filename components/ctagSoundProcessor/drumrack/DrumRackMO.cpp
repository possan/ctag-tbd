#include "DrumRackSynth.hpp"
#include "DrumRackMO.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackMO::Init(const DrumRackInitData *initdata) {
    mo_osc.Init();
    mo_osc.set_pitch(100);
    mo_osc.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
    mo_ws.Init(0xcafe);
    //envelope.Init();
    mo_envelope.SetSampleRate(44100.f / 32.f);
    mo_envelope.SetModeExp();
    mo_quantizer.Init();

    initdata->rack->registerParam(initdata->prefix, "shape", [&](const int val) { mo_shape = val; });
    initdata->rack->registerCv(initdata->prefix, "shape", [&](const int val) { cv_mo_shape = val; });
    initdata->rack->registerParam(initdata->prefix, "pitch", [&](const int val) { mo_pitch = val; });
    initdata->rack->registerCv(initdata->prefix, "pitch", [&](const int val) { cv_mo_pitch = val; });
    initdata->rack->registerParam(initdata->prefix, "decimation", [&](const int val) { mo_decimation = val; });
    initdata->rack->registerCv(initdata->prefix, "decimation", [&](const int val) { cv_mo_decimation = val; });
    initdata->rack->registerParam(initdata->prefix, "bit_reduction", [&](const int val) { mo_bit_reduction = val; });
    initdata->rack->registerCv(initdata->prefix, "bit_reduction", [&](const int val) { cv_mo_bit_reduction = val; });
    initdata->rack->registerParam(initdata->prefix, "q_scale", [&](const int val) { mo_q_scale = val; });
    initdata->rack->registerCv(initdata->prefix, "q_scale", [&](const int val) { cv_mo_q_scale = val; });
    initdata->rack->registerParam(initdata->prefix, "param_0", [&](const int val) { mo_param_0 = val; });
    initdata->rack->registerCv(initdata->prefix, "param_0", [&](const int val) { cv_mo_param_0 = val; });
    initdata->rack->registerParam(initdata->prefix, "param_1", [&](const int val) { mo_param_1 = val; });
    initdata->rack->registerCv(initdata->prefix, "param_1", [&](const int val) { cv_mo_param_1 = val; });
    initdata->rack->registerParam(initdata->prefix, "waveshaping", [&](const int val) { mo_waveshaping = val; });
    initdata->rack->registerCv(initdata->prefix, "waveshaping", [&](const int val) { cv_mo_waveshaping = val; });
    initdata->rack->registerParam(initdata->prefix, "fm_amt", [&](const int val) { mo_fm_amt = val; });
    initdata->rack->registerCv(initdata->prefix, "fm_amt", [&](const int val) { cv_mo_fm_amt = val; });
    initdata->rack->registerParam(initdata->prefix, "p0_amt", [&](const int val) { mo_p0_amt = val; });
    initdata->rack->registerCv(initdata->prefix, "p0_amt", [&](const int val) { cv_mo_p0_amt = val; });
    initdata->rack->registerParam(initdata->prefix, "p1_amt", [&](const int val) { mo_p1_amt = val; });
    initdata->rack->registerCv(initdata->prefix, "p1_amt", [&](const int val) { cv_mo_p1_amt = val; });
    initdata->rack->registerParam(initdata->prefix, "enableEG", [&](const int val) { mo_enableEG = val; });
    initdata->rack->registerTrig(initdata->prefix, "enableEG", [&](const int val) { trig_mo_enableEG = val; });
    initdata->rack->registerParam(initdata->prefix, "loopEG", [&](const int val) { mo_loopEG = val; });
    initdata->rack->registerTrig(initdata->prefix, "loopEG", [&](const int val) { trig_mo_loopEG = val; });
    initdata->rack->registerParam(initdata->prefix, "attack", [&](const int val) { mo_attack = val; });
    initdata->rack->registerCv(initdata->prefix, "attack", [&](const int val) { cv_mo_attack = val; });
    initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val) { mo_decay = val; });
    initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val) { cv_mo_decay = val; });

    this->enabled = false;
}

void DrumRackMO::Process(const DrumRackProcessData &data) {
    std::fill_n(mo_out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    // ad envelope and loop
    float a = mo_attack / 4095.f * 5.f;
    float d = mo_decay / 4095.f * 5.f;
    if (cv_mo_attack != -1) {
        a = fabsf(data.cv[cv_mo_attack]) * 12.f;
    }
    if (cv_mo_decay != -1) {
        d = fabsf(data.cv[cv_mo_decay]) * 12.f;
    }
    mo_envelope.SetAttack(a);
    mo_envelope.SetDecay(d);
    if (trig_mo_loopEG != -1) {
        mo_envelope.SetLoop(data.trig[trig_mo_loopEG] == 1 ? false : true);
    } else {
        mo_envelope.SetLoop(mo_loopEG);
    }
    int32_t ad_value = static_cast<uint32_t>(mo_envelope.Process() * 65535.f);

    // shape
    int s = mo_shape;
    if (cv_mo_shape != -1) {
        s = fabsf(data.cv[cv_mo_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    mo_osc.set_shape(ms);

    bool trigger = false;
    if (trig_mo_enableEG != -1) {
        trigger = data.trig[trig_mo_enableEG] == 1 ? false : true;
    } else {
        trigger = mo_enableEG;
    }

    if (!mo_prevTrigger && trigger) {
        //envelope.Trigger(braids::EnvelopeSegment::ENV_SEGMENT_ATTACK);
        mo_envelope.Trigger();
        mo_osc.Strike();
    }
    mo_prevTrigger = trigger;

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2];
    parameters[0] = mo_param_0;
    parameters[1] = mo_param_1;
    if (cv_mo_param_0 != -1) {
        parameters[0] = static_cast<int16_t>(fabsf(data.cv[cv_mo_param_0] * 32767));
    }
    if (cv_mo_param_1 != -1) {
        parameters[1] = static_cast<int16_t>(fabsf(data.cv[cv_mo_param_1] * 32767));
    }
    int32_t mod_amt[2];
    mod_amt[0] = mo_p0_amt;
    mod_amt[1] = mo_p1_amt;
    int32_t mod[2];
    if (cv_mo_p0_amt != -1) {
        mod[0] = static_cast<int32_t >(data.cv[cv_mo_p0_amt] * 65535.f);
    } else {
        mod[0] = ad_value;
    }
    if (cv_mo_p1_amt != -1) {
        mod[1] = static_cast<int32_t >(data.cv[cv_mo_p1_amt] * 65535.f);
    } else {
        mod[1] = ad_value;
    }
    for (int i = 0; i < 2; ++i) {
        int32_t value = parameters[i];
        value += (mod[i] * mod_amt[i]) / 64;
        CONSTRAIN(value, 0, 32767);
        parameters[i] = value;
    }
    mo_osc.set_parameters(parameters[0], parameters[1]);

    // pitch calculation and quantization + fm
    int32_t ipitch = mo_pitch;
    if (cv_mo_pitch != -1) {
        ipitch += static_cast<int32_t>(data.cv[cv_mo_pitch] * 12.f * 5.f * 128.f); // five octaves
    }
    int32_t sc = mo_q_scale;
    if (cv_mo_q_scale != -1) {
        sc = static_cast<int32_t>(fabsf(data.cv[cv_mo_q_scale]) * 48.f);
        CONSTRAIN(sc, 0, 47);
    }
    mo_quantizer.Configure(braids::scales[sc]);
    ipitch = mo_quantizer.Process(ipitch, mo_pitch);

    int32_t fm = mo_fm_amt * ad_value / 512;
    if (cv_mo_fm_amt != -1) {
        fm = static_cast<int32_t>(data.cv[cv_mo_fm_amt] * 12.f * 3.f * 128.f); // three octaves
    }
    ipitch += fm;
    CONSTRAIN(ipitch, 0, 16383);
    mo_osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[BUF_SZ];
    mo_osc.Render(mo_sync, buffer, BUF_SZ);

    // calculate amplitude modulation
    int32_t mod_gain = 65535;
    mod_gain = (ad_value) / 16;

    // convert final audio buffer
    int32_t sample = 0;
    uint16_t signature = mo_waveshaping;
    if (cv_mo_waveshaping != -1) {
        signature = static_cast<uint16_t>(fabsf(data.cv[cv_mo_waveshaping]) * 65535.f);
    }
    int32_t dfactor = mo_decimation;
    if (cv_mo_decimation != -1) {
        dfactor = static_cast<int32_t>(fabsf(data.cv[cv_mo_decimation]) * 30) + 1;
    }
    int32_t br = mo_bit_reduction;
    if (cv_mo_bit_reduction != -1) {
        br = static_cast<int32_t>(fabsf(data.cv[cv_mo_bit_reduction]) * 6);
    }
    int16_t bit_mask = mo_bit_reduction_masks[6 - br];
    for (int i = 0; i < BUF_SZ; i++) {
        if ((i % dfactor) == 0) {
            sample = buffer[i] & bit_mask;
        }
        int16_t warped = mo_ws.Transform(sample);
        buffer[i] = stmlib::Mix(sample, warped, signature);
        buffer[i] = buffer[i] * mod_gain / 65535;
        mo_out[i] = static_cast<float>(buffer[i]) / 32767.f;
    }
}

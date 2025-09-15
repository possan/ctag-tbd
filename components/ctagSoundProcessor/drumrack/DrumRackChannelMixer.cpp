#include "DrumRackSynth.hpp"
#include "DrumRackChannelMixer.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define minVolume 0.000001f
#define maxFXSendLevelDly 2.f
#define maxFXSendLevelRev 1.5f

void DrumRackChannelMixer::Init(const DrumRackInitData *initdata) {
	cc_base = initdata->cc_base;

	initdata->rack->registerParamAndCC(initdata, "device", 0, [&](const int val){ mix_device = val;});
	initdata->rack->registerParamAndCC(initdata, "lev", 1, [&](const int val){ mix_lev = val;});
	initdata->rack->registerParamAndCC(initdata, "pan", 2, [&](const int val){ mix_pan = val;});
	initdata->rack->registerParamAndCC(initdata, "fx1", 3, [&](const int val){ mix_fx1 = val;});
	initdata->rack->registerParamAndCC(initdata, "fx2", 4, [&](const int val){ mix_fx2 = val;});

	this->enabled = true;
	this->device = -1;
}

void DrumRackChannelMixer::PreProcess(const DrumRackProcessData &data) {
    MK_FLT_PAR_ABS_NOCV(fDev, mix_device, 4095.f, 4095.f);
    MK_FLT_PAR_ABS_NOCV(fPan, mix_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fLev, mix_lev, 4095.f, 2.f); fLev *= fLev;
    MK_FLT_PAR_ABS_NOCV(fFX1Send, mix_fx1, 4095.f, maxFXSendLevelDly); fFX1Send *= fFX1Send;
    MK_FLT_PAR_ABS_NOCV(fFX2Send, mix_fx2, 4095.f, maxFXSendLevelRev); fFX2Send *= fFX2Send;

    this->enabled = fLev > minVolume;

	if (this->mix_device != this->device) {
		// ESP_LOGI("DrumRackChannelMixer", "Device changed from %d to %d", this->device, (int)this->mix_device);
		this->device = this->mix_device;
	}

	float _pan = (fPan * 2.0f) - 1.0f;
	if (_pan != this->pan) {
		// ESP_LOGI("DrumRackChannelMixer", "Pan changed from %f to %f", this->pan, _pan);
		this->pan = _pan;
	}
	this->level = fLev;
	this->send1 = fFX1Send;
	this->send2 = fFX2Send;
}

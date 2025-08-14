#include "DrumRackSynth.hpp"
#include "DrumRackChannelMixer.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define minVolume 0.000001f
#define maxFXSendLevelDly 2.f
#define maxFXSendLevelRev 1.5f

void DrumRackChannelMixer::Init(const DrumRackInitData *initdata) {
	initdata->rack->registerParam(initdata->prefix, "mute", [&](const int val){ mix_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "mute", [&](const int val){ trig_mix_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "device", [&](const int val){ mix_device = val;});
	initdata->rack->registerCv(initdata->prefix, "device", [&](const int val){ cv_mix_device = val;});
	initdata->rack->registerParam(initdata->prefix, "lev", [&](const int val){ mix_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "lev", [&](const int val){ cv_mix_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "pan", [&](const int val){ mix_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "pan", [&](const int val){ cv_mix_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "fx1", [&](const int val){ mix_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "fx1", [&](const int val){ cv_mix_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "fx2", [&](const int val){ mix_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "fx2", [&](const int val){ cv_mix_fx2 = val;});
	this->enabled = false;
	this->device = -1;
}

void DrumRackChannelMixer::PreProcess(const DrumRackProcessData &data) {
    MK_BOOL_PAR(bMute, mix_mute)
    MK_FLT_PAR_ABS(fDev, mix_device, 4095.f, 10.f);
    MK_FLT_PAR_ABS_PAN(fPan, mix_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fLev, mix_lev, 4095.f, 2.f); fLev *= fLev;
    MK_FLT_PAR_ABS(fFX1Send, mix_fx1, 4095.f, maxFXSendLevelDly); fFX1Send *= fFX1Send;
    MK_FLT_PAR_ABS(fFX2Send, mix_fx2, 4095.f, maxFXSendLevelRev); fFX2Send *= fFX2Send;

    this->enabled = (!bMute && fLev > minVolume);

	int idev = 	(int)fDev;
	if (idev != this->device) {
		ESP_LOGI("DrumRackChannelMixer", "Device changed from %d to %d", this->device, idev);
		this->device = idev;
	}

	this->pan = fPan;
	this->level = fLev;
	this->send1 = fFX1Send;
	this->send2 = fFX2Send;
}

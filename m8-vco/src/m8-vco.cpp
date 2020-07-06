#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2.h>

#include "Oscillators.h"

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

/**********************************************************************************************************************************************************/
#define PLUGIN_URI "http://VeJaPlugins.com/plugins/Release/m8vco"
#define MAX_OUTPUT_BUFFER_LENGHT 256
#define VCF_LOW_PASS_MODE 0
#define WAVETABLE_SIZE 32768

using namespace VeJa::Plugins::Oscillators;

enum{
    CvPitchInput,
    CvVelocityInput,
    CvGateInput,
    CvMODinput,
    AUDIO_output,
    VCO_Waveform,
    VCO_Mode,
    VCO_Range,
    VCO_Finetune,
    PWM_MODE,
    Pulse_Width,
    MOD_inp_C
};


class Mars_8{
public:
    Mars_8()
    {
        vco = new Oscillator<float>(48000 ,440 ,WAVETABLE_SIZE);
    }
    ~Mars_8() {}
    static LV2_Handle instantiate(const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features);
    static void activate(LV2_Handle instance);
    static void deactivate(LV2_Handle instance);
    static void connect_port(LV2_Handle instance, uint32_t port, void *data);
    static void run(LV2_Handle instance, uint32_t n_samples);
    static void cleanup(LV2_Handle instance);
    static const void* extension_data(const char* uri);
        
    float *cv_pitch_input;
    float *cv_velocity_input;
    float *cv_gate_put;
    float *cv_mod_input;
    float *output;
    float *vco_waveform;
    float *vco_mode;
    float *vco_range;
    float *vco_finetune;
    float *pwm_mode;
    float *pulse_width;
    float *crossmod_input;
    float *mod_inp_c;

    //important stuff
    float Tau;
    float sampleRate;

    float prev_velocity;

    Oscillator<float> * vco;

};
/**********************************************************************************************************************************************************/

float  MAP(float x, float Omin, float Omax, float Nmin, float Nmax)
{
    return (( x - Omin ) * (Nmax -  Nmin)  / (Omax - Omin) + Nmin);
}      

/**********************************************************************************************************************************************************/
LV2_Handle Mars_8::instantiate(const LV2_Descriptor*   descriptor,
double                              samplerate,
const char*                         bundle_path,
const LV2_Feature* const* features)
{
    Mars_8* self = new Mars_8();

    return (LV2_Handle)self; 
}
/**********************************************************************************************************************************************************/
void Mars_8::connect_port(LV2_Handle instance, uint32_t port, void *data)
{
    Mars_8* self = (Mars_8*)instance;
    switch (port)
    {
        case CvPitchInput:
            self->cv_pitch_input = (float*) data;
            break;
        case CvVelocityInput:
            self->cv_velocity_input = (float*) data;
            break;
        case CvGateInput:
            self->cv_gate_put = (float*) data;
            break;
        case CvMODinput:
            self->cv_mod_input = (float*) data;
            break;
        case AUDIO_output:
            self->output = (float*) data;
            break;
        case VCO_Waveform:
            self->vco_waveform = (float*) data;
            break;
        case VCO_Mode:
            self->vco_mode = (float*) data;
            break;
        case VCO_Range:
            self->vco_range = (float*) data;
            break;
        case VCO_Finetune:
            self->vco_finetune = (float*) data;
            break;
        case PWM_MODE:
            self->pwm_mode = (float*) data;
            break;
        case Pulse_Width:
            self->pulse_width = (float*) data;
            break;
        case MOD_inp_C:
            self->mod_inp_c = (float*) data;
            break;
    }
}
/**********************************************************************************************************************************************************/
void Mars_8::activate(LV2_Handle instance)
{
}

/**********************************************************************************************************************************************************/
void Mars_8::run(LV2_Handle instance, uint32_t n_samples)
{
    Mars_8* self = (Mars_8*)instance;

    float vco_wave = *self->vco_waveform;
    float vco_low = *self->vco_mode;
    float range = *self->vco_range;
    float finetune = *self->vco_finetune;
    float pwm_sync = *self->pwm_mode;
    float pwm = *self->pulse_width;
    float modulation = *self->mod_inp_c;

  	//start audio processing
    for(uint32_t i = 0; i < n_samples; i++)
    {
        float cv_pitch = self->cv_pitch_input[i];
        float cv_velocity = self->cv_velocity_input[i];
        float cv_gate = self->cv_gate_put[i];
        float cv_mod = self->cv_mod_input[i];

        if (cv_velocity != 0)
        {
            self->prev_velocity = cv_velocity;
        }

        if ((cv_gate == 0) && (cv_velocity == 0))
        {   
            self->prev_velocity = 0;
        }
        
        if (pwm_sync == 0)
        {
            self->vco->SetPulseWidth(cv_mod / 10);
        }
        else 
        {
           self->vco->SetPulseWidth(pwm); 
        }
        

        float vco_freq = 440 / pow(2,5.75) * pow(2, (cv_pitch + vco_low + range + (finetune / 10)));

        //update frequency based on cross mod and vco modulation
        self->vco->NewFrequency(vco_freq + (cv_mod * modulation));

        //get the audio sample
        self->output[i] = self->vco->GetDataValue((int)vco_wave) * (self->prev_velocity/10);
        self->vco->UpdateFrequency();
    }
}   

/**********************************************************************************************************************************************************/
void Mars_8::deactivate(LV2_Handle instance)
{
    // TODO: include the deactivate function code here
}
/**********************************************************************************************************************************************************/
void Mars_8::cleanup(LV2_Handle instance)
{
  delete ((Mars_8 *) instance); 
}
/**********************************************************************************************************************************************************/
static const LV2_Descriptor Descriptor = {
    PLUGIN_URI,
    Mars_8::instantiate,
    Mars_8::connect_port,
    Mars_8::activate,
    Mars_8::run,
    Mars_8::deactivate,
    Mars_8::cleanup,
    Mars_8::extension_data
};
/**********************************************************************************************************************************************************/
LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index == 0) return &Descriptor;
    else return NULL;
}
/**********************************************************************************************************************************************************/
const void* Mars_8::extension_data(const char* uri)
{
    return NULL;
}

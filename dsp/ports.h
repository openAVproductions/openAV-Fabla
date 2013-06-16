
#ifndef FABLA_PORTS_H
#define FABLA_PORTS_H

// Plugin's URI
#define FABLA_URI    "http://www.openavproductions.com/fabla"
#define FABLA_UI_URI "http://www.openavproductions.com/fabla/gui"

// Lv2 includes
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

typedef enum {
  ATOM_IN= 0,
  ATOM_OUT,
  
  AUDIO_OUT_L,
  AUDIO_OUT_R,
  
  MASTER_VOL,
  
  OSC_1_MOD,
  OSC_1_VOL,
  
  OSC_2_MOD,
  OSC_2_VOL,
  
  OSC_3_MOD,
  OSC_3_VOL,
  
  LFO_MOD_AMP,
  LFO_MOD_SPEED,
  
  LFO_FREQ_AMP,
  LFO_FREQ_SPEED,
  
  FILTER_FREQ,
  FILTER_Q,
  FILTER_TYPE,
  
  REVERB_SIZE,
  REVERB_DAMPING,
  REVERB_WET,
  REVERB_ENABLE,
  
} PortIndex;


#endif // FABLA_PORTS_H


/*
 * Author: Harry van Haaren 2013
 *         harryhaaren@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef SHARED_H
#define SHARED_H

#include <string>
#include <sstream>

#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"

#define LV2_MIDI__MidiEvent "http://lv2plug.in/ns/ext/midi#MidiEvent"

#include "ports.h"

#define UI_WAVEFORM_PIXELS 324

typedef struct {
  LV2_URID atom_Blank;
  LV2_URID atom_Object;
  LV2_URID atom_URID;
  LV2_URID atom_Path;
  LV2_URID atom_Resource;
  LV2_URID atom_Sequence;
  LV2_URID atom_Vector;
  LV2_URID atom_Float;
  LV2_URID atom_Chunk;
  LV2_URID atom_eventTransfer;
  
  LV2_URID time_Position;
  LV2_URID time_barBeat;
  LV2_URID time_beatsPerMinute;
  LV2_URID time_speed;
  
  LV2_URID log_Error;
  LV2_URID log_Trace;
  
  LV2_URID midi_Event;

  LV2_URID patch_Set;
  LV2_URID patch_Get;
  LV2_URID patch_property;
  LV2_URID patch_value;
  LV2_URID patch_body;
  
  // Fabla specific
  LV2_URID fabla_Play;
  LV2_URID fabla_Stop;
  LV2_URID fabla_Load;
  LV2_URID fabla_Unload;
  LV2_URID fabla_MeterLevels;
  LV2_URID fabla_Waveform;
  LV2_URID fabla_WaveformMsgNum;
  
  LV2_URID fabla_UiRequestPaths;
  
  LV2_URID fabla_pad;
  LV2_URID fabla_filename;
  
  LV2_URID fabla_level_l;
  LV2_URID fabla_level_r;
  LV2_URID fabla_waveformData;
  
  LV2_URID padFilename[16];
  LV2_URID padFpath[16];
  
} Fabla_URIs;


static inline void
map_uris(LV2_URID_Map* map, Fabla_URIs* uris)
{
  uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
  uris->atom_Object        = map->map(map->handle, LV2_ATOM__Object);
  uris->atom_URID          = map->map(map->handle, LV2_ATOM__URID);
  uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
  uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
  uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
  uris->atom_Vector        = map->map(map->handle, LV2_ATOM__Vector);
  uris->atom_Float         = map->map(map->handle, LV2_ATOM__Float);
  uris->atom_Chunk         = map->map(map->handle, LV2_ATOM__Chunk);
  uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);

  uris->time_Position      = map->map(map->handle, LV2_TIME__Position);
  uris->time_barBeat       = map->map(map->handle, LV2_TIME__barBeat);
  uris->time_beatsPerMinute= map->map(map->handle, LV2_TIME__beatsPerMinute);
  uris->time_speed         = map->map(map->handle, LV2_TIME__speed);
    
  uris->log_Error          = map->map(map->handle, LV2_LOG__Error);
  uris->log_Trace          = map->map(map->handle, LV2_LOG__Trace);
  
  uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);

  uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
  uris->patch_Get          = map->map(map->handle, LV2_PATCH__Get);
  uris->patch_property     = map->map(map->handle, LV2_PATCH__property);
  uris->patch_value        = map->map(map->handle, LV2_PATCH__value);
  uris->patch_body         = map->map(map->handle, LV2_PATCH__body);

  uris->fabla_Play         = map->map(map->handle, FABLA_URI"#Play");
  uris->fabla_Stop         = map->map(map->handle, FABLA_URI"#Stop");
  uris->fabla_Load         = map->map(map->handle, FABLA_URI"#Load");
  uris->fabla_Unload       = map->map(map->handle, FABLA_URI"#Unload");
  uris->fabla_MeterLevels  = map->map(map->handle, FABLA_URI"#MeterLevels");
  uris->fabla_Waveform     = map->map(map->handle, FABLA_URI"#Waveform");
  uris->fabla_WaveformMsgNum=map->map(map->handle, FABLA_URI"#WaveformMsgNum");
  
  uris->fabla_UiRequestPaths=map->map(map->handle, FABLA_URI"#UiRequestPaths");
  
  uris->fabla_pad          = map->map(map->handle, FABLA_URI"#pad");
  uris->fabla_level_l      = map->map(map->handle, FABLA_URI"#level_r");
  uris->fabla_level_r      = map->map(map->handle, FABLA_URI"#level_l");
  uris->fabla_filename     = map->map(map->handle, FABLA_URI"#filename");
  uris->fabla_waveformData = map->map(map->handle, FABLA_URI"#waveformData");

  //34
  // Sample restore URI's  per pad
  for ( int i = 0; i < 16; i++ )
  {
    std::stringstream s;
    s << FABLA_URI"#pad_" << i << "_filename";
    uris->padFilename[i] = map->map(map->handle, s.str().c_str() );
  }

  // Sample file path properties
  for ( int i = 0; i < 16; i++ )
  {
    std::stringstream s;
    s << FABLA_URI"#pad_fpath_" << i+1;
    uris->padFpath[i] = map->map(map->handle, s.str().c_str() );
  }
}


class UIPadData
{
  public:
    UIPadData()
    {
      loaded = false;
      
      gain  = 0.5;
      speed = 0.5;
      pan   = 0.5;
      
      a     = 0;
      d     = 0;
      s     = 1;
      r     = 0;
      
      memset( &waveform[0], 0, sizeof(float)*UI_WAVEFORM_PIXELS );
    }
    
    bool loaded;
    
    std::string name;
    
    float gain;
    float speed;
    float pan;
    
    float a;
    float d;
    float s;
    float r;
    
    // stores waveform data for UI
    long waveformLength;
    float waveform[UI_WAVEFORM_PIXELS];
};


#endif // SHARED_H


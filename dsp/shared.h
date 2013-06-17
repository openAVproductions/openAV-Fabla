
#ifndef SHARED_H
#define SHARED_H

#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"

#define LV2_MIDI__MidiEvent "http://lv2plug.in/ns/ext/midi#MidiEvent"

#include "ports.h"


typedef struct {
  LV2_URID atom_Blank;
  LV2_URID atom_Path;
  LV2_URID atom_Resource;
  LV2_URID atom_Sequence;
  LV2_URID atom_Vector;
  LV2_URID atom_Float;
  LV2_URID atom_eventTransfer;
  
  LV2_URID time_Position;
  LV2_URID time_barBeat;
  LV2_URID time_beatsPerMinute;
  LV2_URID time_speed;
  
  LV2_URID log_Error;
  LV2_URID log_Trace;
  
  LV2_URID midi_Event;
  
  LV2_URID patch_Set;
  LV2_URID patch_body;
  
  LV2_URID fabla_Play;
  LV2_URID fabla_Stop;
  LV2_URID fabla_pad;
  
} Fabla_URIs;


static inline void
map_uris(LV2_URID_Map* map, Fabla_URIs* uris)
{
  uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
  uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
  uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
  uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
  uris->atom_Vector        = map->map(map->handle, LV2_ATOM__Vector);
  uris->atom_Float         = map->map(map->handle, LV2_ATOM__Float);
  uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
  
  uris->time_Position      = map->map(map->handle, LV2_TIME__Position);
  uris->time_barBeat       = map->map(map->handle, LV2_TIME__barBeat);
  uris->time_beatsPerMinute= map->map(map->handle, LV2_TIME__beatsPerMinute);
  uris->time_speed         = map->map(map->handle, LV2_TIME__speed);
    
  uris->log_Error          = map->map(map->handle, LV2_LOG__Error);
  uris->log_Trace          = map->map(map->handle, LV2_LOG__Trace);
  
  uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
  
  uris->fabla_Play         = map->map(map->handle, FABLA_URI"#Play");
  uris->fabla_Stop         = map->map(map->handle, FABLA_URI"#Stop");
  uris->fabla_pad          = map->map(map->handle, FABLA_URI"#pad");
}


#endif // SHARED_H


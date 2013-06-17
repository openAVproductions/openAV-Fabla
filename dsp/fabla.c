
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"

#include "voice.hxx"
#include "denormals.hxx"

#define NVOICES 16

static const char* default_sample = "click.wav";

typedef struct {
        LV2_Atom atom;
        Sample*  sample;
} SampleMessage;


typedef struct {
  // instantiate values
  int sr;
  
  // port values
  float* master;
  
  float* output_L;
  float* output_R;
  
  // URID map
  LV2_URID_Map* map;
  
  // Worker threads
  LV2_Worker_Schedule* schedule;
  
  // Lv2 Log
  LV2_Log_Log*   log;
  LV2_Log_Logger logger; // convienience
  
  // Atom ports
  LV2_Atom_Sequence*   control_port;
  LV2_Atom_Sequence*   notify_port;
  
  Fabla_URIs* uris;
  LV2_Atom_Forge forge;
  LV2_Atom_Forge_Frame notify_frame;
  
  // LV2 Time 
  float  bpm;    // Beats per minute (tempo)
  float  speed;  // Transport speed (usually 0=stop, 1=play)
  
  Voice* voice[NVOICES];
  
} FABLA_DSP;


static Sample* load_sample(FABLA_DSP* self, const char* path)
{
  const size_t path_len  = strlen(path);
  
  lv2_log_trace(&self->logger, "Loading sample %s\n", path);
  
  Sample* sample = new Sample();
  SF_INFO* const info    = &sample->info;
  SNDFILE* const sndfile = sf_open(path, SFM_READ, info);
  
  if (!sndfile || !info->frames || (info->channels != 1)) {
    lv2_log_error(&self->logger, "Failed to open sample '%s'\n", path);
    free(sample);
    return NULL;
  }
  else
  {
    lv2_log_error(&self->logger, "Sample has '%i' samples\n", info->frames);
  }
  
  // Read data
  float* const data = (float*)malloc(sizeof(float) * info->frames);
  if (!data) {
    lv2_log_error(&self->logger, "Failed to allocate memory for sample\n");
    return NULL;
  }
  sf_seek(sndfile, 0ul, SEEK_SET);
  sf_read_float(sndfile, data, info->frames);
  sf_close(sndfile);
  
  // Fill sample struct and return it
  sample->data     = data;
  sample->path     = (char*)malloc(path_len + 1);
  sample->path_len = path_len;
  memcpy(sample->path, path, path_len + 1);
  
  lv2_log_error(&self->logger, "Loaded sample:\n\t %i samples\n\tdata = %i", info->frames, sample->data);
  
  return sample;
}

static void
free_sample(FABLA_DSP* self, Sample* sample)
{
  if (sample) {
    lv2_log_trace(&self->logger, "Freeing %s\n", sample->path);
    free(sample->path);
    free(sample->data);
    free(sample);
  }
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  AVOIDDENORMALS();
  
  // Initialize self
  FABLA_DSP* self = (FABLA_DSP*)malloc(sizeof(FABLA_DSP));
  memset(self, 0, sizeof(FABLA_DSP));
  
  self->uris  = (Fabla_URIs*)malloc(sizeof(Fabla_URIs));
  memset(self, 0, sizeof(Fabla_URIs));
  
  self->sr  = rate;
  self->bpm = 120.0f;
  
  // act on host features
  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map)) {
      self->map = (LV2_URID_Map*)features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_WORKER__schedule)) {
      self->schedule = (LV2_Worker_Schedule*)features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_LOG__log)) {
      self->log = (LV2_Log_Log*)features[i]->data;
    }
  }
  
  
  
  // allocate voices
  for(int i = 0; i < NVOICES; i++)
    self->voice[i] = new Voice(rate);
  
  // map all known URIs to ints
  map_uris( self->map, self->uris );
  
  // then init the forge to write Atoms
  lv2_atom_forge_init(&self->forge, self->map);
  
  // Load the default sample file
  const size_t path_len    = strlen(bundle_path);
  const size_t file_len    = strlen(default_sample);
  const size_t len         = path_len + file_len;
  char*        sample_path = (char*)malloc(len + 1);
  snprintf(sample_path, len + 1, "%s%s", bundle_path, default_sample);
  
  Sample* sample = load_sample(self, sample_path);
  
  for(int i = 0; i < NVOICES; i++)
    self->voice[i]->sample = sample;
  
  free(sample_path);
  
  return (LV2_Handle)self;
}


static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
  FABLA_DSP* self = (FABLA_DSP*)instance;
  
  switch ((PortIndex)port)
  {
    case ATOM_IN:
      self->control_port = (LV2_Atom_Sequence*)data; break;
    case ATOM_OUT:
      self->notify_port = (LV2_Atom_Sequence*)data; break;
    
    case AUDIO_OUT_L:
      self->output_L = (float*)data; break;
    case AUDIO_OUT_R:
      self->output_R = (float*)data; break;
    
    case MASTER_VOL:
      self->master = (float*)data; break;
    
    default:
      printf("Error: Attempted connect of non-existing port with ID %u \n", port); break;
  }
}


static void
activate(LV2_Handle instance)
{
}

#define DB_CO(g) ((g) > 0.0001f ? powf(10.0f, (g) * 0.05f) : 0.0f)

static void
run(LV2_Handle instance, uint32_t n_samples)
{
  FABLA_DSP* self = (FABLA_DSP*)instance;
  
  const float        gain   = *(self->master);
  float* const       outputL = self->output_L;
  float* const       outputR = self->output_R;
  
  // zero output buffer
  memset ( outputL, 0, n_samples );
  memset ( outputR, 0, n_samples );
  
  // Set up forge to write directly to notify output port
  const uint32_t notify_capacity = self->notify_port->atom.size;
  lv2_atom_forge_set_buffer(&self->forge, (uint8_t*)self->notify_port, notify_capacity);
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);
  
  LV2_ATOM_SEQUENCE_FOREACH(self->control_port, ev)
  {
    //self->frame_offset = ev->time.frames;
    
    if (ev->body.type == self->uris->midi_Event)
    {
      uint8_t* const data = (uint8_t* const)(ev + 1);
      if ( (data[0] & 0xF0) == 0x90 ) // event & channel
      {
        //lv2_log_note(&self->logger, "Note on : %d\n", data[1] );
        lv2_atom_forge_frame_time(&self->forge, 0);
        LV2_Atom_Forge_Frame set_frame;
        LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_Play, 0);
        LV2_Atom_Forge_Frame body_frame;
        lv2_atom_forge_blank(&self->forge, &body_frame, 2, 0);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_pad, 0);
        lv2_atom_forge_int(&self->forge, int(data[1]) );
        
        // use next available voice for the note
        int n = int(data[1]);
        bool alloced = false;
        for(int i = 0; i < NVOICES; i++)
        {
          if ( !self->voice[i]->playing() )
          {
            self->voice[i]->play( n, int(data[2]) );
            lv2_log_note(&self->logger, "Voice %i gets note ON %i\n", i, n );
            alloced = true;
            break;
          }
        }
        if ( !alloced )
          lv2_log_note(&self->logger, "Note %i ON: but no voice available\n", n );
      }
      else if ( (data[0] & 0xF0) == 0x80 )
      {
        //lv2_log_note(&self->logger, "Note off: %d\n", data[1] );
        lv2_atom_forge_frame_time(&self->forge, 0);
        
        LV2_Atom_Forge_Frame set_frame;
        LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_Stop, 0);
        LV2_Atom_Forge_Frame body_frame;
        lv2_atom_forge_blank(&self->forge, &body_frame, 2, 0);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_pad, 0);
        lv2_atom_forge_int(&self->forge, int(data[1]) );
        
        // release ADSR of note
        for(int i = 0; i < NVOICES; i++)
        {
          self->voice[i]->stopIfNoteEquals( int(data[1]) );
        }
      }
    }
    
    if (ev->body.type == self->uris->atom_Blank)
    {
      const LV2_Atom_Object* obj = (LV2_Atom_Object*)&ev->body;
      if (obj->body.otype == self->uris->time_Position)
      {
        // Receive position information, update
        LV2_Atom *beat = NULL, *bpm = NULL, *speed = NULL;
        lv2_atom_object_get(obj,
                            self->uris->time_barBeat, &beat,
                            self->uris->time_beatsPerMinute, &bpm,
                            self->uris->time_speed, &speed,
                            NULL);
        
        if (bpm && bpm->type == self->uris->atom_Float) {
          // Tempo changed, update BPM
          self->bpm = ((LV2_Atom_Float*)bpm)->body;
          const float frames_per_beat = 60.0f / self->bpm * self->sr;
          lv2_log_note(&self->logger, "Tempo change to %f, fpb at %f\n", self->bpm, frames_per_beat);
        }
        if (speed && speed->type == self->uris->atom_Float) {
          // Speed changed, e.g. 0 (stop) to 1 (play)
          self->speed = ((LV2_Atom_Float*)speed)->body;
        }
        if (beat && beat->type == self->uris->atom_Float) {
          // Received a beat position, synchronise
          // This hard sync may cause clicks, a real plugin would be more graceful
          const float frames_per_beat = 60.0f / self->bpm * self->sr;
          const float bar_beats       = ((LV2_Atom_Float*)beat)->body;
          const float beat_beats      = bar_beats - floorf(bar_beats);
          
          /*
          self->elapsed_len           = beat_beats * frames_per_beat;
          if (self->elapsed_len < self->attack_len) {
                  self->state = STATE_ATTACK;
          } else if (self->elapsed_len < self->attack_len + self->decay_len) {
                  self->state = STATE_DECAY;
          } else {
                  self->state = STATE_OFF;
          }
          */
        }
      }
    }
  }
  
  
  for (uint32_t pos = 0; pos < n_samples; pos++)
  {
    float accumL;
    float accumR;
    
    for(int i = 0; i < NVOICES; i++ )
    {
      self->voice[i]->process( 1, &accumL, &accumR );
    }
    
    outputL[pos] = accumL * gain;
    outputR[pos] = accumR * gain;
  }
}


static void
deactivate(LV2_Handle instance)
{
}

static void
cleanup(LV2_Handle instance)
{
  free(instance);
}

const void*
extension_data(const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	FABLA_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}

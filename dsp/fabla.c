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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

#include "voice.hxx"
#include "sample.hxx"
#include "denormals.hxx"
#include "dsp_dbmeter.hxx"
#include "dsp_compressor.hxx"

#define NVOICES 64

class SampleMessage
{
  public:
    SampleMessage(LV2_URID type)
    {
      //atom.type = type;
      //atom.size = sizeof(int) + sizeof(Sample*);
      pad = -1;
      sample = 0;
    }
    //LV2_Atom atom;
    int pad;
    Sample*  sample;
};

class PadData
{
  public:
    PadData()
    {
      gain  = 0;
      speed = 0;
      
      a     = 0;
      d     = 0;
      s     = 0;
      r     = 0;
    }
    
    float* gain;
    float* speed;
    float* pan;
    
    float* a;
    float* d;
    float* s;
    float* r;
};

typedef struct {
  // instantiate values
  int sr;
  
  // port values
  float* master;
  float w, a, b, g1, g2; // smoothing variables
  
  float* output_L;
  float* output_R;
  
  float* comp_attack;
  float* comp_decay;
  float* comp_thres;
  float* comp_ratio;
  float* comp_makeup;
  float* comp_enable;
  
  PadData padData[16];
  
  // URID map
  LV2_URID_Map* map;
  LV2_URID_Unmap* unmap;
  
  // Worker threads
  bool haveWorkerExtension;
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
  Sample* samples[16];
  Compressor* comp;
  
  // UI related stuff
  int uiUpdateCounter;
  DBMeter* meter;
  
  // if true, re-write the path names to UI
  // restore triggers this, to load UI waveforms
  bool updateUiPaths;
  int  updateUiPathCounter;
  
} FABLA_DSP;


static Sample* load_sample(FABLA_DSP* self, const char* path)
{
  const size_t path_len  = strlen(path);
  
  //lv2_log_note(&self->logger, "Loading sample %s\n", path);
  
  Sample* sample = new Sample();
  SF_INFO* const info    = &sample->info;
  SNDFILE* const sndfile = sf_open(path, SFM_READ, info);
  
  if (!sndfile) // || !info->frames ) { // || (info->channels != 1)) {
  {
    lv2_log_error(&self->logger, "Failed to open sample '%s'\n", path);
    free(sample);
    return NULL;
  }
  
  //lv2_log_note(&self->logger, "Sample has '%li' samples\n", info->frames);
  
  // Read data
  float* data = (float*)malloc(sizeof(float) * info->frames * info->channels );
  if (!data) {
    lv2_log_error(&self->logger, "Failed to allocate memory for sample\n");
    return NULL;
  }
  
  sf_seek(sndfile, 0ul, SEEK_SET);
  sf_read_float(sndfile, data, info->frames * info->channels);
  sf_close(sndfile);
  
  int chnls = info->channels;
  if ( chnls > 1 )
  {
    lv2_log_note(&self->logger, "Sample '%s' has %i channels: using channel 1\n", path, chnls);
    // we're gonna kick all samples that are *not* channel 1
    float* tmp = (float*)malloc( sizeof(float) * info->frames );
    
    printf("Non mono file: %i chnls found, old size %li, new size %li \n", chnls,info->channels * info->frames, info->frames );
    for(unsigned int i = 0; i < info->frames; i++ )
    {
      tmp[i] = data[ i * chnls ];
    }
    
    // swap buffer, freeing used "multi-channel" buffer
    free( data );
    data = tmp;
  }
  
  // Fill sample struct and return it
  sample->data     = data;
  sample->path     = (char*)malloc(path_len + 1);
  sample->path_len = path_len;
  memcpy(sample->path, path, path_len + 1);
  //lv2_log_error(&self->logger, "Loaded sample:\n\t %i samples\n\tPath: %s\n", int(info->frames), sample->path);
  
  return sample;
}

static void
free_sample(FABLA_DSP* self, Sample* sample)
{
  if (sample) {
    //lv2_log_trace(&self->logger, "Freeing %s\n", sample->path);
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
  
  for(int i = 0; i < 16; i++ )
    self->samples[i] = 0;
  
  self->sr  = rate;
  self->bpm = 120.0f;
  
  self->schedule = 0;
  
  // act on host features
  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map)) {
      self->map = (LV2_URID_Map*)features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_URID__unmap)) {
      self->unmap = (LV2_URID_Unmap*)features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_WORKER__schedule)) {
      self->schedule = (LV2_Worker_Schedule*)features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_LOG__log)) {
      self->log = (LV2_Log_Log*)features[i]->data;
    }
  }
  
  if ( self->schedule == 0 )
  {
    if ( self->log )
      lv2_log_warning(&self->logger, "Fabla: Warning, your host doesn't support the Worker extension. Loading samples may cause Xruns!");
    else
      printf("Fabla: Warning, your host doesn't support the Worker extension. Loading samples may cause Xruns!");
  }
  
  // initialize master output smoothing state
  self->w = 10.0f / (rate * 0.02);
  self->a = 0.07f;
  self->b = 1.0f / (1.0f - self->a);
  self->g1 = self->g2 = 0.0f;
  
  // set UI update variables
  self->updateUiPaths = false;
  self->updateUiPathCounter = 0;
  
  // allocate voices
  for(int i = 0; i < NVOICES; i++)
    self->voice[i] = new Voice(rate);
  
  // allocate meters
  self->uiUpdateCounter = 0;
  self->meter = new DBMeter( rate );
  self->comp  = new Compressor( rate );
  
  // map all known URIs to ints
  map_uris( self->map, self->uris );
  
  // then init the forge to write Atoms
  lv2_atom_forge_init(&self->forge, self->map);
  
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
    
    case COMP_ATTACK:
      self->comp_attack = (float*)data; break;
    case COMP_DECAY:
      self->comp_decay  = (float*)data; break;
    case COMP_THRES:
      self->comp_thres  = (float*)data; break;
    case COMP_RATIO:
      self->comp_ratio  = (float*)data; break;
    case COMP_MAKEUP:
      self->comp_makeup = (float*)data; break;
    case COMP_ENABLE:
      self->comp_enable = (float*)data; break;
    
    // deal with 16 * ADSR / gain / speed / pan here
    case PAD_GAIN:
    case pg2: case pg3: case pg4: case pg5: case pg6: case pg7: case pg8: case pg9:
    case pg10: case pg11: case pg12: case pg13: case pg14: case pg15: case pg16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_GAIN) ].gain = (float*)data;
        //printf("Gain Pad %i, port num %i\n", port - int(PAD_GAIN), port);
        break;
    
    case PAD_SPEED:
    case pspd2: case pspd3: case pspd4: case pspd5: case pspd6: case pspd7: case pspd8: case pspd9:
    case pspd10: case pspd11: case pspd12: case pspd13: case pspd14: case pspd15: case pspd16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_SPEED) ].speed = (float*)data;
        //printf("Speed: Pad %i\n", port);
        break;
    
    case PAD_PAN:
    case pp2: case pp3: case pp4: case pp5: case pp6: case pp7: case pp8: case pp9:
    case pp10: case pp11: case pp12: case pp13: case pp14: case pp15: case pp16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_PAN) ].pan = (float*)data;
        break;
    
    // ADSR
    case PAD_ATTACK:
    case pa2: case pa3: case pa4: case pa5: case pa6: case pa7: case pa8: case pa9:
    case pa10: case pa11: case pa12: case pa13: case pa14: case pa15: case pa16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_ATTACK) ].a = (float*)data;
        break;
    
    case PAD_DECAY:
    case pd2: case pd3: case pd4: case pd5: case pd6: case pd7: case pd8: case pd9:
    case pd10: case pd11: case pd12: case pd13: case pd14: case pd15: case pd16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_DECAY) ].d = (float*)data;
        break;
    
    case PAD_SUSTAIN:
    case ps2: case ps3: case ps4: case ps5: case ps6: case ps7: case ps8: case ps9:
    case ps10: case ps11: case ps12: case ps13: case ps14: case ps15: case ps16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_SUSTAIN) ].s = (float*)data;
        break;
    
    case PAD_RELEASE:
    case pr2: case pr3: case pr4: case pr5: case pr6: case pr7: case pr8: case pr9:
    case pr10: case pr11: case pr12: case pr13: case pr14: case pr15: case pr16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_RELEASE) ].r = (float*)data;
        break;
    
    default:
      printf("Error: Attempted connect of non-existing port with ID %u \n", port); break;
  }
}

static void noteOn(FABLA_DSP* self, int note, int velocity, int frame)
{
  // clip MIDI input, only play 16 samples
  if ( note > 15) note = 15;
  if ( note <  0) note =  0;
  
  if ( !self->samples[note] )
  {
    // no sample loaded
    return;
  }
  
  bool alloced = false;
  for(int i = 0; i < NVOICES; i++)
  {
    // check that the voice isn't playing, and there is a sample loaded
    if ( !self->voice[i]->playing() )
    {
      // set the right sample to the voice
      self->voice[i]->sample = self->samples[note];
      
      // set the ADSR values (copied into voice), 1/2 second control on dial
      self->voice[i]->setAdsr(*self->padData[note].a * 0.5,
                              *self->padData[note].d * 0.5,
                              *self->padData[note].s,
                              *self->padData[note].r * 0.5);
      
      // set voice dependant values: pan / volume
      self->voice[i]->setPan   ( self->samples[note]->pan  );
      
      float vol = self->samples[note]->gain * (velocity / 127.);
      self->voice[i]->setVolume( vol );
      
      // play the voice
      self->voice[i]->play( note, velocity, frame );
      //lv2_log_note(&self->logger, "Voice %i gets note ON %i\n", i, note );
      alloced = true;
      break;
    }
  }
  if ( !alloced )
    lv2_log_note(&self->logger, "Note %i ON: but no voice available\n", note );
  
  //lv2_log_note(&self->logger, "noteOn done\n" );
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
  
  // smoothing algo is a lowpass, to de-zip the master volume slider
  // x^^4 approximates exponential volume control, master vol + makeup gain here
  self->g1 += self->w * ( pow (*self->master, 4.f ) - self->g1 - self->a * self->g2 - 1e-20f);
  self->g2 += self->w * (self->b * self->g1 - self->g2 + 1e-20f);
  const float        gain   = self->g2;
  
  float* const       outputL = self->output_L;
  float* const       outputR = self->output_R;
  
  // zero output buffer
  memset ( outputL, 0, n_samples );
  memset ( outputR, 0, n_samples );
  
  
  // loop over the pads, setting control port values
  for(int i = 0; i < 16; i++)
  {
    if ( self->samples[i] )
    {
      self->samples[i]->gain  = *(self->padData[i].gain );
      self->samples[i]->speed = *(self->padData[i].speed);
      self->samples[i]->pan   = *(self->padData[i].pan  );
    }
  }
  
  // Set up forge to write directly to notify output port
  const uint32_t notify_capacity = self->notify_port->atom.size;
  lv2_atom_forge_set_buffer(&self->forge, (uint8_t*)self->notify_port, notify_capacity);
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);
  
  LV2_ATOM_SEQUENCE_FOREACH(self->control_port, ev)
  {
    if (ev->body.type == self->uris->midi_Event)
    {
      uint8_t* const data = (uint8_t* const)(ev + 1);
      if ( (data[0] & 0xF0) == 0x90 ) // event & channel
      {
        //lv2_log_note(&self->logger, "Note on : %d, frame %i, event# this nframes %i\n", data[1], int(ev->time.frames), evCounter++ );
        lv2_atom_forge_frame_time(&self->forge, 0);
        LV2_Atom_Forge_Frame set_frame;
        
        // LV2_Atom* set = (LV2_Atom*)
        lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_Play, 0);
        LV2_Atom_Forge_Frame body_frame;
        lv2_atom_forge_blank(&self->forge, &body_frame, self->uris->atom_Blank, 0);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_pad, 0);
        lv2_atom_forge_int(&self->forge, int(data[1]) );
        
        lv2_atom_forge_pop(&self->forge, &body_frame);
        lv2_atom_forge_pop(&self->forge, &set_frame);
        
        // use next available voice for the note
        int n = int(data[1]) - 36;
        int v = int(data[2]);
        
        noteOn( self, n, v, ev->time.frames );
      }
      else if ( (data[0] & 0xF0) == 0x80 )
      {
        //lv2_log_note(&self->logger, "Note off: %d\n", data[1] );
        lv2_atom_forge_frame_time(&self->forge, 0);
        
        LV2_Atom_Forge_Frame set_frame;
        
        //LV2_Atom* set = (LV2_Atom*)
        lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_Stop, 0);
        LV2_Atom_Forge_Frame body_frame;
        lv2_atom_forge_blank(&self->forge, &body_frame, 2, 0);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_pad, 0);
        lv2_atom_forge_int(&self->forge, int(data[1]) );
        
        lv2_atom_forge_pop(&self->forge, &body_frame);
        lv2_atom_forge_pop(&self->forge, &set_frame);
        
        // release ADSR of note
        for(int i = 0; i < NVOICES; i++)
        {
          self->voice[i]->stopIfNoteEquals( int(data[1]) );
        }
        
        
      }
    } // MIDI event
    
    if (ev->body.type == self->uris->atom_Blank)
    {
      const LV2_Atom_Object* obj = (LV2_Atom_Object*)&ev->body;
      
      const LV2_Atom_Object* body = NULL;
      lv2_atom_object_get(obj, self->uris->fabla_Load, &body, 0);
      if (body)
      {
        // Get data from body
        const LV2_Atom_Int* padNum = 0;
        lv2_atom_object_get( body, self->uris->fabla_pad, &padNum, 0);
        int* p = (int*)LV2_ATOM_BODY(padNum);
        int pad = *p;
        
        const LV2_Atom_String* path = 0;
        lv2_atom_object_get( body, self->uris->fabla_filename, &path, 0);
        const char* f = (const char*)LV2_ATOM_BODY(path);
        //lv2_log_note(&self->logger, "fabla_Load recieved %s on pad %i\n", f, pad );
        
        // schedule work
        //SampleMessage message(self->uris->fabla_Load);
        //message.pad = pad;
        
        //size_t s = sizeof(SampleMessage);
        //lv2_log_note(&self->logger, "fabla_Load: scheduling work now, size %i\n", int(s) );
        
        
        Sample* newSamp = 0;
        if ( self->schedule )
        {
          // TODO: use Worker extension
          newSamp = load_sample(self, f);
          //self->schedule->schedule_work(self->schedule->handle, s, &message);
        }
        else
        {
          newSamp = load_sample(self, f);
        }
        
        
        if( self->samples[pad] != 0 )
        {
          //lv2_log_note(&self->logger, "freeing sample with %i frames\n", int(self->samples[pad]->info.frames) );
          if ( self->schedule )
          {
            // TODO: use Worker extension
            free( self->samples[pad]->data );
          }
          else
          {
            free( self->samples[pad]->data );
          }
        }
        
        self->samples[pad] = newSamp;
        //lv2_log_note(&self->logger, "finished loading new sample, writing waveform path to UI!\n" );
        
        
        // send the filename to the UI, it will load the waveform itself
        lv2_atom_forge_frame_time(&self->forge, 0);
        LV2_Atom_Forge_Frame set_frame;
        
        //LV2_Atom* set = (LV2_Atom*)
        lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_Waveform, 0);
        LV2_Atom_Forge_Frame body_frame;
        lv2_atom_forge_blank(&self->forge, &body_frame, 2, 0);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_pad, 0);
        lv2_atom_forge_int(&self->forge, pad);
        
        lv2_atom_forge_property_head(&self->forge, self->uris->fabla_filename, 0);
        lv2_atom_forge_path(&self->forge, f, strlen(f) );
        
        lv2_atom_forge_pop(&self->forge, &body_frame);
        lv2_atom_forge_pop(&self->forge, &set_frame);
        
      }
      
      const LV2_Atom_Object* playBody = NULL;
      lv2_atom_object_get(obj, self->uris->fabla_Play, &playBody, 0);
      if (playBody)
      {
        // extract note from Atom, and play
        //lv2_log_note(&self->logger, "playbody pad \n");
        const LV2_Atom_Int* padNum = 0;
        lv2_atom_object_get( playBody, self->uris->fabla_pad, &padNum, 0);
        if ( padNum )
        {
          int* p = (int*)LV2_ATOM_BODY(padNum);
          int pad = *p;
          noteOn( self, pad, 120, 0 );
        }
      }
      
      // check if UI opened: will send a "fabla_UiRequestPaths" message
      const LV2_Atom_Object* requestPathsBody = NULL;
      lv2_atom_object_get(obj, self->uris->fabla_UiRequestPaths, &requestPathsBody, 0);
      if (requestPathsBody)
      {
        // contains no data: the message itself notifies the UI has been opened,
        // and needs to be told what samples are loaded where.
        self->updateUiPaths = true;
      }
      
      
      
    } // atom Blank
    
  } // LV2_ATOM_SEQUENCE_FOREACH
  
  // triggered by Restore / UI re-instantiate, need to write Atoms from here
  if ( self->updateUiPaths )
  {
    if ( self->samples[self->updateUiPathCounter] )
    {
      //lv2_log_note(&self->logger, "writing Atom %i to UI: %s\n", self->updateUiPathCounter, self->samples[self->updateUiPathCounter]->path);
      // write path to UI so it loads the waveform
      lv2_atom_forge_frame_time(&self->forge, 0);
      LV2_Atom_Forge_Frame set_frame;
      
      lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
      
      lv2_atom_forge_property_head(&self->forge, self->uris->fabla_Waveform, 0);
      LV2_Atom_Forge_Frame body_frame;
      lv2_atom_forge_blank(&self->forge, &body_frame, 1, 0);
      
      lv2_atom_forge_property_head(&self->forge, self->uris->fabla_pad, 0);
      lv2_atom_forge_int(&self->forge, self->updateUiPathCounter);
      
      
      lv2_atom_forge_property_head(&self->forge, self->uris->fabla_filename, 0);
      lv2_atom_forge_path(&self->forge,
                          self->samples[self->updateUiPathCounter]->path,
                          self->samples[self->updateUiPathCounter]->path_len);
      
      lv2_atom_forge_pop(&self->forge, &body_frame);
      lv2_atom_forge_pop(&self->forge, &set_frame);
    }
    
    self->updateUiPathCounter++;
    
    if ( self->updateUiPathCounter > 15 )
    {
      self->updateUiPaths = false;
      self->updateUiPathCounter = 0;
      //lv2_log_note(&self->logger, "finished writing pads to UI\n");
    }
  }
  
  // set the compressor values
  self->comp->setAttack   ( *self->comp_attack );
  self->comp->setRelease  ( *self->comp_decay  );
  self->comp->setThreshold( *self->comp_thres  );
  self->comp->setRatio    ( *self->comp_ratio  );
  self->comp->setMakeup   ( *self->comp_makeup );
  
  
  //printf("%f\t%f\t%f\t%f\n", *self->comp_attack, *self->comp_decay, *self->comp_thres, *self->comp_ratio );
  // makeup TODO
  
  
  for (uint32_t pos = 0; pos < n_samples; pos++)
  {
    float accumL = 0.f;
    float accumR = 0.f;
    
    for(int i = 0; i < NVOICES; i++ )
    {
      self->voice[i]->process( 1, &accumL, &accumR );
    }
    
    accumL = accumL * gain;
    accumR = accumR * gain;
    
    float* buf[2];
    buf[0] = &accumL;
    buf[1] = &accumR;
    
    if ( *self->comp_enable > 0.5 )
    {
      self->comp->process( 1, &buf[0], &buf[0] );   // stereo, in place
    }
    
    self->meter->process( 1, &buf[0], &buf[0] ); // stereo, in place
    
    outputL[pos] = accumL;
    outputR[pos] = accumR;
  }
  
  self->uiUpdateCounter += n_samples;
  
  // disable for Atom debug purposes: stops the huge stream of Atoms
  if ( self->uiUpdateCounter > self->sr / 15 ) // ( false )// 
  {
    // send levels to UI
    float L = self->meter->getLeftDB();
    float R = self->meter->getRightDB();
    
    lv2_atom_forge_frame_time(&self->forge, 0);
    LV2_Atom_Forge_Frame set_frame;
    
    //LV2_Atom* set = (LV2_Atom*)
    lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
    
    lv2_atom_forge_property_head(&self->forge, self->uris->fabla_MeterLevels, 0);
    LV2_Atom_Forge_Frame body_frame;
    lv2_atom_forge_blank(&self->forge, &body_frame, 4, 0);
    
    lv2_atom_forge_property_head(&self->forge, self->uris->fabla_level_l, 0);
    lv2_atom_forge_float(&self->forge, L );
    
    lv2_atom_forge_property_head(&self->forge, self->uris->fabla_level_r, 0);
    lv2_atom_forge_float(&self->forge, R );
    
    lv2_atom_forge_pop(&self->forge, &body_frame);
    lv2_atom_forge_pop(&self->forge, &set_frame);
    
    self-> uiUpdateCounter = 0;
  }
}

static LV2_Worker_Status
work(LV2_Handle                  instance,
     LV2_Worker_Respond_Function respond,
     LV2_Worker_Respond_Handle   handle,
     uint32_t                    size,
     const void*                 data)
{
  printf("Fabla: Work() now\n" );
  FABLA_DSP*  self = (FABLA_DSP*)instance;
  LV2_Atom* atom = (LV2_Atom*)data;
  
  if (atom->type == self->uris->fabla_Unload)
  {
    {
      SampleMessage* msg = (SampleMessage*)data;
      free_sample(self, msg->sample);
    }
  }
  else
  {
    printf("Fabla Work()  LoadSample type message\n" );
    /* Handle set message (load sample). */
    LV2_Atom_Object* obj = (LV2_Atom_Object*)data;
    
    printf("Fabla Work()  LV2_Atom_Object atom type %i, body.otype %i \n", obj->atom.type, obj->body.otype );
    
    /* Get file path from message */
    const LV2_Atom_Int* sampleNum = 0;// FIXME read_set_file_sample_number(&self->uris, obj);
    
    if ( !sampleNum )
    {
      printf("Fabla Work()  LoadSample Sample number not found in Atom\n" );
    }
    else
    {
      //print(self, self->uris->log_Error, "Fabla Work()  LoadSample on sampleNumber %i\n", sampleNum->body );
    }
    
    const LV2_Atom* file_path = 0; //fixme: take from Atom message
    if (!file_path) {
      printf( "Fabla Work()  LoadSample FILE PATH NOT VALID\n" );
      return LV2_WORKER_ERR_UNKNOWN;
    }
    
    //int padNum = sampleNum->body;
    
    /* Load sample. */
    /*
    SampleMessage* sampleMessage;
    if ( file_path )
      sampleMessage = load_sample(self, padNum, (const char*)LV2_ATOM_BODY(file_path) );
    
    if (sampleMessage) {
      // here we send the SampleMessage to the GUI too, so it can show the waveform
      
      
      // Loaded sample, send it to run() to be applied
      respond(handle, sizeof(SampleMessage), &sampleMessage);
    }
    else
    {
      print( "Fabla Work(), sampleMessage not valid, check load_sample code\n" );
    }
    */
  }

  return LV2_WORKER_SUCCESS;
}

static LV2_Worker_Status
work_response(LV2_Handle  instance,
              uint32_t    size,
              const void* data)
{
  FABLA_DSP* self = (FABLA_DSP*)instance;
  
  int sampleNum = 0;
  
  Sample* freeOldSample = 0;
  
  {
    // Get details from the message
    SampleMessage* message =  *(SampleMessage**)data;
    sampleNum = message->pad;
    
    // check if there's currently a sample loaded on the pad
    // this gets used later to see if we need to de-allocate the old sample
    freeOldSample = self->samples[sampleNum];
    
    // point to the new sample
    self->samples[sampleNum] = message->sample;
    
    /*
    // Send a notification that we're using a new sample
    lv2_atom_forge_frame_time(&self->forge, self->frame_offset);
    write_set_file_with_data(&self->forge, &self->uris,
                   sampleNum,
                   self->samples[sampleNum]->path,
                   self->samples[sampleNum]->path_len,
                   message->sample->data,
                   message->sample->info.frames);
    */
  }
  
  if ( freeOldSample )
  {
    /*
    // send worker to free the current sample, 
    SampleMessage msg = { { sizeof(Sample*), self->uris->fabla_Unload },
                          sampleNum,
                          freeOldSample };
    SampleMessage msg( 
    
    self->schedule->schedule_work(self->schedule->handle, sizeof(msg), &msg);
    */
  }
  
  return LV2_WORKER_SUCCESS;
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


static LV2_State_Status
save(LV2_Handle                instance,
     LV2_State_Store_Function  store,
     LV2_State_Handle          handle,
     uint32_t                  flags,
     const LV2_Feature* const* features)
{
  // Analyse new features, check for map path
  LV2_State_Map_Path* map_path = NULL;
  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_STATE__mapPath)) {
      map_path = (LV2_State_Map_Path*)features[i]->data;
    }
  }
  
  if ( !map_path )
  {
    printf("Error: map path not available! SAVE DID NOT COMPLETE!\n" );
    return LV2_STATE_ERR_NO_FEATURE;
  }
  
  FABLA_DSP* self  = (FABLA_DSP*)instance;
  
  // loop over samples, if loaded save its state in the dictionary using
  // the custom URI's created in uris.hxx
  for ( int i = 0; i < 16; i++ )
  {
    if ( self->samples[i] && self->samples[i]->path )
    {
      char* apath = map_path->abstract_path(map_path->handle, self->samples[i]->path);
      
      if ( apath )
      {
        printf("Storing on pad %i, apath %s\n", i, apath );
        store(handle,
            self->uris->padFilename[i],
            apath,
            strlen(self->samples[i]->path) + 1,
            self->uris->atom_Path,
            LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
        
        free(apath);
      }
      else
      {
        printf("apath = null on pad %i\n", i );
      }
    }
  }
  
  return LV2_STATE_SUCCESS;
}

static LV2_State_Status
restore(LV2_Handle                  instance,
        LV2_State_Retrieve_Function retrieve,
        LV2_State_Handle            handle,
        uint32_t                    flags,
        const LV2_Feature* const*   features)
{
  FABLA_DSP* self = (FABLA_DSP*)instance;
  
  size_t   size;
  uint32_t type;
  uint32_t valflags;
  
  for ( int i = 0; i < 16; i++ )
  {
    const void* value = retrieve( handle, self->uris->padFilename[i], &size, &type, &valflags);
    if (value)
    {
      const char* path = (const char*)value;
      if (path)
      {
        //printf( "Restoring pad %i, filepath: %s\n", i, path);
        
        if ( self->samples[i] )
        {
          free_sample(self, self->samples[i] );
        }
        
        Sample* newSample = load_sample(self, path);
        if ( newSample )
        {
          self->samples[i] = newSample;
          
          //printf("Restored sample %s successfully\n", self->samples[i]->path);
        }
        else
        {
          printf("Error: load_sample() return zero on pad %i\n", i);
        }
        
      } // path is valid
      else
      {
        printf( "Error: path of sample not valid from Restore::retrieve()\n");
      }
    }
  }
  
  self->updateUiPaths = true;
  
  return LV2_STATE_SUCCESS;
}

const void*
extension_data(const char* uri)
{
  static const LV2_State_Interface  state  = { save, restore };
  static const LV2_Worker_Interface worker = { work, work_response, NULL };
  
  
  if (!strcmp(uri, LV2_WORKER__interface)) {
    return &worker;
  } else if (!strcmp(uri, LV2_STATE__interface)) {
    return &state;
  }
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

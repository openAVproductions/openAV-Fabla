
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

#define NVOICES 16

static const char* default_sample = "click.wav";

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
  
  float* output_L;
  float* output_R;
  
  float* comp_attack;
  float* comp_decay;
  float* comp_thres;
  float* comp_ratio;
  float* comp_makeup;
  
  PadData padData[16];
  
  // URID map
  LV2_URID_Map* map;
  LV2_URID_Unmap* unmap;
  
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
  Sample* samples[16];
  
  int uiUpdateCounter;
  DBMeter* meterL;
  DBMeter* meterR;
  Compressor* comp;
  
} FABLA_DSP;


static Sample* load_sample(FABLA_DSP* self, const char* path)
{
  const size_t path_len  = strlen(path);
  
  lv2_log_trace(&self->logger, "Loading sample %s\n", path);
  
  Sample* sample = new Sample();
  SF_INFO* const info    = &sample->info;
  SNDFILE* const sndfile = sf_open(path, SFM_READ, info);
  
  if (!sndfile) // || !info->frames ) { // || (info->channels != 1)) {
  {
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
  
  lv2_log_error(&self->logger, "Loaded sample:\n\t %i samples\n\tdata = %i\n\tPath: %s\n", info->frames, sample->data, sample->path);
  
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
  
  for(int i = 0; i < 16; i++ )
    self->samples[i] = 0;
  
  self->sr  = rate;
  self->bpm = 120.0f;
  
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
  
  
  
  // allocate voices
  for(int i = 0; i < NVOICES; i++)
    self->voice[i] = new Voice(rate);
  
  // allocate meters
  self->uiUpdateCounter = 0;
  self->meterL = new DBMeter( rate );
  self->meterR = new DBMeter( rate );
  self->comp   = new Compressor( rate );
  
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
  
  /*
  Sample* newSamp = load_sample(self, sample_path);
  self->samples[0] = newSamp;
  free(sample_path);
  newSamp = load_sample(self, "/root/drums.wav");
  self->samples[1] = newSamp;
  */
  
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
    
    // deal with 16 * ADSR / gain / speed / pan here
    case PAD_GAIN:
    case pg2: case pg3: case pg4: case pg5: case pg6: case pg7: case pg8: case pg9:
    case pg10: case pg11: case pg12: case pg13: case pg14: case pg15: case pg16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_GAIN) ].gain = (float*)data;
        //printf("Gain Pad %i\n", port);
        break;
    
    case PAD_SPEED:
    case pspd2: case pspd3: case pspd4: case pspd5: case pspd6: case pspd7: case pspd8: case pspd9:
    case pspd10: case pspd11: case pspd12: case pspd13: case pspd14: case pspd15: case pspd16:
        // hack the enum to access the right array slice
        self->padData[ port - int(PAD_SPEED) ].speed = (float*)data;
        //printf("Speed: Pad %i\n", port);
        break;
    
    default:
      printf("Error: Attempted connect of non-existing port with ID %u \n", port); break;
  }
}

static void noteOn(FABLA_DSP* self, int note, int velocity)
{
  // clip MIDI input, only play 16 samples
  if ( note > 15) note = 15;
  if ( note <  0) note =  0;
  
  bool alloced = false;
  for(int i = 0; i < NVOICES; i++)
  {
    if ( !self->voice[i]->playing() )
    {
      // set the right sample to the voice
      self->voice[i]->sample = self->samples[note];
      
      // play the voice
      self->voice[i]->play( note, velocity );
      //lv2_log_note(&self->logger, "Voice %i gets note ON %i\n", i, note );
      alloced = true;
      break;
    }
  }
  if ( !alloced )
    lv2_log_note(&self->logger, "Note %i ON: but no voice available\n", note );
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
  
  // x^^4 approximates exponential volume control, * 1.5 to make 75% be 0dB FS
  const float        gain   = pow ( (*self->master), 4.f ) * 1.5;
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
        
        lv2_atom_forge_pop(&self->forge, &body_frame);
        lv2_atom_forge_pop(&self->forge, &set_frame);
        
        // use next available voice for the note
        int n = int(data[1]) - 36;
        int v = int(data[2]);
        
        noteOn( self, n, v );
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
        
        lv2_atom_forge_pop(&self->forge, &body_frame);
        lv2_atom_forge_pop(&self->forge, &set_frame);
        
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
        lv2_log_note(&self->logger, "fabla_Load recieved %s on pad %i\n", f, pad );
        
        // schedule work
        //SampleMessage message(self->uris->fabla_Load);
        //message.pad = pad;
        
        size_t s = sizeof(SampleMessage);
        lv2_log_note(&self->logger, "fabla_Load: scheduling work now, size %i\n", s );
        
        //self->schedule->schedule_work(self->schedule->handle, s, &message);
        
        Sample* newSamp = load_sample(self, f);
        
        if( self->samples[pad] != 0 )
        {
          lv2_log_note(&self->logger, "freeing sample with %i frames\n", self->samples[pad]->info.frames );
          free( self->samples[pad]->data );
        }
        
        self->samples[pad] = newSamp;
        
        lv2_log_note(&self->logger, "finished loading new sample!\n" );
      }
      
      const LV2_Atom_Object* playBody = NULL;
      lv2_atom_object_get(obj, self->uris->fabla_Play, &playBody, 0);
      if (playBody)
      {
        // extract note from Atom, and play
        lv2_log_note(&self->logger, "playbody pad \n");
        const LV2_Atom_Int* padNum = 0;
        lv2_atom_object_get( playBody, self->uris->fabla_pad, &padNum, 0);
        if ( padNum )
        {
          int* p = (int*)LV2_ATOM_BODY(padNum);
          int pad = *p;
          noteOn( self, pad, 120 );
        }
      }
      
      if (obj->body.otype == self->uris->atom_Blank)
      {
        // work schedule here
        lv2_log_note(&self->logger, "atom_Blank ===== recieved\n");
      }
    }
    
    /* TIME STUFF: Currently irrelevant
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
          *
        }
      }
    }
    */
  }
  
  // loop over the pads, setting control port values
  for(int i = 0; i < 16; i++)
  {
    if ( self->samples[i] )
    {
      self->samples[i]->gain  = *(self->padData[i].gain);
      self->samples[i]->speed = *(self->padData[i].speed);
    }
  }
  
  // set the compressor values
  self->comp->setAttack( *self->comp_attack );
  self->comp->setRelease(*self->comp_decay  );
  self->comp->setThreshold(*self->comp_thres);
  self->comp->setRatio ( *self->comp_ratio  );
  
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
    
    self->meterL->process( 1, &buf[0], &buf[0] ); // left  channel, in place
    self->meterR->process( 1, &buf[1], &buf[1] ); // right channel, in place
    
    self->comp->process( 1, &buf[0], &buf[0] );   // stereo, in place
    
    outputL[pos] = accumL;
    outputR[pos] = accumR;
  }
  
  self->uiUpdateCounter += n_samples;
  
  if ( self->uiUpdateCounter > self->sr / 20 )
  {
    // send levels to UI
    float L = self->meterL->getDB();
    float R = self->meterR->getDB();
    
    lv2_atom_forge_frame_time(&self->forge, 0);
    LV2_Atom_Forge_Frame set_frame;
    LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(&self->forge, &set_frame, 1, self->uris->atom_eventTransfer);
    
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



/**
   Do work in a non-realtime thread.

   This is called for every piece of work scheduled in the audio thread using
   self->schedule->schedule_work().  A reply can be sent back to the audio
   thread using the provided respond function.
*/
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
    
    int padNum = sampleNum->body;
    
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

/**
   Handle a response from work() in the audio thread.

   When running normally, this will be called by the host after run().  When
   freewheeling, this will be called immediately at the point the work was
   scheduled.
*/
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
  LV2_State_Map_Path* map_path = NULL;
  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_STATE__mapPath)) {
      map_path = (LV2_State_Map_Path*)features[i]->data;
    }
  }
  
  FABLA_DSP* self  = (FABLA_DSP*)instance;
  
  // loop over samples, if loaded save its state in the dictionary using
  // the custom URI's created in uris.hxx
  for ( int i = 0; i < 16; i++ )
  {
    if ( self->samples[i] )
    {
      //char* apath = map_path->abstract_path(map_path->handle, self->samples[i]->path);
      
      store(handle,
            self->uris->padFilename[i],
            self->samples[i]->path,
            strlen(self->samples[i]->path) + 1,
            self->uris->atom_Path,
            LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
      
      
      printf("Pad %i storing %s\n", i, self->samples[i]->path );
      //free(apath);
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
      //#ifdef DEBUG 
      printf( "Restoring file %s\n", path);
      //#endif
      if ( self->samples[i] )
      {
        free_sample(self, self->samples[i] );
      }
      Sample* newSample = load_sample(self, path);
      if ( newSample )
      {
        self->samples[i] = newSample;
        
        /*
        // Send a notification to UI that we're using a new sample
        lv2_atom_forge_frame_time(&self->forge, 0); // 0 = frame offset
        write_set_file_with_data(&self->forge, &self->uris,
                       i,
                       self->samples[i]->path,
                       self->samples[i]->path_len,
                       message->sample->data,
                       message->sample->info.frames);
        */
        
        printf("Restored sample %s successfully\n", self->samples[i]->path);
      }
      else
      {
        printf( "Error loading sample\n");
      }
    }
  }
  
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

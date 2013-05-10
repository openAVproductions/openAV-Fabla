
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "uris.hxx"

// include faust stuff
#include "../dsp/cpp_ui.h"
#include "../dsp/fabla/fabla.cpp"

#define DEBUG

#ifdef __SSE__
/* On Intel set FZ (Flush to Zero) and DAZ (Denormals Are Zero)
   flags to avoid costly denormals */
#ifdef __SSE3__
#include <pmmintrin.h>
void avoidDenormals()
{
#ifdef DEBUG
  std::cout << "Denormals: FZ DAZ using SSE3" << std::endl;
#endif
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}
#else
#include <xmmintrin.h>
void avoidDenormals()
{
#ifdef DEBUG
  std::cout << "Denormals: FZ" << std::endl;
#endif
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
}
#endif //__SSE3__

#else
void avoidDenormals()
{
  std::cout << "Denormals: Warning! No protection" << std::endl;
}
#endif //__SSE__


/**
   Print an error message to the host log if available, or stderr otherwise.
*/
LV2_LOG_FUNC(3, 4)
static void
print(Fabla* self, LV2_URID type, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  if (self->log) {
    self->log->vprintf(self->log->handle, type, fmt, args);
  } else {
    vfprintf(stderr, fmt, args);
  }
  va_end(args);
}

/**
   An atom-like message used internally to apply/free samples.

   This is only used internally to communicate with the worker, it is never
   sent to the outside world via a port since it is not POD.  It is convenient
   to use an Atom header so actual atoms can be easily sent through the same
   ringbuffer.
*/
typedef struct {
  LV2_Atom atom;
  int      sampleNum;
  Sample*  sample;
} SampleMessage;

/**
   Load a new sample and return it.

   Since this is of course not a real-time safe action, this is called in the
   worker thread only.  The sample is loaded and returned only, plugin state is
   not modified.
*/
static SampleMessage*
load_sample(Fabla* self, int sampleNum, const char* path)
{
  const size_t path_len  = strlen(path);

  print(self, self->uris.log_Error, "Loading sample %s to pad number %i\n", path, sampleNum);

  SampleMessage* sampleMessage  = (SampleMessage*)malloc(sizeof(SampleMessage));
  Sample* const  sample  = (Sample*)malloc(sizeof(Sample));
  SF_INFO* const info    = &sample->info;
  SNDFILE* const sndfile = sf_open(path, SFM_READ, info);
  
  if (!sndfile || !info->frames)
  {
    print(self, self->uris.log_Error, "Failed to open sample '%s'\n", path);
    free(sample);
    return NULL;
  }
  
  
  //print(self, self->uris.log_Error, "Allocating memory for sample   info->frames = %i\n", info->frames);
  
  /* Read data */
  float* data = 0;
  
  // if mono or stereo then load
  if ( info->channels == 1 || info->channels == 2 )
  {
    data = (float*)malloc(sizeof(float) * info->frames);
  }
  
  if (!data)
  {
    print(self, self->uris.log_Error, "Failed to allocate memory for sample.\n");
    return NULL;
  }
  sf_seek(sndfile, 0ul, SEEK_SET);
  sf_read_float(sndfile, data, info->frames);
  sf_close(sndfile);

  /* Fill sample struct and return it. */
  sample->data     = data;
  sample->path     = (char*)malloc(path_len + 1);
  sample->path_len = path_len;
  memcpy(sample->path, path, path_len + 1);
  
  sampleMessage->sampleNum = sampleNum;
  sampleMessage->sample = sample;
  
  return sampleMessage;
}

static void
free_sample(Fabla* self, Sample* sample)
{
  if (sample)
  {
    print(self, self->uris.log_Trace, "Freeing %s\n", sample->path);
    free(sample->path);
    free(sample->data);
    free(sample);
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
  Fabla*  self = (Fabla*)instance;
  LV2_Atom* atom = (LV2_Atom*)data;
  
  //print(self, self->uris.log_Error, "Fabla: Work() now\n" );
  
  if (atom->type == self->uris.eg_freeSample)
  {
    // FIXME: MUTEX
    //g_mutex_lock( &self->sampleMutex );
    {
      //print(self, self->uris.log_Error, "Freeing sample: Mutex locked!\n" );
      // lock mutex, then work with sample, as GUI might be drawing it!
      SampleMessage* msg = (SampleMessage*)data;
      free_sample(self, msg->sample);
    }
    //g_mutex_unlock( &self->sampleMutex );
  }
  else
  {
    //print(self, self->uris.log_Error, "Fabla Work()  LoadSample type message\n" );
    /* Handle set message (load sample). */
    LV2_Atom_Object* obj = (LV2_Atom_Object*)data;
    
    //print(self, self->uris.log_Error, "Fabla Work()  LV2_Atom_Object atom type %i, body.otype %i \n", obj->atom.type, obj->body.otype );
    
    /* Get file path from message */
    const LV2_Atom_Int* sampleNum = read_set_file_sample_number(&self->uris, obj);
    
    if ( !sampleNum )
    {
      print(self, self->uris.log_Error, "Fabla Work()  LoadSample Sample number not found in Atom\n" );
    }
    else
    {
      //print(self, self->uris.log_Error, "Fabla Work()  LoadSample on sampleNumber %i\n", sampleNum->body );
    }
    
    const LV2_Atom* file_path = read_set_file(&self->uris, obj);
    if (!file_path) {
      print(self, self->uris.log_Error, "Fabla Work()  LoadSample FILE PATH NOT VALID\n" );
      return LV2_WORKER_ERR_UNKNOWN;
    }
    
    int padNum = sampleNum->body;
    
    /* Load sample. */
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
      print(self, self->uris.log_Error, "Fabla Work(), sampleMessage not valid, check load_sample code\n" );
    }
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
  Fabla* self = (Fabla*)instance;
  
  int sampleNum = 0;
  
  Sample* freeOldSample = 0;
  
  {
    // Get details from the message
    SampleMessage* message =  *(SampleMessage**)data;
    sampleNum = message->sampleNum;
    
    // check if there's currently a sample loaded on the pad
    // this gets used later to see if we need to de-allocate the old sample
    freeOldSample = self->sample[sampleNum];
    
    // point to the new sample
    self->sample[sampleNum] = message->sample;
    
    // set the "playback" of the current sample past the end by a frame:
    // stops the sample from playing just after being loaded
    self->playback[sampleNum].frame = message->sample->info.frames + 1;
    
    // Send a notification that we're using a new sample
    lv2_atom_forge_frame_time(&self->forge, self->frame_offset);
    write_set_file(&self->forge, &self->uris,
                   sampleNum,
                   self->sample[sampleNum]->path,
                   self->sample[sampleNum]->path_len,
                   message->sample->data,
                   message->sample->info.frames);
  }
  
  if ( freeOldSample )
  {
    // send worker to free the current sample, 
    SampleMessage msg = { { sizeof(Sample*), self->uris.eg_freeSample },
                        sampleNum,
                        freeOldSample };
    
    self->schedule->schedule_work(self->schedule->handle, sizeof(msg), &msg);
  }
  
  return LV2_WORKER_SUCCESS;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
  Fabla* self = (Fabla*)instance;
  switch (port)
  {
    case SAMPLER_CONTROL:
      self->control_port = (LV2_Atom_Sequence*)data;
      break;
    case SAMPLER_RESPONSE:
      self->notify_port = (LV2_Atom_Sequence*)data;
      break;
    case SAMPLER_REVERB_SIZE:
      self->reverb_size = (float*)data;
      self->faust_reverb_size = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-Decay Times in Bands (see tooltips)Mid RT60");
      std::cout << "faust rever size pointer = " << self->faust_reverb_size << "  value = " << *self->faust_reverb_size << endl;
      break;
    case SAMPLER_REVERB_WET:
      self->reverb_wet = (float*)data;
      self->faust_reverb_wet  = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-OutputDry/Wet Mix");
      std::cout << "faust rever wet pointer = " << self->faust_reverb_wet << "  value = " << *self->faust_reverb_wet << endl;
      break;
    case SAMPLER_HIGHPASS:
      self->highpass = (float*)data;
      self->faust_highpass = self->faustUI->getFloatPointer("---fablahipcutoff");
      *self->faust_highpass = 10; 
      break;
    case SAMPLER_LOWPASS:
      self->lowpass = (float*)data;
      self->faust_lowpass  = self->faustUI->getFloatPointer("---fablalopcutoff");
      *self->faust_lowpass = 10000;
      break;
    case SAMPLER_ECHO_FEEDBACK:
      self->echo_feedback = (float*)data;
      self->faust_echo_feedback = self->faustUI->getFloatPointer("---fabla-stereoecho-echo  1000feedback");
      *self->faust_echo_feedback = 0.f;
      break;
    case SAMPLER_ECHO_TIME:
      self->echo_time = (float*)data;
      self->faust_echo_time  = self->faustUI->getFloatPointer("---fabla-stereoecho-echo  1000millisecond");
      break;
    case SAMPLER_OUT_L:
      self->output_port_L = (float*)data;
      break;
    case SAMPLER_OUT_R:
      self->output_port_R = (float*)data;
      break;
    case SAMPLER_MASTER_VOL:
      self->master_vol = (float*)data;
      break;
    default:
      break;
  }
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               path,
            const LV2_Feature* const* features)
{
  /* Allocate and initialise instance structure. */
  Fabla* self = (Fabla*)malloc(sizeof(Fabla));
  if (!self) {
    return NULL;
  }
  memset(self, 0, sizeof(Fabla));
  
  // make sure denormals get nuked to zero
  avoidDenormals();
  
  /* Get host features */
  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map)) {
      self->map = (LV2_URID_Map*)features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_WORKER__schedule)) {
      self->schedule = (LV2_Worker_Schedule*)features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_LOG__log)) {
      self->log = (LV2_Log_Log*)features[i]->data;
    }
  }
  
  bool haveFeatures = true;
  
  if (!self->map) {
    print(self, self->uris.log_Error, "Missing feature urid:map.\n");
    haveFeatures = false;
  } else if (!self->schedule) {
    print(self, self->uris.log_Error, "Missing feature work:schedule.\n");
    haveFeatures = false;
  }
  
  if ( haveFeatures )
  {
    /* Map URIs and initialise forge */
    map_sampler_uris(self->map, &self->uris);
    lv2_atom_forge_init(&self->forge, self->map);
    
    // Set up the Faust DSP units
    char* nullArray[0];
    self->faustUI = new CppUI(0, nullArray);
    self->faustDSP = new FablaDSP();
    self->faustDSP->buildUserInterface(self->faustUI);
    self->faustDSP->init(rate);
    
    // setup defaults for reverb
    float* tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-InputIn Delay");
    *tmp = 0.f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-Decay Times in Bands (see tooltips)LF X");
    *tmp = 400.f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-Decay Times in Bands (see tooltips)Low RT60");
    *tmp = 2.5f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-Decay Times in Bands (see tooltips)Mid RT60");
    *tmp = 4.5f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-Decay Times in Bands (see tooltips)HF Damping");
    *tmp = 6000.f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-RM Peaking Equalizer 1Eq1 Freq");
    *tmp = 315.f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-RM Peaking Equalizer 1Eq1 Level");
    *tmp = 0.f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-RM Peaking Equalizer 2Eq2 Freq");
    *tmp = 630.f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-RM Peaking Equalizer 2Eq2 Level");
    *tmp = 0.f;
    tmp = self->faustUI->getFloatPointer("---fabla-Zita_Rev1-OutputLevel");
    *tmp = 0.f;
    
    return (LV2_Handle)self;
  }
  
  free(self);
  return 0;
}

static void
cleanup(LV2_Handle instance)
{
  Fabla* self = (Fabla*)instance;
  free_sample(self, self->sample[0] );
  free(self);
}

static void
run(LV2_Handle instance,
    uint32_t   sample_count)
{
  Fabla*     self        = (Fabla*)instance;
  FablaURIs* uris        = &self->uris;
  sf_count_t   pos         = 0;
  float*       output_L    = self->output_port_L;
  float*       output_R    = self->output_port_R;
  
  // Set up forge to write directly to notify output port
  const uint32_t notify_capacity = self->notify_port->atom.size;
  lv2_atom_forge_set_buffer(&self->forge,
                            (uint8_t*)self->notify_port,
                            notify_capacity);

  /* Start a sequence in the notify output port. */
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);
  
  /* Read incoming events */
  LV2_ATOM_SEQUENCE_FOREACH(self->control_port, ev)
  {
    //print(self, self->uris.log_Error,"fabla recieved atom\n");
    
    self->frame_offset = ev->time.frames;
    
    if (ev->body.type == uris->midi_Event) // MIDI event on Atom port
    {
      uint8_t* const data = (uint8_t* const)(ev + 1);
      if ( (data[0] & 0xF0) == 0x90 ) // event & channel
      {
        if ( data[1] >= 36 && data[1] < 36 + 16 ) // note bounds
        {
          self->playback[data[1]-36].frame = 0;
          self->playback[data[1]-36].play  = true;
          self->playback[data[1]-36].volume= (data[2] / 127.f);
          
          // update UI that a note has occured
          lv2_atom_forge_frame_time(&self->forge, 0);
          
          //print(self, self->uris.log_Error, "writing PLAY sample %d\n", data[1]-36);
          
          write_play_sample( &self->forge, &self->uris, data[1]-36 );
        }
      }
      else if ( (data[0] & 0xF0) == 0x80 )
      {
        //print(self, self->uris.log_Error, "writing STOP sample %d\n", data[1]-36);
        
        lv2_atom_forge_frame_time(&self->forge, 0);
        write_stop_sample( &self->forge, &self->uris, data[1]-36 );
      }
    }
    else if (is_object_type(uris, ev->body.type) )
    {
      const LV2_Atom_Object* obj = (LV2_Atom_Object*)&ev->body;
      
        /* Received a set message, send it to the worker. */
        //print(self, self->uris.log_Error, "Fabla: \"patch set\" Queuing work now\n" );
        self->schedule->schedule_work(self->schedule->handle,
                                      lv2_atom_total_size(&ev->body),
                                      &ev->body);

        //print(self, self->uris.log_Trace, "Unknown object type %d\n", obj->body.otype);
    }
    else
    {
      //print(self, self->uris.log_Trace, "Unknown event type %d\n", ev->body.type);
    }
  }
  
  // copy the effect port values to FAUST variables
  
  *self->faust_reverb_wet     = ((*self->reverb_wet) * 2)-1;
  *self->faust_reverb_size    = 2 + ((*self->reverb_size) * 4 );
  
  *self->faust_echo_feedback  = 1e-10 + *self->echo_feedback * 90;
  *self->faust_echo_time      = 1e-10 + *self->echo_time * 990;
  
  *self->faust_highpass =  10 + (*self->highpass *  4000);
  *self->faust_lowpass  = 100 + (*self->lowpass  * 12000);
  
  float outL = 1e-10;
  float outR = 1e-10;
  
  // nframes
  for (unsigned int i = 0; i < sample_count; i++)
  {
    float tmp = 1e-10;  // DC offset: float denormals
    
    // pads
    for ( int p = 0; p < 16; p++ )
    {
      // sample loaded && playing
      if ( self->sample[p] && self->playback[p].play ) // check sample loaded
      {
        // add sample
        if ( self->playback[p].frame < self->sample[p]->info.frames )
        {
          tmp +=   self->sample[p]->data[self->playback[p].frame++] // sample value 
                 * self->playback[p].volume;                        // volume
          
          // Envelope, 1000 samples
          if ( self->playback[p].frame < 1000 )
          {
            // Attack
            float a = self->playback[p].frame / 1000.f;
            tmp *= a;
            //std::cout << "attack " << a << endl;
          }
          else if ( self->playback[p].frame + 1000 > self->sample[p]->info.frames )
          {
            // Release
            int rFrame = self->sample[p]->info.frames - self->playback[p].frame;
            float r = rFrame / 1000.f;
            tmp *= r;
            //std::cout << "release  " << r << endl;
          }
        }
        else // stop sample
        {
          self->playback[p].play  = false;
          self->playback[p].frame = 0;
        }
      }

    } // pads
    
    // compute faust units
    float* buf[3];
    
    buf[0] = &tmp;
    buf[1] = &outL;
    buf[2] = &outR;
    
    
    
    // make FAUST process
    self->faustDSP->compute( 1, &buf[0], &buf[1] );
    
    // write output
    output_L[i] = *buf[1] * (*self->master_vol);
    output_R[i] = *buf[2] * (*self->master_vol);
  }
  
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

  Fabla* self  = (Fabla*)instance;
  
  // loop over samples, if loaded save its state in the dictionary using
  // the custom URI's created in uris.hxx
  for ( int i = 0; i < 16; i++ )
  {
    if ( self->sample[i] )
    {
      char* apath = map_path->abstract_path(map_path->handle, self->sample[i]->path);
      store(handle, self->uris.sampleRestorePad[i], apath, strlen(self->sample[i]->path) + 1, self->uris.atom_Path, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
      free(apath);
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
  Fabla* self = (Fabla*)instance;

  size_t   size;
  uint32_t type;
  uint32_t valflags;
  
  for ( int i = 0; i < 16; i++ )
  {
    const void* value = retrieve( handle, self->uris.sampleRestorePad[i], &size, &type, &valflags);
    if (value)
    {
      const char* path = (const char*)value;
      //#ifdef DEBUG 
      print(self, self->uris.log_Error, "Restoring file %s\n", path);
      //#endif
      if ( self->sample[i] )
      {
        free_sample(self, self->sample[i] );
      }
      SampleMessage* message = load_sample(self, i, path);
      if ( message )
      {
        self->sample[i] = message->sample;
        
        // Send a notification to UI that we're using a new sample
        lv2_atom_forge_frame_time(&self->forge, 0); // 0 = frame offset
        write_set_file(&self->forge, &self->uris,
                       i,
                       self->sample[i]->path,
                       self->sample[i]->path_len,
                       message->sample->data,
                       message->sample->info.frames);
      
        print(self, self->uris.log_Error, "Loaded sample %s successfully\n", self->sample[i]->path);
      }
      else
      {
        print(self, self->uris.log_Error, "Error loading sample\n");
      }
    }
  }
  
   return LV2_STATE_SUCCESS;
}

static const void*
extension_data(const char* uri)
{
  static const LV2_State_Interface  state  = { save, restore };
  static const LV2_Worker_Interface worker = { work, work_response, NULL };
  if (!strcmp(uri, LV2_STATE__interface)) {
    return &state;
  } else if (!strcmp(uri, LV2_WORKER__interface)) {
    return &worker;
  }
  return NULL;
}

static const LV2_Descriptor descriptor = {
  FABLA_URI,
  instantiate,
  connect_port,
  NULL,  // activate,
  run,
  NULL,  // deactivate,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &descriptor;
  default:
    return NULL;
  }
}

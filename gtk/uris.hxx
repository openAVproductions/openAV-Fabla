
#ifndef FABLA_URIS_H
#define FABLA_URIS_H

#include <sndfile.h>

#include <sstream>
#include <iostream>

#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define LV2_MIDI__MidiEvent "http://lv2plug.in/ns/ext/midi#MidiEvent"

#define FABLA_URI               "http://www.openavproductions.com/fabla"
#define FABLA_URI__file         FABLA_URI "#file"
#define FABLA_URI__applySample  FABLA_URI "#applySample"
#define FABLA_URI__freeSample   FABLA_URI "#freeSample"
#define FABLA_URI__sampleNumber FABLA_URI "#sampleNumber"
#define FABLA_URI__sampleData   FABLA_URI "#sampleData"

#define FABLA_URI__playSample   FABLA_URI "#playSample"
#define FABLA_URI__stopSample   FABLA_URI "#stopSample"

#define FABLA_URI__sampleRestorePad  FABLA_URI "#sampleRestorePad"

class CppUI;
class FablaDSP;


enum {
  SAMPLER_OUT_L         = 0,
  SAMPLER_OUT_R         = 1,
  
  SAMPLER_CONTROL       = 2,
  SAMPLER_RESPONSE      = 3,
  
  SAMPLER_REVERB_SIZE   = 4,
  SAMPLER_REVERB_WET    = 5,
  
  SAMPLER_MASTER_VOL    = 6,
  
  SAMPLER_HIGHPASS      = 7,
  SAMPLER_LOWPASS       = 8,
  
  SAMPLER_ECHO_TIME     = 9,
  SAMPLER_ECHO_FEEDBACK =10,
};

typedef struct {
	LV2_URID atom_Blank;
	LV2_URID atom_Path;
	LV2_URID atom_Resource;
	LV2_URID atom_Sequence;
	LV2_URID atom_Vector;
	LV2_URID atom_eventTransfer;
	LV2_URID eg_applySample;
	LV2_URID eg_file;
	LV2_URID eg_freeSample;
  LV2_URID eg_sampleNumber;
  LV2_URID eg_sampleData;
	LV2_URID log_Error;
	LV2_URID log_Trace;
	LV2_URID midi_Event;
	LV2_URID patch_Set;
	LV2_URID patch_body;
	LV2_URID playSample;
	LV2_URID stopSample;
  
  LV2_URID sampleRestorePad[16];
} FablaURIs;


typedef struct {
  SF_INFO info;      /**< Info about sample from sndfile */
  float*  data;      /**< Sample data in float */
  char*   path;      /**< Path of file */
  size_t  path_len;  /**< Length of path */
} Sample;

typedef struct {
  size_t len;
  float* sample;
} UiSample;

typedef struct {
  sf_count_t frame;
  bool play;
  float volume;
} SamplePlayback;

typedef struct {
  /* Features */
  LV2_URID_Map*        map;
  LV2_Worker_Schedule* schedule;
  LV2_Log_Log*         log;

  /* Forge for creating atoms */
  LV2_Atom_Forge forge;

  /* Sample */
  Sample* sample[16];

  /* Ports */
  float*               output_port_L;
  float*               output_port_R;
  LV2_Atom_Sequence*   control_port;
  LV2_Atom_Sequence*   notify_port;

  // effect floats
  float* echo_time;
  float* echo_feedback;
  float* faust_echo_time;
  float* faust_echo_feedback;
  
  float* reverb_size;
  float* reverb_wet;
  float* faust_reverb_size;
  float* faust_reverb_wet;
  
  // remove floats
  float* highpass;
  float* lowpass;
  float* faust_highpass;
  float* faust_lowpass;
  
  // master volume
  float* master_vol;
  
  
  /* Forge frame for notify port (for writing worker replies). */
  LV2_Atom_Forge_Frame notify_frame;

  /* URIs */
  FablaURIs uris;

  /* Current position in run() */
  uint32_t frame_offset;

  /* Playback state */
  SamplePlayback playback[16];
  
  // FAUST DSP units
  CppUI*     faustUI;
  FablaDSP*  faustDSP;
  
} Fabla;

static inline void
map_sampler_uris(LV2_URID_Map* map, FablaURIs* uris)
{
	uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
	uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
	uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
	uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
	uris->atom_Vector        = map->map(map->handle, LV2_ATOM__Vector);
	uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
	uris->eg_applySample     = map->map(map->handle, FABLA_URI__applySample);
	uris->eg_file            = map->map(map->handle, FABLA_URI__file);
	uris->eg_freeSample      = map->map(map->handle, FABLA_URI__freeSample);
	uris->eg_sampleNumber    = map->map(map->handle, FABLA_URI__sampleNumber);
	uris->log_Error          = map->map(map->handle, LV2_LOG__Error);
	uris->log_Trace          = map->map(map->handle, LV2_LOG__Trace);
	uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
	uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
	uris->patch_body         = map->map(map->handle, LV2_PATCH__body);
	uris->playSample         = map->map(map->handle, FABLA_URI__playSample);
	uris->stopSample         = map->map(map->handle, FABLA_URI__stopSample);
  
  // Sample restore URI's  per pad 
  for ( int i = 0; i < 16; i++ )
  {
    std::stringstream s;
    s << FABLA_URI__sampleRestorePad << i;
    uris->sampleRestorePad[i] = map->map(map->handle, s.str().c_str() );
  }
}

static inline bool
is_object_type(const FablaURIs* uris, LV2_URID type)
{
	return type == uris->atom_Resource
		|| type == uris->atom_Blank;
}

/**
 * Write a message like the following to @p forge:
 * [
 *     a patch:Set ;
 *     patch:body [
 *         eg-sampler:file </home/me/foo.wav> ;
 *     ] ;
 * ]
 */

static inline LV2_Atom*
write_set_file(LV2_Atom_Forge*    forge,
               const FablaURIs*   uris,
               int                sampleNum,
               const char*        filename,
               const size_t       filename_len)
{
  std::cout << "Writing filename " << filename << " to Atom -> DSP now!" << std::endl;
  
  assert( forge );
  assert( uris );
  assert( filename );
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(
    forge, &set_frame, 1, uris->patch_Set);
  
  lv2_atom_forge_property_head(forge, uris->patch_body, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank(forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head(forge, uris->eg_file, 0);
  lv2_atom_forge_path(forge, filename, filename_len);
  
  lv2_atom_forge_property_head(forge, uris->eg_sampleNumber, 0);
  lv2_atom_forge_int(forge, sampleNum);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  std::cout << "Writing filename " << filename << " to Atom -> DSP  DONE!" << std::endl;
  
  return set;
}

static inline LV2_Atom*
write_set_file_with_data(LV2_Atom_Forge*    forge,
               const FablaURIs*   uris,
               int                sampleNum,
               const char*        filename,
               const size_t       filename_len,
               float*             sampleData,
               const size_t       sampleDataLen)
{
  std::cout << "Writing filename " << filename << " to Atom -> UI now!" << std::endl;
  
  assert( forge );
  assert( uris );
  assert( filename );
  
  //std::cout << "Asserts passed, writing message" << std::endl;
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(
    forge, &set_frame, 1, uris->patch_Set);
  
  lv2_atom_forge_property_head(forge, uris->patch_body, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank(forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head(forge, uris->eg_file, 0);
  lv2_atom_forge_path(forge, filename, filename_len);
  
  lv2_atom_forge_property_head(forge, uris->eg_sampleNumber, 0);
  lv2_atom_forge_int(forge, sampleNum);
  
  /*
  // for writing data from DSP -> UI only
  if ( sampleData && sampleDataLen )
  {
    std::cout << "Writing vector with " << sampleDataLen << " samples " << std::endl;
    
    lv2_atom_forge_property_head(forge, uris->eg_sampleData, 0);
    lv2_atom_forge_vector(forge,
                          sizeof(float), // child size
                          uris->atom_Vector, // child URID
                          sampleDataLen,
                          sampleData );
  }
  */
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  std::cout << "Writing filename " << filename << " to Atom -> UI  DONE!" << std::endl;
  
  return set;
}

static inline LV2_Atom*
write_play_sample(LV2_Atom_Forge* forge,
               const FablaURIs*   uris,
               int                sampleNum )
{
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(forge, &set_frame, 1, uris->playSample);
  
  lv2_atom_forge_property_head(forge, uris->playSample, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank(forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head(forge, uris->eg_sampleNumber, 0);
  lv2_atom_forge_int(forge, sampleNum);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  return set;
}

static inline LV2_Atom*
write_sample_to_ui(LV2_Atom_Forge* forge,
                   const FablaURIs*   uris,
                   int                sampleNum )
{
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(forge, &set_frame, 1, uris->playSample);
  
  lv2_atom_forge_property_head(forge, uris->playSample, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank(forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head(forge, uris->eg_sampleNumber, 0);
  lv2_atom_forge_int(forge, sampleNum);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  return set;
}

static inline LV2_Atom*
write_stop_sample(LV2_Atom_Forge* forge,
               const FablaURIs*   uris,
               int                sampleNum )
{
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(forge, &set_frame, 1, uris->stopSample);
  
  lv2_atom_forge_property_head(forge, uris->stopSample, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank(forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head(forge, uris->eg_sampleNumber, 0);
  lv2_atom_forge_int(forge, sampleNum);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  //fprintf(stderr, "write_stop_sample = %i\n", sampleNum);
  
  return set;
}

/**
 * Get the file path from a message like:
 * [
 *     a patch:Set ;
 *     patch:body [
 *         eg-sampler:file </home/me/foo.wav> ;
 *     ] ;
 * ]
 */
static inline const LV2_Atom*
read_set_file(const FablaURIs*     uris,
              const LV2_Atom_Object* obj)
{
	if (obj->body.otype != uris->patch_Set) {
		fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
		return NULL;
	}

	/* Get body of message. */
	const LV2_Atom_Object* body = NULL;
	lv2_atom_object_get(obj, uris->patch_body, &body, 0);
	if (!body) {
		fprintf(stderr, "Malformed set message has no body.\n");
		return NULL;
	}
	if (!is_object_type(uris, body->atom.type)) {
		fprintf(stderr, "Malformed set message has non-object body.\n");
		return NULL;
	}

	/* Get file path from body. */
	const LV2_Atom* file_path = NULL;
	lv2_atom_object_get(body, uris->eg_file, &file_path, 0);
	if (!file_path) {
		fprintf(stderr, "Ignored set message with no file PATH.\n");
		return NULL;
	}

	return file_path;
}


static inline const LV2_Atom_Int*
read_set_file_sample_number(const FablaURIs*     uris,
              const LV2_Atom_Object* obj)
{
	if (obj->body.otype != uris->patch_Set) {
		fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
		return NULL;
	}

	/* Get body of message. */
	const LV2_Atom_Object* body = NULL;
	lv2_atom_object_get(obj, uris->patch_body, &body, 0);
	if (!body) {
		fprintf(stderr, "Malformed set message has no body.\n");
		return NULL;
	}
	if (!is_object_type(uris, body->atom.type)) {
		fprintf(stderr, "Malformed set message has non-object body.\n");
		return NULL;
	}

	/* Get file path from body. */
	const LV2_Atom_Int* padNum = 0;
	lv2_atom_object_get(body, uris->eg_sampleNumber, &padNum, 0);
  
	return padNum;
}

static inline const UiSample*
read_set_file_sample_data(const FablaURIs*     uris,
              const LV2_Atom_Object* obj)
{
	if (obj->body.otype != uris->patch_Set) {
		fprintf(stderr, "UiSample Ignoring unknown message type %d\n", obj->body.otype);
		return NULL;
	}

	/* Get body of message. */
	const LV2_Atom_Object* body = NULL;
	lv2_atom_object_get(obj, uris->patch_body, &body, 0);
	if (!body) {
		fprintf(stderr, "UiSample Malformed set message has no body.\n");
		return NULL;
	}
	if (!is_object_type(uris, body->atom.type)) {
		fprintf(stderr, "UiSample Malformed set message has non-object body.\n");
		return NULL;
	}
  
  
  
	/* Get file path from body. */
	const LV2_Atom_Vector* vector = 0;
	lv2_atom_object_get(body, uris->eg_sampleData, &vector, 0);
  
  //UiSample* uiSample = (UiSample*)malloc(sizeof(UiSample);
  //uiSample->data = vector->body;
  
	return 0;
}


static inline const LV2_Atom_Int*
read_play_sample(const FablaURIs*     uris,
              const LV2_Atom_Object* obj)
{
	if (obj->body.otype != uris->playSample) {
		fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
		return NULL;
	}

	/* Get body of message. */
	const LV2_Atom_Object* body = NULL;
	lv2_atom_object_get(obj, uris->playSample, &body, 0);
	if (!body) {
		fprintf(stderr, "Malformed set message has no body.\n");
		return NULL;
	}
	if (!is_object_type(uris, body->atom.type)) {
		fprintf(stderr, "Malformed set message has non-object body.\n");
		return NULL;
	}

	/* Get int from body. */
	const LV2_Atom_Int* padNum = 0;
	lv2_atom_object_get(body, uris->eg_sampleNumber, &padNum, 0);

	return padNum;
}


static inline const LV2_Atom_Int*
read_stop_sample(const FablaURIs*     uris,
              const LV2_Atom_Object* obj)
{
	if (obj->body.otype != uris->stopSample) {
		fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
		return NULL;
	}

	/* Get body of message. */
	const LV2_Atom_Object* body = NULL;
	lv2_atom_object_get(obj, uris->stopSample, &body, 0);
	if (!body) {
		fprintf(stderr, "Malformed set message has no body.\n");
		return NULL;
	}
	if (!is_object_type(uris, body->atom.type)) {
		fprintf(stderr, "Malformed set message has non-object body.\n");
		return NULL;
	}

	/* Get int from body. */
	const LV2_Atom_Int* padNum = 0;
	lv2_atom_object_get(body, uris->eg_sampleNumber, &padNum, 0);
  
  fprintf(stderr, "padnum in Uri.hxx read_stop() = %i\n", padNum->body);
  
	return padNum;
}

#endif  /* SAMPLER_URIS_H */

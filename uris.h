
#ifndef FABLA_URIS_H
#define FABLA_URIS_H

#include <sndfile.h>

#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"

#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define LV2_MIDI__MidiEvent "http://lv2plug.in/ns/ext/midi#MidiEvent"

#define FABLA_URI            "http://www.openavproductions.com/fabla"
#define FABLA_URI__file        FABLA_URI "#file"
#define FABLA_URI__applySample FABLA_URI "#applySample"
#define FABLA_URI__freeSample  FABLA_URI "#freeSample"

// GMutex:
// for loading / freeing samples, and GUI drawing. It will *never* block
// the RT thread, don't worry :)
#include <gtk/gtk.h>

typedef struct {
	LV2_URID atom_Blank;
	LV2_URID atom_Path;
	LV2_URID atom_Resource;
	LV2_URID atom_Sequence;
	LV2_URID atom_eventTransfer;
	LV2_URID eg_applySample;
	LV2_URID eg_file;
	LV2_URID eg_freeSample;
	LV2_URID log_Error;
	LV2_URID log_Trace;
	LV2_URID midi_Event;
	LV2_URID patch_Set;
	LV2_URID patch_body;
} FablaURIs;


typedef struct {
  SF_INFO info;      /**< Info about sample from sndfile */
  float*  data;      /**< Sample data in float */
  char*   path;      /**< Path of file */
  size_t  path_len;  /**< Length of path */
} Sample;

typedef struct {
  sf_count_t frame;
  bool play;
  
} SamplePlayback;

typedef struct {
  /* Features */
  LV2_URID_Map*        map;
  LV2_Worker_Schedule* schedule;
  LV2_Log_Log*         log;

  /* Forge for creating atoms */
  LV2_Atom_Forge forge;

  /* Sample */
  GMutex sampleMutex; // never gets locked in the RT thread
  Sample* sample[16];

  /* Ports */
  float*               output_port;
  LV2_Atom_Sequence*   control_port;
  LV2_Atom_Sequence*   notify_port;

  /* Forge frame for notify port (for writing worker replies). */
  LV2_Atom_Forge_Frame notify_frame;

  /* URIs */
  FablaURIs uris;

  /* Current position in run() */
  uint32_t frame_offset;

  /* Playback state */
  sf_count_t frame;
  bool       play;
} Fabla;

static inline void
map_sampler_uris(LV2_URID_Map* map, FablaURIs* uris)
{
	uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
	uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
	uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
	uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
	uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
	uris->eg_applySample     = map->map(map->handle, FABLA_URI__applySample);
	uris->eg_file            = map->map(map->handle, FABLA_URI__file);
	uris->eg_freeSample      = map->map(map->handle, FABLA_URI__freeSample);
	uris->log_Error          = map->map(map->handle, LV2_LOG__Error);
	uris->log_Trace          = map->map(map->handle, LV2_LOG__Trace);
	uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
	uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
	uris->patch_body         = map->map(map->handle, LV2_PATCH__body);
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
               const FablaURIs* uris,
               const char*        filename,
               const size_t       filename_len)
{
	LV2_Atom_Forge_Frame set_frame;
	LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(
		forge, &set_frame, 1, uris->patch_Set);

	lv2_atom_forge_property_head(forge, uris->patch_body, 0);
	LV2_Atom_Forge_Frame body_frame;
	lv2_atom_forge_blank(forge, &body_frame, 2, 0);

	lv2_atom_forge_property_head(forge, uris->eg_file, 0);
	lv2_atom_forge_path(forge, filename, filename_len);

	lv2_atom_forge_pop(forge, &body_frame);
	lv2_atom_forge_pop(forge, &set_frame);

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

#endif  /* SAMPLER_URIS_H */

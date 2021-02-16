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


#include <string>
#include <stdlib.h>

#include "../dsp/ports.h"
#include "../dsp/shared.h"

#include "fabla.hxx"

#include "lv2/lv2plug.in/ns/ext/atom/forge.h"

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"

void initForge(Fabla* self)
{
  // then init the forge to write Atoms
  self->voidForge = malloc( sizeof(LV2_Atom_Forge) );
  lv2_atom_forge_init( (LV2_Atom_Forge*)self->voidForge, self->map);
}


void writeLoadSample(Fabla* self, int pad, const char* filename, size_t filename_len)
{
  LV2_Atom_Forge* forge = (LV2_Atom_Forge*)self->voidForge;
  
  // To write messages, we set up a buffer:
  uint8_t obj_buf[1024];
  // Then we tell the forge to use that buffer
  lv2_atom_forge_set_buffer( forge, obj_buf, 1024);
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank( forge, &set_frame, 1, self->uris->atom_eventTransfer);

  lv2_atom_forge_property_head( forge, self->uris->fabla_Load, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank( forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head( forge, self->uris->fabla_pad, 0);
  lv2_atom_forge_int(forge, pad);
  
  lv2_atom_forge_property_head( forge, self->uris->fabla_filename, 0);
  lv2_atom_forge_path(forge, filename, filename_len);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  //printf("writing message now, filename: %s, size %i\n", filename, lv2_atom_total_size(set) );
  
  self->write_function(self->controller, ATOM_IN, lv2_atom_total_size(set),
              self->uris->atom_eventTransfer,
              set );
}

void writePadPlay(Fabla* self, int pad)
{
  LV2_Atom_Forge* forge = (LV2_Atom_Forge*)self->voidForge;
  
  // To write messages, we set up a buffer:
  uint8_t obj_buf[1024];
  // Then we tell the forge to use that buffer
  lv2_atom_forge_set_buffer( forge, obj_buf, 1024);
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank( forge, &set_frame, 1, self->uris->atom_eventTransfer);

  lv2_atom_forge_property_head( forge, self->uris->fabla_Play, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank( forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head( forge, self->uris->fabla_pad, 0);
  lv2_atom_forge_int(forge, pad);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  //printf("writing pad play\n" );
  
  self->write_function(self->controller, ATOM_IN, lv2_atom_total_size(set),
              self->uris->atom_eventTransfer,
              set );
}

/// Called when the UI is instantiated. DSP writes the path/filename for each
/// pad to the UI, so it can open the file, read the waveform & display it.
void writeUpdateUiPaths(Fabla* self)
{
  LV2_Atom_Forge* forge = (LV2_Atom_Forge*)self->voidForge;
  
  uint8_t obj_buf[1024];
  lv2_atom_forge_set_buffer( forge, obj_buf, 1024);
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank( forge, &set_frame, 1, self->uris->atom_eventTransfer);

  lv2_atom_forge_property_head( forge, self->uris->fabla_UiRequestPaths, 0);
  lv2_atom_forge_bool( forge, true);
  lv2_atom_forge_pop(forge, &set_frame);
  
  self->write_function(self->controller, ATOM_IN, lv2_atom_total_size(set),
              self->uris->atom_eventTransfer,
              set );
}

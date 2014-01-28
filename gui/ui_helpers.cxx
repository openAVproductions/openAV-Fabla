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

#include <FL/Fl_Native_File_Chooser.H>

#include "fabla.h"

/// Setup the forge
void initForge(Fabla* self)
{
  // then init the forge to write Atoms
  self->voidForge = malloc( sizeof(LV2_Atom_Forge) );
  lv2_atom_forge_init( (LV2_Atom_Forge*)self->voidForge, self->map);
}

/// Tells the DSP to load a sample of path filename, to pad number pad
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

void writeUnloadSample( Fabla* self, int pad )
{
  LV2_Atom_Forge* forge = (LV2_Atom_Forge*)self->voidForge;
  
  uint8_t obj_buf[1024];
  lv2_atom_forge_set_buffer( forge, obj_buf, 1024);
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank( forge, &set_frame, 1, self->uris->atom_eventTransfer);

  lv2_atom_forge_property_head( forge, self->uris->fabla_Unload, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank( forge, &body_frame, 2, 0);
  
  lv2_atom_forge_property_head( forge, self->uris->fabla_pad, 0);
  lv2_atom_forge_int(forge, pad);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  self->write_function(self->controller, ATOM_IN, lv2_atom_total_size(set),
            self->uris->atom_eventTransfer,
            set );
  
  // Update UI state, resetting the pad's state
  self->widget->padData[pad].init();
  self->widget->waveform->redraw();
}

/// Tells the DSP that it should play a certain pad
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

void padClicked( Fabla* self, int pad, int rclick )
{
  //printf("pad %i clicked, right = %i\n", id, rclick);

  if ( rclick )
  {
    // pop up "right click" menu, for load / remove action
    Fl_Menu_Item rclick_menu[] =
    {
      { "Load" },
      { "Remove  "},
      { 0 }
    };
    Fl_Menu_Item *m = (Fl_Menu_Item*) rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0);
    if ( !m )
    {
        return;
    }
    else if ( strcmp(m->label(), "Load") == 0 )
    {
      /// pop up "load sample" dialog, write Atom event to DSP
      Fl_Native_File_Chooser fnfc;
      fnfc.title("Load Sample");
      fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
      fnfc.filter("Wav\t*.wav");
      fnfc.directory( getenv("HOME") );
      
      switch ( fnfc.show() )
      {
        case -1: printf("ERROR: %s\\n", fnfc.errmsg());    break;  // ERROR
        case  1: printf("CANCEL\\n");                      break;  // CANCEL
        default: printf("Loading directory: %s\n", fnfc.filename());
        writeLoadSample( self, pad, fnfc.filename(), strlen(fnfc.filename()));
        break;
      }
    }
    else if ( strcmp(m->label(), "Remove  ") == 0 )
    {
      /// write Unload action to this pad
      writeUnloadSample( self, pad );
    }
  }
}

/// Called when the UI is instantiated. DSP writes the path/filename for each
/// pad to the UI, so it can open the file, read the waveform & display it.
void writeUpdateUiPaths(Fabla* self)
{
  /*
  LV2_Atom_Forge* forge = (LV2_Atom_Forge*)self->voidForge;
  
  uint8_t obj_buf[1024];
  lv2_atom_forge_set_buffer( forge, obj_buf, 1024);
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank( forge, &set_frame, 1, self->uris->atom_eventTransfer);

  lv2_atom_forge_property_head( forge, self->uris->fabla_UiRequestPaths, 0);
  lv2_atom_forge_pop(forge, &set_frame);
  
  self->write_function(self->controller, ATOM_IN, lv2_atom_total_size(set),
              self->uris->atom_eventTransfer,
              set );
  */
}


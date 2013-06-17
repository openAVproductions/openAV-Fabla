

#include <string>


#include "../dsp/ports.h"
#include "../dsp/shared.h"

#include "fabla.hxx"

#include "lv2/lv2plug.in/ns/ext/atom/forge.h"

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"


static void writeLoadSample(Fabla* self, int pad, const char* filename, size_t filename_len) 
{
  
  LV2_Atom_Forge* forge = (LV2_Atom_Forge*)self->voidForge;
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank( forge, &set_frame, 1, self->uris->atom_eventTransfer);

  lv2_atom_forge_property_head( forge, self->uris->fabla_Load, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank( forge, &body_frame, 4, 0);
  
  lv2_atom_forge_property_head( forge, self->uris->fabla_pad, 0);
  lv2_atom_forge_int(forge, pad);
  
  lv2_atom_forge_property_head( forge, self->uris->fabla_pad, 0);
  lv2_atom_forge_path(forge, filename, filename_len);
  
  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
}

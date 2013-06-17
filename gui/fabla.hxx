
#ifndef FABLA_HEADER
#define FABLA_HEADER

#include "../dsp/ports.h"
#include "../dsp/shared.h"

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"


class FablaUI;
//class LV2_Atom_Forge;

typedef struct {
  FablaUI* widget;
  
  // URID map
  LV2_URID_Map* map;
  LV2_URID_Unmap* unmap;
  Fabla_URIs* uris;
  
  void* voidForge;
  
  LV2UI_Write_Function write_function;
  LV2UI_Controller     controller;
  
} Fabla;

#endif // FABLA_HEADER

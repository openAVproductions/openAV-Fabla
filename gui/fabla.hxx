
#ifndef FABLA_HEADER
#define FABLA_HEADER

#include "../dsp/ports.h"
#include "../dsp/shared.h"

class FablaUI;

typedef struct {
  FablaUI* widget;
  
  // URID map
  LV2_URID_Map* map;
  LV2_URID_Unmap* unmap;
  Fabla_URIs* uris;
  
  LV2UI_Write_Function write_function;
  LV2UI_Controller     controller;
  
} Fabla;

#endif // FABLA_HEADER

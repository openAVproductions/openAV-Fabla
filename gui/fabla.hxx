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

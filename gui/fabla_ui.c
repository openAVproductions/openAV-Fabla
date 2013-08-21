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
#include <iostream>

// X window embedding
#include <FL/x.H>

// include the URI and global data of this plugin
#include "../dsp/ports.h"
#include "../dsp/shared.h"

// this is our custom widget include
#include "fabla.h"

// fabla struct
#include "fabla.hxx"

// core spec include
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

// GUI
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "lv2/lv2plug.in/ns/ext/atom/util.h"
//#include "lv2/lv2plug.in/ns/ext/atom/forge.h"

#include <sndfile.h>

using namespace std;

extern void initForge(Fabla*);
extern void writeLoadSample(Fabla* self, int pad, const char* filename, size_t filename_len);

static LV2UI_Handle instantiate(const struct _LV2UI_Descriptor * descriptor,
                const char * plugin_uri,
                const char * bundle_path,
                LV2UI_Write_Function write_function,
                LV2UI_Controller controller,
                LV2UI_Widget * widget,
                const LV2_Feature * const * features) {

    if (strcmp(plugin_uri, FABLA_URI) != 0) {
        fprintf(stderr, "Fabla_UI_URI error: this GUI does not support plugin with URI %s\n", plugin_uri);
        return NULL;
    }
    
    void* parentXwindow = 0;
    
    LV2UI_Resize* resize = NULL;
    
    Fabla* self = (Fabla*)malloc(sizeof(Fabla));
    if (self == NULL) return NULL;
    memset(self, 0, sizeof(Fabla));
    
    self->uris  = (Fabla_URIs*)malloc(sizeof(Fabla_URIs));
    memset(self->uris, 0, sizeof(Fabla_URIs));
    
    
    for (int i = 0; features[i]; ++i) {
      //cout << "FablaUI got feature " << features[i]->URI << endl;
      if (!strcmp(features[i]->URI, LV2_UI__parent)) {
        parentXwindow = features[i]->data;
        //cout << "got parent UI feature: X11 id = " << (Window)parentXwindow << endl;
      } else if (!strcmp(features[i]->URI, LV2_UI__resize)) {
      
        resize = (LV2UI_Resize*)features[i]->data;
      
      }
      else if (!strcmp(features[i]->URI, LV2_URID__map)) {
        self->map = (LV2_URID_Map*)features[i]->data;
      }
      else if (!strcmp(features[i]->URI, LV2_URID__unmap)) {
        self->unmap = (LV2_URID_Unmap*)features[i]->data;
      }
    }
    
    self->write_function = write_function;
    self->controller = controller;
    
    map_uris( self->map, self->uris );
    
    initForge( self );
    
    // Create UI, and set controller / write func
    self->widget = new FablaUI( parentXwindow, self );
    self->widget->controller = controller;
    self->widget->writeFunction = write_function;
    
    if (resize) {
      cout << "FablaUI: resizing now, resize = " << resize << "  resize->handle " << resize->handle << "  resize->ui_resize = " << resize->ui_resize << endl;
      resize->ui_resize(resize->handle, self->widget->w->w(),self->widget->w->h());
    }
    else
    {
      cout << "FablaUI: Warning, host doesn't support resize extension.\n\
      Please ask the developers of the host to support this extension. "<< endl;
    }
    
    //cout << "returning..." << int(self->widget->getXID()) << endl;
    
    return (LV2UI_Handle)self;
}

static void cleanup(LV2UI_Handle ui) {
    //printf("FablaUI: cleanup()\n");
    Fabla *pluginGui = (Fabla *) ui;
    delete pluginGui->widget;
    free( pluginGui );
}

static void port_event(LV2UI_Handle handle,
               uint32_t port_index,
               uint32_t buffer_size,
               uint32_t format,
               const void * buffer)
{
    Fabla *self = (Fabla *) handle;
    
    FablaUI* ui = (FablaUI*)self->widget;
    float v = *((float*)buffer);
    
    //cout << "Port event on index " << port_index << "  Format is " << format << endl;
    
    Fl::lock();
    
    /*
    LV2_Atom* atom = 0;
    LV2_Atom_Object* obj = 0;
    LV2_Atom_Int* pad = 0;
    */
    
    switch ( port_index )
    {
      case MASTER_VOL: ui->masterVol->value( v );     break;
      
      case COMP_ATTACK: ui->compAttack->value(v);     break;
      case COMP_DECAY:  ui->compRelease->value(v);    break;
      case COMP_RATIO:  ui->compRatio->value(v);
                        ui->compressor->ratio(v);     break;
      case COMP_THRES:  ui->compThres->value(v);
                        ui->compressor->threshold(v); break;
      case COMP_MAKEUP: ui->compressor->makeup(v);    break;
      
      case COMP_ENABLE: ui->compressor->set_active(v);break;
      
      
      
      
      case ATOM_OUT:
          if (format != self->uris->atom_eventTransfer) {
            printf("FablaUI: format != atom_eventTransfer\n");
            return;
          }
          
          {
            LV2_Atom* atom = (LV2_Atom*)buffer;
            if (atom->type != self->uris->atom_Blank) {
              printf("FablaUI: atom->type != atom_Blank\n");
              return;
            }
            
            // Get body
            LV2_Atom_Object* obj = (LV2_Atom_Object*)atom;
            const LV2_Atom_Object* body = NULL;
            
            
            // NOTE ON
            lv2_atom_object_get(obj, self->uris->fabla_Play, &body, 0);
            if (body) {
              // Get int from body
              const LV2_Atom_Int* padNum = 0;
              lv2_atom_object_get( body, self->uris->fabla_pad, &padNum, 0);
              int* p = (int*)LV2_ATOM_BODY(padNum);
              int pad = *p - 36;
              
              if ( pad >= 0 && pad < 16 )
              {
                //printf("pad on %i\n", pad );
                switch ( pad )
                {
                  case 0:  ui->p1->play(true);  break;
                  case 1:  ui->p2->play(true);  break;
                  case 2:  ui->p3->play(true);  break;
                  case 3:  ui->p4->play(true);  break;
                  case 4:  ui->p5->play(true);  break;
                  case 5:  ui->p6->play(true);  break;
                  case 6:  ui->p7->play(true);  break;
                  case 7:  ui->p8->play(true);  break;
                  case 8:  ui->p9->play(true);  break;
                  case 9:  ui->p10->play(true); break;
                  case 10: ui->p11->play(true); break;
                  case 11: ui->p12->play(true); break;
                  case 12: ui->p13->play(true); break;
                  case 13: ui->p14->play(true); break;
                  case 14: ui->p15->play(true); break;
                  case 15: ui->p16->play(true); break;
                  default: break;
                }
                
                // set the "selectedPad" to the played note
                ui->select_pad(pad);
              }
            }
            
            // NOTE Off
            body = NULL;
            lv2_atom_object_get(obj, self->uris->fabla_Stop, &body, 0);
            if (body) {
              // Get int from body
              const LV2_Atom_Int* padNum = 0;
              lv2_atom_object_get( body, self->uris->fabla_pad, &padNum, 0);
              int* p = (int*)LV2_ATOM_BODY(padNum);
              int pad = *p - 36;
              
              if ( pad >= 0 && pad < 16 )
              {
                //printf("pad off %i\n", pad );
                switch ( pad )
                {
                  case 0:  ui->p1->play(false);  break;
                  case 1:  ui->p2->play(false);  break;
                  case 2:  ui->p3->play(false);  break;
                  case 3:  ui->p4->play(false);  break;
                  case 4:  ui->p5->play(false);  break;
                  case 5:  ui->p6->play(false);  break;
                  case 6:  ui->p7->play(false);  break;
                  case 7:  ui->p8->play(false);  break;
                  case 8:  ui->p9->play(false);  break;
                  case 9:  ui->p10->play(false); break;
                  case 10: ui->p11->play(false); break;
                  case 11: ui->p12->play(false); break;
                  case 12: ui->p13->play(false); break;
                  case 13: ui->p14->play(false); break;
                  case 14: ui->p15->play(false); break;
                  case 15: ui->p16->play(false); break;
                  default: break;
                }
              }
            }
            
            // Meter Levels
            body = NULL;
            lv2_atom_object_get(obj, self->uris->fabla_MeterLevels, &body, 0);
            if (body)
            {
              const LV2_Atom_Float* L = 0;
              lv2_atom_object_get( body, self->uris->fabla_level_l, &L, 0);
              float levelL = *(float*)LV2_ATOM_BODY(L);
              
              const LV2_Atom_Float* R = 0;
              lv2_atom_object_get( body, self->uris->fabla_level_r, &R, 0);
              float levelR = *(float*)LV2_ATOM_BODY(R);
              
              
              // range scale, so 75% of the way up is 0dB FS
              float zeroOneL = (1-(levelL / -96.f));
              float zeroOneR = (1-(levelR / -96.f));
              
              float finalL = pow(zeroOneL, 4);
              float finalR = pow(zeroOneR, 4);
              
              //printf("levelL = %f\n final %f\n", levelL, final );
              
              ui->masterVol->amplitude( finalL, finalR );
            }
            
            
            // Waveform Data
            body = NULL;
            lv2_atom_object_get(obj, self->uris->fabla_Waveform, &body, 0);
            if (body)
            {
              const LV2_Atom_Int* padNum = 0;
              lv2_atom_object_get( body, self->uris->fabla_pad, &padNum, 0);
              int pad = -1;
              if ( padNum )
                pad = *(int*)LV2_ATOM_BODY(padNum);
              
              const LV2_Atom_String* path = 0;
              lv2_atom_object_get( body, self->uris->fabla_filename, &path, 0);
              const char* f = 0;
              if ( path )
                f = (const char*)LV2_ATOM_BODY(path);
              
              if ( f )
                //printf( "FablaUI:  recieved waveform, path: %s\n", f );
              
              if ( pad == -1 || f == 0 )
              {
                printf( "FablaUI:  Error, waveform data message malformed\n" );
                return;
              }
              
              //printf("FablaUI:  recieved waveform data on pad %i, path %s\nLoading sample now...\n", pad , f);
              
              SF_INFO info;
              SNDFILE* const sndfile = sf_open( f, SFM_READ, &info);
              
              if (!sndfile) // || !info.frames ) { // || (info.channels != 1)) {
              {
                printf( "FablaUI:  Failed to open sample '%s'\n", f);
                return;
              }
              
              // Read data
              float* const data = (float*)malloc(sizeof(float) * info.frames);
              if (!data) {
                printf("FablaUI: Failed to allocate memory for sample\n");
                return;
              }
              sf_seek(sndfile, 0ul, SEEK_SET);
              sf_read_float(sndfile, data, info.frames);
              
              
              // find how many samples per pixel
              int samplesPerPix = info.frames / UI_WAVEFORM_PIXELS;
              
              // loop over each pixel value we need
              for( int p = 0; p < UI_WAVEFORM_PIXELS; p++ )
              {
                float average = 0.f;
                
                // calc value for this pixel
                for( int i = 0; i < samplesPerPix; i++ )
                {
                  float tmp = data[i + (p * samplesPerPix)];
                  if ( tmp < 0 ) { tmp = -tmp; }
                  average += tmp;
                }
                average = (average / samplesPerPix);
                ui->padData[pad].waveform[p] = average;
              }
              
              
              // only display "name" of file, not path
              std::string name = f;
              int i = name.find_last_of('/') + 1;
              std::string sub = name.substr( i );
              ui->padData[pad].name = sub;
              //printf("FablaUI: name %s\ni %i\nsub %s\n", name.c_str(), i, sub.c_str() );
              
              ui->padData[pad].loaded = true;
              ui->padData[pad].waveformLength = info.frames;
              
              if(ui->selectedPad == pad )
              {
                ui->waveform->setData( UI_WAVEFORM_PIXELS, info.frames, &ui->padData[pad].waveform[0], sub );
              }
              
              ui->waveform->redraw();
              
              free(data);
              sf_close(sndfile);
              
              // set UI pad loaded
              switch ( pad )
              {
                case 0:  ui->p1->loaded(true);  break;
                case 1:  ui->p2->loaded(true);  break;
                case 2:  ui->p3->loaded(true);  break;
                case 3:  ui->p4->loaded(true);  break;
                case 4:  ui->p5->loaded(true);  break;
                case 5:  ui->p6->loaded(true);  break;
                case 6:  ui->p7->loaded(true);  break;
                case 7:  ui->p8->loaded(true);  break;
                case 8:  ui->p9->loaded(true);  break;
                case 9:  ui->p10->loaded(true); break;
                case 10: ui->p11->loaded(true); break;
                case 11: ui->p12->loaded(true); break;
                case 12: ui->p13->loaded(true); break;
                case 13: ui->p14->loaded(true); break;
                case 14: ui->p15->loaded(true); break;
                case 15: ui->p16->loaded(true); break;
                default: break;
              }
            }
          }
        break;
      
      
      // handle all PAD ports here:
      case PAD_GAIN:
      case pg2: case pg3: case pg4: case pg5: case pg6: case pg7: case pg8: case pg9:
      case pg10: case pg11: case pg12: case pg13: case pg14: case pg15: case pg16:
          // hack the enum to access the right array slice
          //printf("Gain Pad %i, pad# %i\n", port_index, port_index - int(PAD_GAIN) );
          ui->padData[ port_index-int(PAD_GAIN) ].gain = *(float*)buffer;
          if ( int(port_index - PAD_GAIN) == ui->selectedPad )
            ui->gain->value( *(float*)buffer );
          break;
      case PAD_SPEED:
      case pspd2: case pspd3: case pspd4: case pspd5: case pspd6: case pspd7: case pspd8: case pspd9:
      case pspd10: case pspd11: case pspd12: case pspd13: case pspd14: case pspd15: case pspd16:
          ui->padData[ port_index-int(PAD_SPEED) ].speed = *(float*)buffer;
          if ( int(port_index - PAD_SPEED) == int(ui->selectedPad) )
            ui->speed->value( *(float*)buffer );
          break;
      
      
      case PAD_PAN:
      case pp2: case pp3: case pp4: case pp5: case pp6: case pp7: case pp8: case pp9:
      case pp10: case pp11: case pp12: case pp13: case pp14: case pp15: case pp16:
          ui->padData[ port_index - int(PAD_PAN) ].pan = *(float*)buffer;
          if ( int(port_index - PAD_PAN) == ui->selectedPad )
            ui->pan->value( *(float*)buffer );
          break;
      
      // ADSR
      case PAD_ATTACK:
      case pa2: case pa3: case pa4: case pa5: case pa6: case pa7: case pa8: case pa9:
      case pa10: case pa11: case pa12: case pa13: case pa14: case pa15: case pa16:
          ui->padData[ port_index - int(PAD_ATTACK) ].a = *(float*)buffer;
          if ( int(port_index - PAD_ATTACK) == ui->selectedPad )
            ui->a->value( *(float*)buffer );
          break;
      
      case PAD_DECAY:
      case pd2: case pd3: case pd4: case pd5: case pd6: case pd7: case pd8: case pd9:
      case pd10: case pd11: case pd12: case pd13: case pd14: case pd15: case pd16:
          ui->padData[ port_index - int(PAD_DECAY) ].d = *(float*)buffer;
          if ( int(port_index - PAD_DECAY) == ui->selectedPad )
            ui->d->value( *(float*)buffer );
          break;
      
      case PAD_SUSTAIN:
      case ps2: case ps3: case ps4: case ps5: case ps6: case ps7: case ps8: case ps9:
      case ps10: case ps11: case ps12: case ps13: case ps14: case ps15: case ps16:
          ui->padData[ port_index - int(PAD_SUSTAIN) ].s = *(float*)buffer;
          if ( int(port_index - PAD_SUSTAIN) == ui->selectedPad )
            ui->s->value( *(float*)buffer );
          break;
      
      case PAD_RELEASE:
      case pr2: case pr3: case pr4: case pr5: case pr6: case pr7: case pr8: case pr9:
      case pr10: case pr11: case pr12: case pr13: case pr14: case pr15: case pr16:
          ui->padData[ port_index - int(PAD_RELEASE) ].r = *(float*)buffer;
          if ( int(port_index - PAD_RELEASE) == ui->selectedPad )
            ui->r->value( *(float*)buffer );
          break;
      
      default: break;
    }
    
    //ui->filterLowpass->value( argv[0]->f );
    Fl::unlock();
    Fl::awake();
    
    return;
}


static int idle(LV2UI_Handle handle)
{
  Fabla* self = (Fabla*)handle;
  self->widget->idle();
  
  return 0;
}

static const LV2UI_Idle_Interface idle_iface = { idle };

static const void*
extension_data(const char* uri)
{
  //cout << "UI extension data!" << endl;
  if (!strcmp(uri, LV2_UI__idleInterface)) {
    //cout << "giving host idle interface!" << endl;
    return &idle_iface;
  }
  return NULL;
}

static LV2UI_Descriptor descriptors[] = {
    {FABLA_UI_URI, instantiate, cleanup, port_event, extension_data}
};

const LV2UI_Descriptor * lv2ui_descriptor(uint32_t index) {
    //printf("lv2ui_descriptor(%u) called\n", (unsigned int)index); 
    if (index >= sizeof(descriptors) / sizeof(descriptors[0])) {
        return NULL;
    }
    return descriptors + index;
}

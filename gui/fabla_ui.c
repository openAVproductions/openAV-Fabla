
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
      //cout << "feature " << features[i]->URI << endl;
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
      //resize->ui_resize(resize->handle, self->widget->getWidth(),self->widget->getHeight());
      resize->ui_resize(resize->handle, self->widget->w->w(),self->widget->w->h());
    }
    
    //cout << "returning..." << int(self->widget->getXID()) << endl;
    
    return (LV2UI_Handle)self;
}

static void cleanup(LV2UI_Handle ui) {
    //printf("cleanup()\n");
    Fabla *pluginGui = (Fabla *) ui;
    delete pluginGui->widget;
    free( pluginGui);
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
      case MASTER_VOL: ui->masterVol->value( v );         break;
      
      case ATOM_OUT:
          if (format != self->uris->atom_eventTransfer) {
            printf("format != atom_eventTransfer\n");
            return;
          }
          
          {
            LV2_Atom* atom = (LV2_Atom*)buffer;
            if (atom->type != self->uris->atom_Blank) {
              printf("atom->type != atom_Blank\n");
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
            
          }
        break;
      
      
      // handle all PAD ports here:
      case PAD_GAIN:
      case pg2: case pg3: case pg4: case pg5: case pg6: case pg7: case pg8: case pg9:
      case pg10: case pg11: case pg12: case pg13: case pg14: case pg15: case pg16:
          // hack the enum to access the right array slice
          printf("Gain Pad %i, pad# %i\n", port_index, port_index - int(PAD_GAIN) );
          ui->padData[ port_index-int(PAD_GAIN) ].gain = *(float*)buffer;
          break;
      
      default: break;
    }
    
    //ui->filterLowpass->value( argv[0]->f );
    Fl::unlock();
    Fl::awake();
    
    /*
    if ( format == 0 )
    {
      float value =  *(float *)buffer;
      switch ( port_index )
      {
        case asdf:
            cout << "Refractor: Retrigger control event, value = " << value << endl;
            //self->guiState->retrigger = value;
            //self->widget->redraw();
            break;
        case asdf:
            cout << "Refractor: Master volume event, value = " << value << endl;
            self->guiState->masterVol = value;
            self->widget->redraw();
      }
    }
    else
    {
      LV2_ATOM_SEQUENCE_FOREACH( (LV2_Atom_Sequence*)buffer, ev)
      {
        //self->frame_offset = ev->time.frames;
        
        if (ev->body.type == self->guiState->uris.midiEvent)
        {
          cout << "GUI got MIDI event!" << endl;
          //uint8_t* const data = (uint8_t* const)(ev + 1);
        }
      }
    }
    */
    
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

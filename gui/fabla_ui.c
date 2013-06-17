
#include <string>
#include <iostream>

// X window embedding
#include <FL/x.H>

// include the URI and global data of this plugin
#include "../dsp/ports.h"
#include "../dsp/shared.h"

// this is our custom widget include
#include "fabla.h"

// core spec include
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

// GUI
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "lv2/lv2plug.in/ns/ext/atom/util.h"

using namespace std;

typedef struct {
  FablaUI* widget;
  
  // URID map
  LV2_URID_Map* map;
  LV2_URID_Unmap* unmap;
  Fabla_URIs* uris;
  
} Fabla;


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
    
    
    
    
    map_uris( self->map, self->uris );
    
    //cout << "Creating UI!" << endl;
    self->widget = new FablaUI( parentXwindow );
    
    //cout << "Writing controller f(x)!" << endl;
    
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
            lv2_atom_object_get(obj, self->uris->fabla_Play, &body, 0);
            if (!body) {
              fprintf(stderr, "Malformed set message has no body.\n");
              return;
            }
            
            // Get int from body
            const LV2_Atom_Int* padNum = 0;
            lv2_atom_object_get( body, self->uris->fabla_pad, &padNum, 0);
            int* p = (int*)LV2_ATOM_BODY(padNum);
            int pad = *p;
            
            if ( pad >= 36 && pad < 36 + 16 )
            {
              printf("pad %i\n", pad );
            }
            
            /*
            LV2_ATOM_OBJECT_FOREACH(obj, i)
            {
              
              void* a = LV2_ATOM_BODY( &i->value );
              int* p = (int*)a;
              
              printf("uris:\n %s\n %i\n",
                self->unmap->unmap( self->unmap->handle, i->key ),
                *p );
            }
            */

            /*
            if (obj->body.otype != self->uris->fabla_pad) {
              printf("uris:\n %s\n %s\n %s\n",
                self->unmap->unmap( self->unmap->handle, obj->body.otype ),
                self->unmap->unmap( self->unmap->handle, self->uris->fabla_Play ),
                self->unmap->unmap( self->unmap->handle, self->uris->fabla_pad ) );
              
              return;
            }
            
            const LV2_Atom* pad = NULL;
            lv2_atom_object_get( obj, self->uris->fabla_pad, &pad, 0);
            
            if ( !pad ) {
              fprintf(stderr, "Malformed message has no pad number\n");
              return;
            }
            
            int p = *((int*)LV2_ATOM_BODY(atom));
            printf("value = %i\n", p);
            */
            
            /*
            const LV2_Atom_Int* padNum = 0;
            lv2_atom_object_get( (LV2_Atom_Object*)atom, self->uris->fabla_pad, &padNum, 0);
            printf("value = %i\n", *padNum);
            */
          }
          
          /*
          int k; float v;
          if (get_cc_key_value(&ui->uris, (LV2_Atom_Object*)atom, &k, &v)) {
            return;
          }
          */
          
          /*
          if ( true )
          {
            // get the object representing the rest of the data
            const LV2_Atom_Object* obj = (LV2_Atom_Object*)&buffer;
            
            printf("checking body.otype = %i, URI = %i\n", obj->body.otype, self->uris->fabla_Play);
            
            printf("uris:\n %s, %s\n",
              self->unmap->unmap( self->unmap->handle, obj->body.otype ),
              self->unmap->unmap( self->unmap->handle, self->uris->fabla_Play ) );
            
            // check if the type of the data is eg_name
            if ( obj->body.otype == self->uris->atom_eventTransfer )
            {
              printf("body == eventTransfer\n");
              LV2_Atom* pad = NULL;
              lv2_atom_object_get( obj->body, self->uris->fabla_Play, &pad, 0);
              
              if ( pad )
              {
                //int* s = (int*)LV2_ATOM_BODY(p);
                printf("int = %i\n", *((int*)pad) );
              }
            }
          }
          */
          
          /*
          printf("atom == %i\n", atom);
          
          printf("atom->type =  %i\n", atom->type);
          
          if (atom->type == self->uris->atom_eventTransfer )
          {
            printf("eventTransfer\n");
          }
          if (atom->type == self->uris->atom_Blank )
          {
            printf("atom_Blank\n");
            
            obj = (LV2_Atom_Object*)atom;
            lv2_atom_object_get(obj, self->uris->fabla_pad, &pad, 0);
          
            //printf( "body.otype = %i\n",obj->body.otype );
          
            printf("pad = %i\n", *pad );
          }
          */
          

          
          
          
          
          /*
          obj = (LV2_Atom_Object*)buffer;
          printf("cast done\n");
          if ( obj == 0 )
          {
            printf("obj == 0!! \n");
          }
          
          printf("past == 0\n");
          
          if ( obj->body ) {
            fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
          }
          */
          
          
          /*
          if (format == self->uris->atom_eventTransfer)
          {
            
            LV2_ATOM_SEQUENCE_FOREACH( (LV2_Atom_Sequence*)buffer, ev)
            {
              printf("event\n");
              if (ev->body.type == self->uris->fabla_Play)
              {
                printf("fabla_Play\n");
              }
            }
          }
          */
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

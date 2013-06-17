
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
    
    LV2_Atom* atom = 0;
    LV2_Atom_Object* obj = 0;
    LV2_Atom_Int* pad = 0;
    
    switch ( port_index )
    {
      case MASTER_VOL: ui->masterVol->value( v );         break;
      
      case ATOM_OUT:
          printf("atom out, buffer size = %i\n", buffer_size);
          atom = (LV2_Atom*)buffer;
          
          
          
          if ( true )
          {
            // get the object representing the rest of the data
            const LV2_Atom_Object* obj = (LV2_Atom_Object*)&buffer;
         
            // check if the type of the data is eg_name
            if ( obj->body.otype == self->uris->fabla_Play )
            {
              // get the data from the body
              const LV2_Atom_Object* body = NULL;
              lv2_atom_object_get(obj, self->uris->fabla_pad, &body, 0);
              
              // convert it to the type it is, and use it
              int* s = (int*)LV2_ATOM_BODY(body);
              printf("int = %i\n", s);
            }
          }
          
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

#include <stdlib.h>

#include <gtk/gtk.h>

#include "uris.hxx"

#include <iostream>
#include <gtkmm.h>

using namespace std;

#include "canvas.hxx"

#define FABLA_UI_URI "http://www.openavproductions.com/fabla/gui"

struct MidiEvent
{
  LV2_Atom atom;
  char buf[4];
};

static void
on_play_clicked(void* handle, int padNum)
{
  FablaUI* ui = (FablaUI*)handle;
  
  // write a MIDI event that will trigger that pad to play
  MidiEvent midiEvent;
  midiEvent.atom.type = ui->uris.midi_Event;
  midiEvent.atom.size = sizeof(char) * 4;
  midiEvent.buf[0] = 144;
  midiEvent.buf[1] = padNum;
  midiEvent.buf[2] = 127;
  
  // now write the LV2_Atom to the atom port:
  ui->write(ui->controller, SAMPLER_CONTROL, sizeof(MidiEvent),
          ui->uris.atom_eventTransfer,
          &midiEvent);
}


static void
on_load_clicked(void* handle, int padNum)
{
  FablaUI* ui = (FablaUI*)handle;

  /* Create a dialog to select a sample file. */
  GtkWidget* dialog = gtk_file_chooser_dialog_new(
    "Load Sample",
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
    NULL);
  
  // set the dialog as "modal": ie in front of: Ardour does this to plugins
  // automatically, and hence if its not done, the load dialog will pop up behind
  // the plugin UI. This is not desired, as it means dragging the filechooser before use
  gtk_window_set_modal( GTK_WINDOW ( dialog ), true );
  
  
  /* Run the dialog, and return if it is cancelled. */
  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
    gtk_widget_destroy(dialog);
    return;
  }

  /* Get the file path from the dialog. */
  char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

  /* Got what we need, destroy the dialog. */
  gtk_widget_destroy(dialog);

#define OBJ_BUF_SIZE 1024
  uint8_t obj_buf[OBJ_BUF_SIZE];
  lv2_atom_forge_set_buffer(&ui->forge, obj_buf, OBJ_BUF_SIZE);
  
  cout << "UI writing work to DSP now on pad " << padNum << endl;
  
  LV2_Atom* msg = write_set_file(&ui->forge, &ui->uris, padNum,
                                 filename, strlen(filename), 0, 0);
  
  if ( msg )
  {
    ui->write(ui->controller, SAMPLER_CONTROL, lv2_atom_total_size(msg),
              ui->uris.atom_eventTransfer,
              msg);
  }
  else
  {
    cout << __FILE__ << __LINE__ << " write_set_file() returned 0! " << endl;
  }
}

static GtkWidget* make_gui(FablaUI *self)
{
    Gtk::Main::init_gtkmm_internals(); // for QT hosts
    
    // Return a pointer to a gtk widget containing our GUI
    GtkWidget* container = gtk_vbox_new(FALSE, 2);
    
    self->canvas = new Canvas();
    
    self->canvas->ui_instance = self;
    //cout << "self = " << self << " ui->canvas->ui_instance " << self->canvas->ui_instance << endl;
    
    // get the gobject, and add that to the container
    gtk_container_add((GtkContainer*)container, (GtkWidget*)self->canvas->gobj() );
    
    return container;
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor*   descriptor,
            const char*               plugin_uri,
            const char*               bundle_path,
            LV2UI_Write_Function      write_function,
            LV2UI_Controller          controller,
            LV2UI_Widget*             widget,
            const LV2_Feature* const* features)
{
  FablaUI* ui = (FablaUI*)malloc(sizeof(FablaUI));
  
  ui->write = write_function;
  ui->controller = controller;
  
  Gtk::Main::init_gtkmm_internals(); // for QT hosts
  
  *widget = NULL;
  
  bool haveInstanceAccess = false;
  void* dspInstance = 0;
  
  for(int i = 0; features[i]; i++)
  {
    if (!strcmp(features[i]->URI, LV2_URID__map))
    {
      cout << "Found feature URID map!" << endl;
      ui->map = (LV2_URID_Map*)features[i]->data;
    }
  }

  if (!ui->map) {
    fprintf(stderr, "sampler_ui: Host does not support urid:Map\n");
    free(ui);
    return NULL;
  }

  map_sampler_uris(ui->map, &ui->uris);

  lv2_atom_forge_init(&ui->forge, ui->map);

  /*
  ui->box = gtk_hbox_new(FALSE, 4);
  ui->label = gtk_label_new("?");
  ui->button = gtk_button_new_with_label("Load Sample");
  gtk_box_pack_start(GTK_BOX(ui->box), ui->label, TRUE, TRUE, 4);
  gtk_box_pack_start(GTK_BOX(ui->box), ui->button, FALSE, TRUE, 4);
  g_signal_connect(ui->button, "clicked",
                   G_CALLBACK(on_load_clicked),
                   ui);
  */
  
  cout << "Creating UI!" << endl;
  *widget = (LV2UI_Widget)make_gui(ui);
  
  // instance access
  //ui->canvas->haveInstanceAccess = haveInstanceAccess;
  
  // dsp instance to canvas
  //ui->canvas->dspInstance = (Fabla*)dspInstance;
  
  return ui;
}

static void
cleanup(LV2UI_Handle handle)
{
  FablaUI* ui = (FablaUI*)handle;
  free(ui);
}

static void
port_event(LV2UI_Handle handle,
           uint32_t     port_index,
           uint32_t     buffer_size,
           uint32_t     format,
           const void*  buffer)
{
  FablaUI* ui = (FablaUI*)handle;
  
  //fprintf(stderr, "UI port_Event\n");
  
  if (format == ui->uris.atom_eventTransfer)
  {
    LV2_Atom* atom = (LV2_Atom*)buffer;
    
    
    
    if (atom->type == ui->uris.atom_Blank)
    {
      LV2_Atom_Object* obj      = (LV2_Atom_Object*)atom;
      
      //fprintf(stderr, "atom_object->atom->type  %i\n", obj->atom.type );
      
      if ( obj->body.otype == ui->uris.playSample )
      {
        // play event
        //fprintf(stderr, "play or stop command\n");
        
        const LV2_Atom_Int* padAtom = read_play_sample(&ui->uris, obj);
        
        if ( padAtom )
        {
          int pad = padAtom->body;
          if ( pad >= 36 && pad < 36+16 )
          {
            ui->canvas->padState[pad-36] = Canvas::PAD_PLAYING;
            ui->canvas->redraw();
          }
          else
          {
            fprintf(stderr, "pad out of bounds!" );
          }
        }
        return;
      }
      
      else if ( obj->body.otype == ui->uris.stopSample )
      {
        fprintf(stderr, "Stop sample atom recieved!" );
        const LV2_Atom_Int* padAtom = read_stop_sample(&ui->uris, obj);
        
        if ( padAtom )
        {
          int pad = padAtom->body;
          if (  pad >= 0 && pad < 16 )
          {
            //fprintf(stderr, "writing LOAODED to pad!" );
            ui->canvas->padState[pad] = Canvas::PAD_LOADED;
            ui->canvas->redraw();
          }
          else
          {
            fprintf(stderr, "pad out of bounds!" );
          }
        }
        return;
      }
      
      else if ( obj->body.otype == ui->uris.patch_Set )
      {
        const LV2_Atom*  file_uri = read_set_file(&ui->uris, obj);
        
        if (!file_uri)
        {
          fprintf(stderr, "file URI not valid!" );
        }
        
        const LV2_Atom_Int* sampleNum = read_set_file_sample_number(&ui->uris, obj);
        if ( sampleNum )
        {
          int pad = sampleNum->body;
          const char* uri = (const char*)LV2_ATOM_BODY(file_uri);
          
          if ( pad >= 0 && pad < 16 )
          {
            cout << " File path " << uri << "  on pad " << pad << endl;
            ui->canvas->sampleNames[pad] = uri;
            ui->canvas->padState[pad] = Canvas::PAD_LOADED;
            ui->canvas->redraw();
          }
          else
          {
            fprintf(stderr, "out of bounds pad, on loading!" );
          }
        }
      }
      
    }
    else
    {
      fprintf(stderr, "Unknown message type.\n");
    }
  }
  else if ( format == 0 )
  {
    float value = *((float*)buffer);
    fprintf(stderr, "Port number %i : value %f\n", port_index, value );
    
    switch ( port_index )
    {
      case SAMPLER_REVERB_SIZE:  ui->canvas->reverbTime  = value; break;
      case SAMPLER_REVERB_WET:   ui->canvas->reverbAmp   = value; break;
      case SAMPLER_MASTER_VOL:   ui->canvas->volume      = value; break;
      case SAMPLER_HIGHPASS:     ui->canvas->highpass    = value; break;
      case SAMPLER_LOWPASS:      ui->canvas->lowpass     = value; break;
      case SAMPLER_ECHO_TIME:    ui->canvas->echoTime    = value; break;
      case SAMPLER_ECHO_FEEDBACK:ui->canvas->echoAmp     = value; break;
    }
    ui->canvas->redraw();
    
  }
  else
  {
    fprintf(stderr, "Unknown format %i\n", format);
  }
}

const void*
extension_data(const char* uri)
{
  return NULL;
}

static const LV2UI_Descriptor descriptor = {
  FABLA_UI_URI,
  instantiate,
  cleanup,
  port_event,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor*
lv2ui_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &descriptor;
  default:
    return NULL;
  }
}

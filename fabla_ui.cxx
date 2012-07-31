#include <stdlib.h>

#include <gtk/gtk.h>

#include "uris.h"

#include <iostream>
#include <gtkmm.h>

using namespace std;

#include "canvas.hxx"

#include "instance-access.h"

#define FABLA_UI_URI "http://www.openavproductions.com/fabla/gui"

static void
on_load_clicked(void* handle)
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

  /* Run the dialog, and return if it is cancelled. */
  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
    gtk_widget_destroy(dialog);
    return;
  }

  /* Get the file path from the dialog. */
  char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

  /* Got what we need, destroy the dialog. */
  gtk_widget_destroy(dialog);
  
  cout << "write () from UI = " << ui->write << endl;
  cout << "controller (from UI) = " << ui->controller << endl;

#define OBJ_BUF_SIZE 1024
  uint8_t obj_buf[OBJ_BUF_SIZE];
  lv2_atom_forge_set_buffer(&ui->forge, obj_buf, OBJ_BUF_SIZE);
  
  LV2_Atom* msg = write_set_file(&ui->forge, &ui->uris,
                                 filename, strlen(filename));

  ui->write(ui->controller, 0, lv2_atom_total_size(msg),
            ui->uris.atom_eventTransfer,
            msg);
}

static GtkWidget* make_gui(FablaUI *self) {
    
    cout << "Init GTKMM!" << endl;
    Gtk::Main::init_gtkmm_internals(); // for QT hosts
    
    
    cout << "create container!" << endl;
    // Return a pointer to a gtk widget containing our GUI
    GtkWidget* container = gtk_vbox_new(FALSE, 2);
    
    cout << "new canvast!" << endl;
    self->canvas = new Canvas();
    
    self->canvas->ui_instance = self;
    cout << "self = " << self << " ui->canvas->ui_instance " << self->canvas->ui_instance << endl;
    
    
    cout << "adding to window!" << endl;
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
  
  for(int i = 0; features[i]; i++)
  {
    if (!strcmp(features[i]->URI, LV2_INSTANCE_ACCESS_URI ))
    {
      cout << "Found feature instance access!" << endl;
      //self->dspInstance = (FablaUI*)features[i]->data;
      haveInstanceAccess = true;
    }
    else if (!strcmp(features[i]->URI, LV2_URID__map))
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
  
  ui->canvas->haveInstanceAccess = haveInstanceAccess;
  
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
  if (format == ui->uris.atom_eventTransfer) {
    LV2_Atom* atom = (LV2_Atom*)buffer;
    if (atom->type == ui->uris.atom_Blank) {
      LV2_Atom_Object* obj      = (LV2_Atom_Object*)atom;
      const LV2_Atom*  file_uri = read_set_file(&ui->uris, obj);
      if (!file_uri) {
        fprintf(stderr, "Unknown message sent to UI.\n");
        return;
      }

      const char* uri = (const char*)LV2_ATOM_BODY(file_uri);
      
      cout << " File path " << uri << endl;
      //gtk_label_set_text(GTK_LABEL(ui->label), uri);
    } else {
      fprintf(stderr, "Unknown message type.\n");
    }
  } else {
    fprintf(stderr, "Unknown format.\n");
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


#ifndef FABLA_CANVAS_HXX
#define FABLA_CANVAS_HXX

#include <gtkmm/drawingarea.h>

#include <vector>
#include <sstream>
#include <iostream>

#include <gdkmm.h>
#include <glibmm.h>

#include "uris.hxx"

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

using namespace std;

class Canvas;

// this needs to be accessible by fabla.c
typedef struct {
  LV2_Atom_Forge forge;

  LV2_URID_Map* map;
  FablaURIs   uris;

  LV2UI_Write_Function write;
  LV2UI_Controller     controller;
  
  Canvas* canvas;
} FablaUI;


static void on_play_clicked(void* handle, int);
static void on_load_clicked(void* handle, int);

class Canvas : public Gtk::DrawingArea
{
  public:
    // core GUI instance writes to these on init
    FablaUI* ui_instance;
    
    Fabla* dspInstance;
    
    int selectedSample;
    std::string sampleNames[16];
    
    // source  (always active)
    float attack, release;
    float speed, gain;
    
    // effect
    float echoTime, echoAmp;
    bool echoActive, reverbActive;
    float reverbTime, reverbAmp;
    
    // remove
    bool  highpassActive;
    bool  lowpassActive;
    float highpass, lowpass;
    
    // master
    float limiter;
    float volume;
    
    enum PadState {
      PAD_EMPTY = 0,
      PAD_LOADED,
      PAD_PLAYING,
    };
    
    PadState padState[16];
    
    Canvas()
    {
      for(int i = 0; i < 16; i++)
        padState[i] = PAD_EMPTY;
      
      // source
      attack = 0.0;
      release = 0.0;
      speed = 1;
      gain = 1;
      
      // effect
      echoAmp = 0.5;
      echoTime = 0.5;
      echoActive = false;
      reverbAmp = 0.5;
      reverbTime = 0.5;
      reverbActive = false;
      
      // remove
      lowpass  = 1.f;
      highpass = 0.f;
      lowpassActive = false;
      highpassActive = false;
      
      // master
      volume = 0.707;
      limiter = 0.f;
      masterClicked = false;
      
      selectedSample = 0;
      
      width = 690;
      height = 546;
      set_size_request( width, height );
      
      loadImages();
      
      // connect GTK signals
      add_events( Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK| Gdk::POINTER_MOTION_MASK );
      
      signal_motion_notify_event() .connect( sigc::mem_fun(*this, &Canvas::on_mouse_move ) );
      signal_button_press_event()  .connect( sigc::mem_fun(*this, &Canvas::on_button_press_event) );
      signal_button_release_event().connect( sigc::mem_fun(*this, &Canvas::on_button_press_event) );
    }
    
    void drawPads(Cairo::RefPtr<Cairo::Context> cr);
    void drawSource(Cairo::RefPtr<Cairo::Context> cr);
    void drawEffect(Cairo::RefPtr<Cairo::Context> cr);
    void drawMaster(Cairo::RefPtr<Cairo::Context> cr);
    void drawRemove(Cairo::RefPtr<Cairo::Context> cr);
    void drawWaveform(Cairo::RefPtr<Cairo::Context> cr);
    
    bool redraw()
    {
      Glib::RefPtr<Gdk::Window> win = get_window();
      if (win)
      {
          Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                  get_allocation().get_height());
          win->invalidate_rect(r, false);
      }
      return true;
    }
    
    // will redraw a portion of the screen
    bool redraw(int x, int y, int sx, int sy)
    {
      Glib::RefPtr<Gdk::Window> win = get_window();
      if (win)
      {
          Gdk::Rectangle r(x,y,sx,sy);
          win->invalidate_rect(r, false);
      }
      return true;
    }
    
  protected:
    bool masterClicked;
    int width, height;
    
    int clickPlaySample;
    
    // Image header
    bool headerLoaded;
    Glib::RefPtr< Gdk::Pixbuf > imagePointer;
    Cairo::RefPtr< Cairo::ImageSurface > imageSurfacePointer;
    
    Glib::RefPtr< Gdk::Pixbuf > padPlayPixbuf;
    Glib::RefPtr< Gdk::Pixbuf > padLoadPixbuf;
    Glib::RefPtr< Gdk::Pixbuf > padEmptyPixbuf;
    Glib::RefPtr< Gdk::Pixbuf > padSelectPixbuf;
    Cairo::RefPtr< Cairo::ImageSurface > padPlayImageSurface;
    Cairo::RefPtr< Cairo::ImageSurface > padLoadImageSurface;
    Cairo::RefPtr< Cairo::ImageSurface > padEmptyImageSurface;
    Cairo::RefPtr< Cairo::ImageSurface > padSelectImageSurface;
    
    enum Colour {
      COLOUR_ORANGE_1 = 0,
      COLOUR_ORANGE_2,
      COLOUR_ORANGE_3,
      COLOUR_GREEN_1,
      COLOUR_GREEN_2,
      COLOUR_GREEN_3,
      COLOUR_BLUE_1,
      COLOUR_BLUE_2,
      COLOUR_BLUE_3,
      COLOUR_PURPLE_1,
      COLOUR_PURPLE_2,
      COLOUR_PURPLE_3,
      COLOUR_GREY_1,
      COLOUR_GREY_2,
      COLOUR_GREY_3,
      COLOUR_GREY_4,
      // specials
      COLOUR_BACKGROUND,
      COLOUR_RECORD_RED,
      COLOUR_TRANSPARENT,
    };
    
    void loadImages()
    {
      // Load big header image
      try {
        imagePointer = Gdk::Pixbuf::create_from_file ("/usr/lib/lv2/fabla.lv2/header.png");
        headerLoaded = true;
      }
      catch(Glib::FileError& e)
      {
        headerLoaded = false;
      }
      
      if ( !headerLoaded ) // if not in /usr/lib/lv2, try local
      {
        try {
          imagePointer = Gdk::Pixbuf::create_from_file ("/usr/local/lib/lv2/fabla.lv2/header.png");
          headerLoaded = true;
        }
        catch(Glib::FileError& e)
        {
          cout << "Fabla: Header image could not be loaded! Continuing..." << e.what() << endl;
          headerLoaded = false;
          return;
        }
      }
      // Detect transparent colors for loaded image
      Cairo::Format format = Cairo::FORMAT_RGB24;
      if (imagePointer->get_has_alpha())
      {
          format = Cairo::FORMAT_ARGB32;
      }
      // Create a new ImageSurface
      imageSurfacePointer = Cairo::ImageSurface::create  (format, imagePointer->get_width(), imagePointer->get_height());
      // Create the new Context for the ImageSurface
      Cairo::RefPtr< Cairo::Context > imageContextPointer = Cairo::Context::create (imageSurfacePointer);
      // Draw the image on the new Context
      Gdk::Cairo::set_source_pixbuf (imageContextPointer, imagePointer, 0.0, 0.0);
      imageContextPointer->paint();
      
      
      // PAD images
      padPlayPixbuf  = Gdk::Pixbuf::create_from_file ("/usr/lib/lv2/fabla.lv2/padplay.png" );
      padLoadPixbuf  = Gdk::Pixbuf::create_from_file ("/usr/lib/lv2/fabla.lv2/padload.png");
      padEmptyPixbuf = Gdk::Pixbuf::create_from_file ("/usr/lib/lv2/fabla.lv2/padempty.png");
      padSelectPixbuf= Gdk::Pixbuf::create_from_file ("/usr/lib/lv2/fabla.lv2/padselect.png");
      
      padPlayImageSurface  = Cairo::ImageSurface::create  ( Cairo::FORMAT_ARGB32, padPlayPixbuf->get_width(), padPlayPixbuf->get_height()  );
      padLoadImageSurface  = Cairo::ImageSurface::create  ( Cairo::FORMAT_ARGB32, padLoadPixbuf->get_width(), padLoadPixbuf->get_height()  );
      padEmptyImageSurface = Cairo::ImageSurface::create  ( Cairo::FORMAT_ARGB32, padEmptyPixbuf->get_width(),padEmptyPixbuf->get_height() );
      padSelectImageSurface= Cairo::ImageSurface::create  ( Cairo::FORMAT_ARGB32,padSelectPixbuf->get_width(),padSelectPixbuf->get_height());
      
      Cairo::RefPtr< Cairo::Context > padPlayContext  = Cairo::Context::create (padPlayImageSurface );
      Cairo::RefPtr< Cairo::Context > padLoadContext  = Cairo::Context::create (padLoadImageSurface );
      Cairo::RefPtr< Cairo::Context > padEmptyContext = Cairo::Context::create (padEmptyImageSurface);
      Cairo::RefPtr< Cairo::Context > padSelectContext= Cairo::Context::create (padSelectImageSurface);
      
      Gdk::Cairo::set_source_pixbuf (padPlayContext , padPlayPixbuf , 0.0, 0.0);
      Gdk::Cairo::set_source_pixbuf (padLoadContext , padLoadPixbuf , 0.0, 0.0);
      Gdk::Cairo::set_source_pixbuf (padEmptyContext, padEmptyPixbuf, 0.0, 0.0);
      Gdk::Cairo::set_source_pixbuf (padSelectContext,padSelectPixbuf,0.0, 0.0);
      
      padPlayContext->paint();
      padLoadContext->paint();
      padEmptyContext->paint();
      padSelectContext->paint();
      
      headerLoaded = true;
    }
    
    bool on_expose_event			(GdkEventExpose* event)
    {
      Glib::RefPtr<Gdk::Window> window = get_window();
      
      if(window)
      {
        Gtk::Allocation allocation = get_allocation();
        width = allocation.get_width();
        height = allocation.get_height();
        
        // clip reigon
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
        cr->rectangle(event->area.x, event->area.y,
                event->area.width, event->area.height);
        cr->clip();
        
        cr->rectangle(event->area.x, event->area.y,
            event->area.width, event->area.height);
        cr->set_source_rgb(0.0,0.0,0.0);
        cr->fill();
        
        if ( headerLoaded )
        {
          cr->save();
          
          // draw header
          cr->set_source (imageSurfacePointer, 0.0, 0.0);
          cr->rectangle (0.0, 0.0, imagePointer->get_width(), imagePointer->get_height());
          cr->clip();
          cr->paint();
          
          cr->restore();
        }
        
        // loop draw the bottom 5 backgrounds
        int drawX = 33;
        int drawY = 74;
        
        // Waveform
        Background( cr, drawX, drawY, 256, 160, "Waveform");
        
        // Pads
        drawY += 160 + 24;
        Background( cr, drawX, drawY, 256, 256 + 23-9, "Pads");
        
        
        // Affect & Remove
        drawX += 256 + 24;
        drawY = 74;
        Background(cr, drawX, drawY, 159, 215, "Source");
        drawY += 215 + 24;
        Background(cr, drawX, drawY, 159, 215, "Remove");
        
        // Adjust and Master
        drawX += 159 + 24;
        drawY = 74;
        Background(cr, drawX, drawY, 159, 215, "Effect");
        drawY += 215 + 24;
        Background(cr, drawX, drawY, 159, 215, "Master");
        
        // fill in widgets
        drawMaster(cr);
        
        drawRemove(cr);
        
        drawSource(cr);
        
        drawEffect(cr);
        
        drawWaveform(cr);
        
        drawPads(cr);
      }
      return true;
    }
    
    void Background(Cairo::RefPtr<Cairo::Context> cr, float x, float y, float sizeX, float sizeY, std::string name)
    {
      // fill background
      cr->rectangle( x, y, sizeX, sizeY);
      setColour( cr, COLOUR_GREY_3 );
      cr->fill();
      
      // set up dashed lines, 1 px off, 1 px on
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      
      // loop over each 2nd line, drawing dots
      for ( int i = x; i < x + sizeX; i += 4 )
      {
        cr->move_to( i, y );
        cr->line_to( i, y + sizeY );
      }
      
      setColour( cr, COLOUR_GREY_4, 0.5 );
      cr->stroke();
      cr->unset_dash();
      
      // draw header
      if ( true )
      {
        // backing
        cr->rectangle( x, y, sizeX, 20);
        setColour( cr, COLOUR_GREY_4 );
        cr->fill();
        
        // text
        std::string text = name;
        cr->move_to( x + 10, y + 14 );
        setColour( cr, COLOUR_BLUE_1 );
        cr->set_font_size( 10 );
        cr->show_text( text );
        
        // lower stripe
        cr->move_to( x        , y + 20 );
        cr->line_to( x + sizeX, y + 20 );
        setColour( cr, COLOUR_BLUE_1 );
        cr->stroke();
      }
      
      // stroke rim
      cr->rectangle( x, y, sizeX, sizeY);
      setColour( cr, COLOUR_BLUE_1 );
      cr->stroke();
    }
    
    void SimpleDial( Cairo::RefPtr<Cairo::Context> cr, bool active, float x, float y, float value)
    {
      int xc = x + 16;
      int yc = y + 22;
      
      float radius = 14;
      
      cr->set_line_cap( Cairo::LINE_CAP_ROUND );
      cr->set_line_join( Cairo::LINE_JOIN_ROUND);
      cr->set_line_width(2.8);
      
      // Arc Angle Value
      cr->set_line_width(2.4);
      cr->move_to(xc,yc);
      cr->set_source_rgba( 0,0,0,0 );
      cr->stroke();
      
      // main arc
      if ( active )
        setColour(cr, COLOUR_GREY_4 );
      else
        setColour(cr, COLOUR_GREY_3 );
      cr->arc(xc,yc, radius, 2.46, 0.75 );
      cr->move_to(xc,yc);
      cr->stroke();
      
      cr->set_line_width(2.8);
      float angle;
      
      if ( value < 0 )
        value = 0.f;
      
      angle = 2.46 + (4.54 * value);
      
      if ( active )
        setColour(cr, COLOUR_GREY_1 );
      else
        setColour(cr, COLOUR_GREY_2 );
      
      cr->set_line_width(1.7);
      cr->arc(xc,yc, 13, 2.46, angle );
      cr->line_to(xc,yc);
      cr->stroke();
      cr->arc(xc,yc, 17, 2.46, angle );
      cr->line_to(xc,yc);
      cr->stroke();
    }
    
    void setColour( Cairo::RefPtr<Cairo::Context> cr, Colour c, float alpha)
    {
      switch( c )
      {
        case COLOUR_ORANGE_1:
          cr->set_source_rgba( 255 / 255.f, 104 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_ORANGE_2:
          cr->set_source_rgba( 178 / 255.f,  71 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_ORANGE_3:
          cr->set_source_rgba(  89 / 255.f,  35 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_GREEN_1:
          cr->set_source_rgba(  25 / 255.f, 255 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_GREEN_2:
          cr->set_source_rgba(  17 / 255.f, 179 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_GREEN_3:
          cr->set_source_rgba(   8 / 255.f,  89 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_BLUE_1:
          cr->set_source_rgba(   0 / 255.f, 153 / 255.f , 255 / 255.f , alpha ); break;
        case COLOUR_BLUE_2:
          cr->set_source_rgba(  20 / 255.f,  73 / 255.f , 109 / 255.f , alpha ); break;
        case COLOUR_BLUE_3:
          cr->set_source_rgba(   0 / 255.f,  53 / 255.f ,  89 / 255.f , alpha ); break;
        case COLOUR_PURPLE_1:
          cr->set_source_rgba( 230 / 255.f,   0 / 255.f , 255 / 255.f , alpha ); break;
        case COLOUR_PURPLE_2:
          cr->set_source_rgba( 161 / 255.f,   0 / 255.f , 179 / 255.f , alpha ); break;
        case COLOUR_PURPLE_3:
          cr->set_source_rgba(  80 / 255.f,   0 / 255.f ,  89 / 255.f , alpha ); break;
        case COLOUR_GREY_1:
          cr->set_source_rgba( 130 / 255.f, 130 / 255.f , 130 / 255.f , alpha ); break;
        case COLOUR_GREY_2:
          cr->set_source_rgba(  98 / 255.f,  98 / 255.f ,  98 / 255.f , alpha ); break;
        case COLOUR_GREY_3:
          cr->set_source_rgba(  66 / 255.f,  66 / 255.f ,  66 / 255.f , alpha ); break;
        case COLOUR_GREY_4:
          cr->set_source_rgba(  28 / 255.f,  28 / 255.f ,  28 / 255.f , alpha ); break;
        case COLOUR_RECORD_RED:
          cr->set_source_rgba(  226 / 255.f, 0/255.f , 0/255.f, alpha ); break;
        case COLOUR_TRANSPARENT:
          cr->set_source_rgba(  0, 0, 0, 0.f ); break;
        case COLOUR_BACKGROUND: default:
          cr->set_source_rgba(  40 / 255.f,  40 / 255.f ,  40 / 255.f , alpha ); break;
      }
    }
    
    void setColour( Cairo::RefPtr<Cairo::Context> cr, Colour c)
    {
      setColour(cr, c, 1.f);
    }
    
    bool on_button_press_event(GdkEventButton* event)
    {
      int x = event->x;
      int y = event->y;
      //std::cout << x << " " << y << endl;
      
      if ( x > 509 && y > 448 && x < 544 && y < 464 ) // LOAD BUTTON
      {
        cout << "load clicked, ui_instrance " << ui_instance << endl;
        // load clicked
        //on_load_clicked( ui_instance );
      }

      if ( x > 324 && y > 105 && x < 462 && y < 188 && event->type == Gdk::BUTTON_PRESS ) // ADSR / Gain
      {
        // X defines what changes: attack, release, gain
        // Y defines the new value for that parameter 
        
        float X = (event->x - 324) / 138.f;
        float Y = 1 - (event->y - 105) / 82.f;
        if ( X < 0.25 ) // Attack
        {
          attack = Y;
          cout << "attack = " << attack << endl;
        }
        else if ( X > 0.33 && X < 0.75 ) // Gain
        {
          gain = 0.5 + Y;
        }
        else if ( X > 0.75 ) // Release
        {
          release = Y;
          cout << "release = " << release << endl;
        }
        redraw();
      }
      
      if ( x > 324 && y > 198 && x < 462 && y < 282 ) // SPEED / PAN
      {
        cout << "Speed pan" << endl;
      }
      
      
      if ( x > 324 && y > 433 && x < 462 && y < 517 ) // lowpass
      {
        if ( event->button == 1 && event->type == Gdk::BUTTON_PRESS ) // set cutoff
        {
          lowpass = (event->x - 324) / 138.f;
          cout << "lowpass graph click at " << x << endl;
          if ( lowpassActive )
            ui_instance->write( ui_instance->controller, SAMPLER_LOWPASS, sizeof(float), 0, (const void*) &lowpass);
        }
        else if ( event->type == Gdk::BUTTON_PRESS  && event->button == 3 ) // turn off / on
        {
          lowpassActive = !lowpassActive;
          cout << "lowpassActive= " << lowpassActive << endl;
          if ( lowpassActive )
            ui_instance->write( ui_instance->controller, SAMPLER_LOWPASS, sizeof(float), 0, (const void*) &lowpass);
          else
          {
            float tmp = 1.f;
            ui_instance->write( ui_instance->controller, SAMPLER_LOWPASS, sizeof(float), 0, (const void*) &tmp);
          }
        }
        redraw();
      }
      
      if ( x > 324 && y > 341 && x < 462 && y < 424 ) // Highpass
      {
        if ( event->button == 1 && event->type == Gdk::BUTTON_PRESS ) // set cutoff
        {
          highpass = (event->x - 324) / 138.f;
          cout << "highpass graph click at " << x << endl;
          if ( highpassActive )
            ui_instance->write( ui_instance->controller, SAMPLER_HIGHPASS, sizeof(float), 0, (const void*) &highpass);
        }
        else if ( event->type == Gdk::BUTTON_PRESS && event->button == 3 ) // turn off / on
        {
          highpassActive = !highpassActive;
          cout << "HigpassActive = " << highpassActive << endl;
          if ( highpassActive )
            ui_instance->write( ui_instance->controller, SAMPLER_HIGHPASS, sizeof(float), 0, (const void*) &highpass);
          else
          {
            float tmp = 0.f;
            ui_instance->write( ui_instance->controller, SAMPLER_HIGHPASS, sizeof(float), 0, (const void*) &tmp);
          }
        }
        redraw();
      }
      
      if ( x > 508 && y > 105 && x < 646 && y < 189 ) // ECHO
      {
        if ( event->button == 1 && event->type == Gdk::BUTTON_PRESS )
        {
          echoTime = (event->x - 508) / 138.f;
          echoAmp  = 1 - (event->y - 105) / 82.f;
          
          cout << "ECHO graph click at " << x << " " << y << " eT: " << echoTime << " eA " << echoAmp << endl;
          if ( echoActive )
          {
            ui_instance->write( ui_instance->controller, SAMPLER_ECHO_FEEDBACK, sizeof(float), 0, (const void*) &echoAmp );
            ui_instance->write( ui_instance->controller, SAMPLER_ECHO_TIME    , sizeof(float), 0, (const void*) &echoTime);
          }
        }
        else if ( event->type == Gdk::BUTTON_PRESS && event->button == 3 )
        {
          echoActive = !echoActive;
          cout << "echoActive = " << echoActive << endl;
          if ( echoActive )
          {
            ui_instance->write( ui_instance->controller, SAMPLER_ECHO_TIME, sizeof(float), 0, (const void*) &echoTime);
            ui_instance->write( ui_instance->controller, SAMPLER_ECHO_FEEDBACK , sizeof(float), 0, (const void*) &echoAmp );
          }
          else
          {
            float tmp = 0.f;
            ui_instance->write( ui_instance->controller, SAMPLER_ECHO_FEEDBACK , sizeof(float), 0, (const void*) &tmp );
          }
        }
        redraw();
      }
      
      if ( x > 508 && y > 199 && x < 646 && y < 281 ) // REVERB
      {
        
        
        if ( event->button == 1 && event->type == Gdk::BUTTON_PRESS )
        {
          reverbTime = (event->x - 508) / 138.f;
          reverbAmp  = 1 - (event->y - 199) / 82.f;
          
          cout << "reverb graph click at " << x << " " << y << endl;
          if ( reverbActive ) // avoid always write "wet" even when disabled
          {
            ui_instance->write( ui_instance->controller, SAMPLER_REVERB_SIZE, sizeof(float), 0, (const void*) &reverbTime);
            ui_instance->write( ui_instance->controller, SAMPLER_REVERB_WET , sizeof(float), 0, (const void*) &reverbAmp );
          }
        }
        else if ( event->type == Gdk::BUTTON_PRESS && event->button == 3 )
        {
          reverbActive = !reverbActive;
          cout << "reverbActive = " << reverbActive << endl;
          if ( reverbActive )
          {
            ui_instance->write( ui_instance->controller, SAMPLER_REVERB_SIZE, sizeof(float), 0, (const void*) &reverbTime);
            ui_instance->write( ui_instance->controller, SAMPLER_REVERB_WET , sizeof(float), 0, (const void*) &reverbAmp );
          }
          else
          {
            float tmp = 0.f;
            ui_instance->write( ui_instance->controller, SAMPLER_REVERB_WET , sizeof(float), 0, (const void*) &tmp );
          }
        }
        redraw();
      }
      
      if ( x > 588 && y > 340 && x < 644 && y < 518 && event->type == Gdk::BUTTON_PRESS ) // master vol
      {
        masterClicked = true;
        volume = 1 - (event->y - 340) / 178.f;
        ui_instance->write( ui_instance->controller, SAMPLER_MASTER_VOL , sizeof(float), 0, (const void*) &volume );
        redraw();
      }
      else if (event->type == Gdk::BUTTON_RELEASE)
      {
        masterClicked = false;
      }
      
      if ( x > 35 && y > 280 && x < 290 && y < 528 ) // PADS
      {
        int x = event->x - 35;
        int y = event->y - 280;
        
        int pad = 8+ ((x / 62) + 4-(y/62)*4);
        
        //cout << "Pad " << pad << " clicked " << endl;
        
        selectedSample = pad;
        
        if ( event->button == 1 && event->type == Gdk::BUTTON_PRESS ) // play event
        {
          padState[pad] = PAD_PLAYING;
          on_play_clicked( ui_instance, pad );
          
          // save the current clicked sample number, so when we release
          // the mouse button, we can deactivate that one, even if the mouse
          // moved
          clickPlaySample = pad;
          
          redraw();
        }
        if ( event->button == 1 && event->type == Gdk::BUTTON_RELEASE ) // play event
        {
          if ( sampleNames[clickPlaySample].compare( "" ) )
            padState[clickPlaySample] = PAD_LOADED;
          else
            padState[clickPlaySample] = PAD_EMPTY;
          redraw();
        }
        else if ( event->button == 3 && event->type == Gdk::BUTTON_PRESS  ) // load event
        {
          on_load_clicked( ui_instance, pad );
        }
      }
      
      
      return true;
    }
    
    bool on_mouse_move(GdkEventMotion* event)
    {
      int x = event->x;
      int y = event->y;
      
      if ( masterClicked ) // MASTER
      {
        float tmp = 1 - (event->y - 340) / 178.f;
        
        if ( tmp > 1 ) tmp = 1.0f;
        if ( tmp < 0 ) tmp = 0.0f;
        
        volume = tmp;
        
        ui_instance->write( ui_instance->controller, SAMPLER_MASTER_VOL , sizeof(float), 0, (const void*) &volume );
        redraw();
      }
      
      
      
      /*
      // if mouse moves over "openavproductions.com", show "link" cursor
      if ( event->x > 623 && event->y > 20 && event->y < 70 )
      {
        Glib::RefPtr <Gdk::Window> ref_window;
        ref_window = get_window();
        Gdk::Cursor m_Cursor(Gdk::HAND1);
        ref_window->set_cursor(m_Cursor);
      }
      else
      {
        Glib::RefPtr <Gdk::Window> ref_window;
        ref_window = get_window();
        ref_window->set_cursor();
      }
      
      
      
      
      // when a click occurs, float* target is set to that location,
      // here we just work with target
      if ( mouseDown && target && clickedWidget != CLICKED_WIDGET_NONE )
      {
        float click  = 1 - (event->y / height);
        float clickX = 1 - (event->x / height);
        
        float delta = clickValue - click;
        
        bool invert = false;
        
        if ( clickedWidget == CLICKED_WIDGET_HIGHPASS )
        {
          // use X delta, not Y
          delta = clickValueX - clickX;
          invert = true;
          //cout << "Highpass delta = " << delta << endl;
        }
        else if ( clickedWidget == CLICKED_WIDGET_LOWPASS  )
        {
          // use X delta, not Y
          delta = clickValueX - clickX;
          invert = true;
          //cout << "Lowpass delta = " << delta << endl;
        }
        
        clickValue  = 1 - ( event->y / height); // reset the starting point, no acceleration 
        clickValueX = 1 - ( event->x / height); // reset the starting point, no acceleration 
        
        //cout << "Delta = " << click << endl;
        
        // scale delta to make it have more effect
        float value = (*target) - delta * 3;
        
        if ( invert )
        {
          value = (*target) + delta * 3;
        }
        
        if ( value > 1.f) value = 1.f;
        if ( value < 0.f) value = 0.f;
        
        if ( clickedWidget == REFRACTOR_CONTROL_RETRIGGER )
        {
          guiState->retrigger = value;
          value = int(value * 16.f);
          redraw(227,447, 40, 40);
        }
        else if ( clickedWidget == REFRACTOR_MASTER_VOLUME )
        {
          guiState->masterVol = value;
          redraw( 859, 340, 60, 190);
        }
        else if ( clickedWidget == REFRACTOR_CONTROL_BITCRUSH )
        {
          guiState->bitcrush = value;
          redraw( 406,330, 150, 100);
        }
        else if ( clickedWidget == REFRACTOR_CONTROL_DISTORTION )
        {
          guiState->distortion = value;
          redraw( 406,430, 150, 100);
        }
        else if ( clickedWidget == CLICKED_WIDGET_HIGHPASS )
        {
          guiState->highpass = value;
          redraw( 594,330, 150, 100);
        }
        else if ( clickedWidget == CLICKED_WIDGET_LOWPASS )
        {
          guiState->lowpass = value;
          redraw( 594,430, 150, 100);
        }
        else if ( clickedWidget == CLICKED_WIDGET_PITCH )
        {
          //cout << "Setting & drawing pitch / source" << endl;
          guiState->pitch = value;
          redraw( 127, 340, 70, 200);
        }
        //cout << "Writing value " << value << " scaled to " << *target << " to port index" << clickedWidget << endl;
        guiState->write_function( guiState->controller, clickedWidget, sizeof(float), 0, (const void*)&value);
      }
      */
      
      return true;
    }
    
};


#endif

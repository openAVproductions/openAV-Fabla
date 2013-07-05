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


#ifndef AVTK_WAVEFORM_H
#define AVTK_WAVEFORM_H

#include <FL/Fl_Widget.H>
#include <vector>
#include <string>
#include <sstream>
#include <cairomm/context.h>
#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace Cairo;

namespace Avtk
{

class Waveform : public Fl_Widget
{
  public:
    Waveform(int _x, int _y, int _w, int _h, const char *_label=0 ):
        Fl_Widget(_x, _y, _w, _h, _label)
    {
      x = _x;
      y = _y;
      w = _w;
      h = _h;
      
      label = _label;
      
      highlight = false;
      newWaveform = false;
      
      waveformCr = 0;
      waveformSurf = 0;
      
      realLength = 0;
      data = 0;
      
      newWaveform = true;
    }
    
    bool highlight;
    int x, y, w, h;
    const char* label;
    
    cairo_t*          waveformCr;
    cairo_surface_t*  waveformSurf;
    
    bool newWaveform;
    
    string waveformName;
    int    data_size;
    long   realLength;
    float* data;
    
    void setData( int numDataPoints, long realLen, float* d, std::string name )
    {
      // title of the sample
      waveformName = name;
      
      // the number of samples in the original file
      realLength = realLen;
      
      // number of pixel points, and the data*
      data_size = numDataPoints;
      data = d;
      
      newWaveform = true;
      redraw();
    }
    
    void draw()
    {
      if (damage() & FL_DAMAGE_ALL)
      {
        cairo_t *cr = Fl::cairo_cc();
        
        //Cairo::Context cr = Cairo::Context( c_cr );
        //Cairo::Context cairoContext = Context( c_cr, false );
        //Cairo::Context* cr = &cairoContext;
        
        cairo_save(cr);
        
        // clear the surface
        cairo_rectangle(cr, x, y, w, h);
        cairo_set_source_rgb (cr, 0.1,0.1,0.1);
        cairo_fill( cr );
        
        
        //cout << "waveform have data" << endl;
        
        if ( newWaveform )
        {
          if ( !waveformCr )
          {
            // create the waveform surface and context
            //cout << "waveform draw() creating new objects" << endl;
            waveformSurf= cairo_image_surface_create ( CAIRO_FORMAT_ARGB32, w, h);
            waveformCr  = cairo_create ( waveformSurf );
            //cout << "waveform draw() creating objects done" << endl;
          }
          
          // clear the surface
          cairo_rectangle(waveformCr, 0, 0, w, h);
          cairo_set_source_rgb (waveformCr, 0.1,0.1,0.1);
          cairo_fill( waveformCr );
          
          // set up dashed lines, 1 px off, 1 px on
          double dashes[1];
          dashes[0] = 2.0;
          
          cairo_set_dash ( waveformCr, dashes, 1, 0.0);
          cairo_set_line_width( waveformCr, 1.0);
          
          // loop over each 2nd line, drawing dots
          cairo_set_line_width(waveformCr, 1.0);
          cairo_set_source_rgb(waveformCr, 0.4,0.4,0.4);
          for ( int i = 1; i < 4; i++ )
          {
            cairo_move_to( waveformCr, ((w / 4.f)*i), 0);
            cairo_line_to( waveformCr, ((w / 4.f)*i), h );
          }
          for ( int i = 1; i < 4; i++ )
          {
            cairo_move_to( waveformCr, 0, ((h / 4.f)*i) );
            cairo_line_to( waveformCr, w, ((h / 4.f)*i) );
          }
          
          cairo_set_source_rgba( waveformCr,  66 / 255.f,  66 / 255.f ,  66 / 255.f , 0.5 );
          cairo_stroke(waveformCr);
          cairo_set_dash ( waveformCr, dashes, 0, 0.0);
          
          
          if ( !data )
          {
            // draw X
            cairo_move_to( waveformCr,  0, 0 );
            cairo_line_to( waveformCr,  w, h );
            cairo_move_to( waveformCr,  0, h );
            cairo_line_to( waveformCr,  w, 0 );
            cairo_set_source_rgba( waveformCr,  66 / 255.f,  66 / 255.f ,  66 / 255.f , 0.5 );
            cairo_stroke(waveformCr);
            
            // draw text
            cairo_move_to( waveformCr,  (w/2.f) - 65, (h/2.f) + 10 );
            cairo_set_source_rgb ( waveformCr, 0.6,0.6,0.6);
            cairo_set_font_size( waveformCr, 20 );
            cairo_show_text( waveformCr, "Load A Sample" );
          }
          else
          {
            // don't draw every sample
            int sampleCountForDrawing = -1;
            
            float currentTop = 0.f;
            float previousTop = 0.f;
            float currentSample = 0.f;
            
            // find how many samples per pixel
            int samplesPerPix = data_size / w;
            //cout << "width = " << w << "  sampsPerPx " << samplesPerPix << endl;
            
            // loop over each pixel value we need
            for( int p = 0; p < w; p++ )
            {
              float average = 0.f;
              
              // calc value for this pixel
              for( int i = 0; i < samplesPerPix; i++ )
              {
                float tmp = data[i + (p * samplesPerPix)];
                if ( tmp < 0 )
                {
                  tmp = -tmp;
                }
                average += tmp;
              }
              average =(average / samplesPerPix);
              
              cairo_move_to( waveformCr, p, (h/2) - (average * (h/2.2f) )  );
              cairo_line_to( waveformCr, p, (h/2) + (average * (h/2.2f) )  );
            }
            
            // stroke the waveform
            cairo_set_source_rgb( waveformCr, 0.8,0.8,0.8);
            cairo_stroke( waveformCr );
            
            /*
            // draw text
            cairo_move_to( waveformCr,  8, h-10 );
            cairo_set_source_rgb ( waveformCr, 0.6,0.6,0.6);
            cairo_set_font_size( waveformCr, 10 );
            std::stringstream s;
            s << "Length: " << realLength;
            cairo_show_text( waveformCr, s.str().c_str() );
            */
            
            // draw sample name
            cairo_move_to( waveformCr,  8, h-10 );
            cairo_set_source_rgb ( waveformCr, 0.6,0.6,0.6);
            cairo_set_font_size( waveformCr, 10 );
            std::stringstream s;
            s << "Sample: " << waveformName;
            cairo_show_text( waveformCr, s.str().c_str() );
            
          }
          
          newWaveform = false;
        }
        
        cairo_set_source_surface(cr, waveformSurf, x, y);
        cairo_rectangle( cr, x, y, w, h);
        cairo_paint(cr);
        
        // stroke rim
        cairo_set_line_width(cr, 0.9);
        cairo_rectangle(cr, x, y, w, h);
        cairo_set_source_rgba( cr,  126 / 255.f,  126 / 255.f ,  126 / 255.f , 0.8 );
        cairo_stroke( cr );
        
        //cout << "waveform draw() done" << endl;
        
        cairo_restore(cr);
      }
    }
    
    void resize(int X, int Y, int W, int H)
    {
      Fl_Widget::resize(X,Y,W,H);
      x = X;
      y = Y;
      w = W;
      h = H;
      redraw();
    }
    
    int handle(int event)
    {
      return 0;
      
      switch(event)
      {
        case FL_PUSH:
          highlight = 0;
          redraw();
          return 1;
        case FL_DRAG: {
            int t = Fl::event_inside(this);
            if (t != highlight) {
              redraw();
            }
          }
          return 1;
        case FL_RELEASE:
          if (highlight) {
            highlight = 0;
            redraw();
            do_callback();
          }
          return 1;
        case FL_SHORTCUT:
          if ( test_shortcut() )
          {
            do_callback();
            return 1;
          }
          return 0;
        default:
          return Fl_Widget::handle(event);
      }
    }
};

} // Avtk

#endif


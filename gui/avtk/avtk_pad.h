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


#ifndef AVTK_PAD_H
#define AVTK_PAD_H


#include "avtk_helpers.h"

#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <valarray>
#include <string>

namespace Avtk
{
  
class Pad : public Fl_Box
{
  public:
    Pad(int _x, int _y, int _w, int _h, const char *_label = 0):
        Fl_Box(_x, _y, _w, _h, _label)
    {
      x = _x;
      y = _y;
      w = _w;
      h = _h;
      
      label = _label;
      
      mouseClickedX = 0;
      mouseClickedY = 0;
      mouseClicked = false;
      mouseRightClicked = false;
      
      s = false;
      p = false;
    }
    
    
    
    void selected(bool sb){s = sb; redraw();}
    void play    (bool pb){p = pb; redraw();}
    void ID(int i){id = i;}
    int  ID(){return id;}
    
    bool s; // selected
    bool p; // playing
    int x, y, w, h;
    const char* label;
    
    int id;
    int mouseClickedX;
    int mouseClickedY;
    bool mouseClicked;
    bool mouseRightClicked;
    
    void draw()
    {
      if (damage() & FL_DAMAGE_ALL)
      {
        cairo_t *cr = Fl::cairo_cc();
        
        cairo_save( cr );
        
        cairo_set_line_width(cr, 1.0);
        
        // fill background
        cairo_rectangle( cr, x, y, w, h);
        cairo_set_source_rgb( cr, 28 / 255.f,  28 / 255.f ,  28 / 255.f  );
        
        if ( p ) // playing
        {
          cairo_fill_preserve( cr );
          cairo_set_source_rgba( cr, 0 / 255.f, 153 / 255.f , 255 / 255.f , 0.21 );
        }
        cairo_fill( cr );
        
        // outline
        cairo_rectangle( cr, x+2, y+2, w-4, h-4);
        
        if ( p ) // selected?
        {
          cairo_set_source_rgba( cr, 0 / 255.f, 153 / 255.f , 255 / 255.f , 0.8 );
          cairo_set_line_width(cr, 3.0);
        }
        else if ( s ) // selected?
        {
          cairo_set_source_rgba( cr,  255 / 255.f,  104 / 255.f ,    0 / 255.f , 0.8 );
          cairo_set_line_width(cr, 3.0);
        }
        else
        {
          //cairo_set_source_rgba( cr,  126 / 255.f,  126 / 255.f ,  126 / 255.f , 0.8 );
          cairo_set_source_rgba( cr,  0 / 255.f,  0 / 255.f ,  0 / 255.f , 0.8 );
          cairo_set_line_width(cr, 2.0);
        }
        cairo_stroke( cr );
        
        cairo_restore( cr );
        
        draw_label();
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
      switch(event)
      {
        case FL_PUSH:
          p = true;
          if ( Fl::event_state(FL_BUTTON1) )
            mouseClicked = true;
          if ( Fl::event_state(FL_BUTTON3) )
            mouseRightClicked = true;
          damage(FL_DAMAGE_ALL);
          redraw();
          do_callback();
          return 1;
        case FL_DRAG:
          {
            /*
            if ( Fl::event_state(FL_BUTTON1) )
            {
              if ( mouseClicked == false ) // catch the "click" event
              {
                mouseClickedX = Fl::event_x();
                mouseClickedY = Fl::event_y();
                mouseClicked = true;
              }
              
              float deltaY = mouseClickedY - Fl::event_y();
              
              float valY = value();
              valY += deltaY / h;
              
              if ( valY > 1.0 ) valY = 1.0;
              if ( valY < 0.0 ) valY = 0.0;
              
              //handle_drag( value + deltaY );
              set_value( valY );
              
              mouseClickedX = Fl::event_x();
              mouseClickedY = Fl::event_y();
              redraw();
              do_callback();
            }
            */
          }
          return 1;
        case FL_RELEASE:
          p = false;
          redraw();
          mouseClicked = false;
          mouseRightClicked = false;
          //do_callback();
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
    
  private:
};

} // Avtk

#endif // AVTK_PAD_H

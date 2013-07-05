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


#ifndef AVTK_OPENGL_H
#define AVTK_OPENGL_H

#include <cmath>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

namespace Avtk
{

class OpenGL : public Fl_Gl_Window
{
  public:
    OpenGL(int _x,int _y,int _w,int _h,const char *l=0):
        Fl_Gl_Window(_x,_y,_w,_h,l)
    {
      overlay_sides = 3;
      offset = 0;
      damage(FL_DAMAGE_ALL);
      
      x = _x;
      y = _y;
      width = _w;
      height = _h;
      
    }
    
    int x, y, width, height;
    
    int overlay_sides;
    float offset;
    
    static void static_update(void* inst)
    {
      OpenGL* instance = (OpenGL*)inst;
      instance->offset += 0.07;
      
      Fl::repeat_timeout( 1 / 60.f, &OpenGL::static_update, inst);
      
      instance->damage(FL_DAMAGE_ALL);
    }
    
    void draw()
    {
      // the valid() property may be used to avoid reinitializing your
      // GL transformation for each redraw:
      if (!valid())
      {
        valid(1);
        glLoadIdentity();
        glViewport(0,0,w(),h());
      }
      
      glClear(GL_COLOR_BUFFER_BIT);
      
      
      glEnable (GL_BLEND);
      //glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);
      glEnable(GL_POLYGON_SMOOTH);
      
      glBegin(GL_POLYGON);
        double ang = 0*2*M_PI/3 + offset;
        glColor3f(1.0,0.48,0);
        glVertex3f(cos(ang)*0.7,sin(ang)*0.7,0);
      
        ang = 1*2*M_PI/3 + offset;
        glColor3f(0.0,0.6,1.0);
        glVertex3f(cos(ang)*0.7,sin(ang)*0.7,0);
        
        ang = 2*2*M_PI/3 + offset;
        glColor3f(0.1,1.0,0.1);
        glVertex3f(cos(ang)*0.7,sin(ang)*0.7,0);
      glEnd();
      
    }
};

} // Avtk

#endif

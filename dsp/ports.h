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

#ifndef FABLA_PORTS_H
#define FABLA_PORTS_H

// Plugin's URI
#define FABLA_URI    "http://www.openavproductions.com/fabla"
#define FABLA_UI_URI "http://www.openavproductions.com/fabla/gui"

typedef enum {
  ATOM_IN= 0,
  ATOM_OUT,
  
  AUDIO_OUT_L,
  AUDIO_OUT_R,
  
  MASTER_VOL,
  
  COMP_ATTACK,
  COMP_DECAY,
  COMP_THRES,
  COMP_RATIO,
  COMP_MAKEUP,
  COMP_ENABLE,
  
  // p*2... is to place hold that index for the functionality before.
  // Actual interaction with the value is by comparison of "port" variable
  // reading:
  //    if ( port > PAD_GAIN && port < PAD_GAIN + 16 )
  // writing:
  //    write( int(PAD_GAIN) + padNumber, &value );
  
  PAD_GAIN,
  pg2,pg3,pg4,pg5,pg6,pg7,pg8,pg9,pg10,pg11,pg12,pg13,pg14,pg15,pg16,
  
  PAD_SPEED,
  pspd2,pspd3,pspd4,pspd5,pspd6,pspd7,pspd8,pspd9,pspd10,pspd11,pspd12,pspd13,pspd14,pspd15,pspd16,
  
  PAD_PAN,
  pp2,pp3,pp4,pp5,pp6,pp7,pp8,pp9,pp10,pp11,pp12,pp13,pp14,pp15,pp16,
  
  
  PAD_ATTACK,
  pa2,pa3,pa4,pa5,pa6,pa7,pa8,pa9,pa10,pa11,pa12,pa13,pa14,pa15,pa16,
  
  PAD_DECAY,
  pd2,pd3,pd4,pd5,pd6,pd7,pd8,pd9,pd10,pd11,pd12,pd13,pd14,pd15,pd16,
  
  PAD_SUSTAIN,
  ps2,ps3,ps4,ps5,ps6,ps7,ps8,ps9,ps10,ps11,ps12,ps13,ps14,ps15,ps16,
  
  PAD_RELEASE,
  pr2,pr3,pr4,pr5,pr6,pr7,pr8,pr9,pr10,pr11,pr12,pr13,pr14,pr15,pr16,

  TRACKSTART,
} PortIndex;


#endif // FABLA_PORTS_H


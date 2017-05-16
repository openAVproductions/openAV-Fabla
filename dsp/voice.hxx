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

#ifndef FABLA_VOICE_H
#define FABLA_VOICE_H

#include "adsr.hxx"
#include "denormals.hxx"

#include <sndfile.h>

#include "sample.hxx"

class Voice
{
  public:
    Voice(int rate)
    {
      sr = rate;
      playingBool = false;
      
      sample = 0;
      index = 0;
      banished = 0;
      
      sampleCountdown = 0;
      sampleCountdownQueued = false;
      
      // center the pan. 0 = left, 1 = right
      pan = 0.5;
      
      adsr = new ADSR( sr, 0.1, 0, 1.0, 0.1 );
    }
    
    Sample* sample;
    ADSR* adsr;
    int banished;
    
    void setAdsr(float a, float d, float s, float r)
    {
      adsr->attack ( a );
      adsr->decay  ( d );
      adsr->sustain( s );
      adsr->release( r );
    }
    
    void setPan( float p )
    {
      pan = p;
    }
    
    void play(int inNote, int vel, int sampleCD)
    {
      banished = 0;
      // if the voice doesn't have a sample: then don't play
      if ( sample )
      {
        adsr->trigger();
        
        note = inNote;
        
        // this will trigger the note on at the right nframe
        sampleCountdown = sampleCD;
        sampleCountdownQueued = true;
        
        index = 0;
      }
    }
    
    
    bool playing()
    {
      // not currently playing, and not queued to play
      if ( playingBool || sampleCountdownQueued )
      {
        return true;
      }
      
      return false;
    }

    void banishIfPlaying(int n)
    {
      if ( note == n ) {
		banished = 1;
      }
    }
    
    void stopIfNoteEquals(int n)
    {
      if ( note == n )
      {
        //printf("releasing note %i",n);
        adsr->release();
        playingBool = false;
      }
    }
    
    /// volume per voice of sample
    void setVolume( float v )
    {
      vol = v * 2;
    }
    
    
    void process( int nframes, float* bufL, float* bufR, bool nosum = false )
    {
	if(banished)
		return;
      // counts down frames until note on
      sampleCountdown--;
      
      if ( sampleCountdownQueued && sampleCountdown <= 0 )
      {
        playingBool = true;
        // now that we're playing, disable queued flag
        sampleCountdownQueued = false;
      }
      
      if( playingBool && sample )
      {
        // linearly interpolate between samples
        float x0 = index - int(index);
        int x1 = int(index);
        int x2 = x1 + 1;
        float y1 = sample->data[x1];
        float y2 = sample->data[x2];
        
        float out = y1 + ( y2 - y1 ) * x0;
        
        // vol is sample gain * midi velocity
        float tmp = out * vol *  adsr->process(nframes);
        
        // sin / cos based amplitude panning
        float panL = cos(pan * 3.14/2.f);
        float panR = sin(pan * 3.14/2.f);
        
        if(nosum)
        {
            *bufL = tmp * panL;
            *bufR = tmp * panR;
        }
        else
        {
            *bufL += tmp * panL;
            *bufR += tmp * panR;
        }
        
        // pitch up twice the range
        float increment = 0.5 + sample->speed;
        if ( increment > 1.0001f )
          increment = 1.0f + (increment-1.0f)*2;
        
        index += increment;
        
        if ( index >= sample->info.frames )
        {
          index = 0;
          playingBool = false;
        }
        
        if ( adsr->finished() )
        {
          // turn off voice if ADSR has finished
          playingBool = false;
        }
        
        //return accum * adsr->process(1);
      }
    }
  
  private:
    int sr;
    int note;
    bool playingBool;
    
    // each voice has own index, allowing for voices playing the same
    // pad, without the "speedup" effect of incrementing the same index
    float index;
    
    // counts down frames until note on event
    bool  sampleCountdownQueued;
    float sampleCountdown;
    
    // vol & pan per voice
    float pan;
    float vol;
  public:
    int getNote() { return note; }
    bool getIsPlaying() { return playingBool; }
};

#endif // FABLA_VOICE_H


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
      
      // center the pan. 0 = left, 1 = right
      pan = 0.5;
      
      adsr = new ADSR( sr, 0.1, 0, 1.0, 0.1 );
    }
    
    Sample* sample;
    ADSR* adsr;
    
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
    
    void play(int inNote, int vel)
    {
      // if the voice doesn't have a sample: then don't play
      if ( sample )
      {
        adsr->trigger();
        
        playingBool = true;
        note = inNote;
        
        index = 0;
      }
    }
    
    
    bool playing(){return playingBool;}
    
    void stopIfNoteEquals(int n)
    {
      if ( note == n )
      {
        printf("releasing note %i",n);
        adsr->release();
        playingBool = false;
      }
    }
    
    
    
    void process( int nframes, float* bufL, float* bufR )
    {
      if( playingBool && sample )
      {
        // linearly interpolate between samples
        float x0 = index - int(index);
        int x1 = int(index);
        int x2 = x1 + 1;
        float y1 = sample->data[x1];
        float y2 = sample->data[x2];
        
        float out = y1 + ( y2 - y1 ) * x0;
        
        float tmp = out * sample->gain *  adsr->process(nframes);
        
        // sin / cos based amplitude panning
        float panL = cos(pan * 3.14/2.f);
        float panR = sin(pan * 3.14/2.f);
        
        *bufL += tmp * panL;
        *bufR += tmp * panR;
        
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
    
    // pan per voice
    float pan;
};

#endif // FABLA_VOICE_H

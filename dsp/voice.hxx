
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
      playingBool = 0;
      
      sample = 0;
      
      index = 0;
      
      adsr = new ADSR( sr, 0.1, 0, 1.0, 0.1 );
    }
    
    Sample* sample;
    ADSR* adsr;
    
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
        float tmp = sample->data[index] * sample->gain;
        *bufL += tmp;
        *bufR += tmp;
        index += sample->speed;
        
        if ( index >= sample->info.frames )
        {
          index = 0;
          playingBool = false;
        }
        
        
        adsr->process(nframes);
        
        if ( adsr->finished() )
        {
          // turn off voice if ADSR has finished
          //playingBool = false;
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
    size_t  index;
};

#endif // FABLA_VOICE_H

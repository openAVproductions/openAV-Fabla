
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
      
      adsr = new ADSR( sr, 200, 200, 0.7, 300 );
    }
    
    Sample* sample;
    ADSR* adsr;
    
    void play(int inNote, int vel)
    {
      playingBool = true;
      note = inNote;
      adsr->trigger();
      
      
      if ( sample )
      {
        sample->index = 0;
        printf("sample index = %i\n sample size = %i\n", int(sample->index), int(sample->info.frames) );
        printf("Loaded sample:\n\t %i samples\n\tdata = %i", sample->info.frames, sample->data);
      }
    }
    
    
    bool playing(){return playingBool;}
    
    void stopIfNoteEquals(int n)
    {
      if ( note == n )
      {
        adsr->release();
      }
    }
    
    
    
    void process( int nframes, float* bufL, float* bufR )
    {
      if( playingBool && sample )
      {
        *bufL += sample->data[sample->index];
        *bufR += sample->data[sample->index];
        sample->index++;
        
        if ( sample->index >= sample->info.frames )
        {
          sample->index = 0;
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
};

#endif // FABLA_VOICE_H

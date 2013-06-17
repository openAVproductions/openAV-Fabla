
#ifndef FABLA_VOICE_H
#define FABLA_VOICE_H

#include "adsr.hxx"
#include "denormals.hxx"

#include <sndfile.h>

class Sample
{
  public:
    Sample()
    {
      index = 0;
      speed = 1.f;
      
      data = 0;
      path = 0;
      path_len = 0;
    }
  
  SF_INFO info;      // Info about sample from sndfile
  float*  data;      // Sample data in float
  char*   path;      // Path of file
  size_t  path_len;  // Length of path
  
  size_t  index;     // Current playback index
  float   speed;     // Current playback speed
};

class Voice
{
  public:
    Voice(int rate)
    {
      sr = rate;
      playingBool = 0;
      
      adsr = new ADSR( sr, 200, 200, 0.7, 300 );
    }
    
    Sample* sample;
    ADSR* adsr;
    
    void play(int inNote, int vel)
    {
      playingBool = true;
      note = inNote;
      adsr->trigger();
      
      printf("sample->data %i\n",sample->data[0]);
      printf("sample index = %i\n sample size = %i\n", int(sample->index), int(sample->info.frames) );
      printf("Loaded sample:\n\t %i samples\n\tdata = %i", sample->info.frames, sample->data);
      
      
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
      if( playingBool && sample && sample->data != 0 )
      {
        /*
        */
        
        bufL[0] = sample->data[sample->index % sample->info.frames];
        bufR[0] = sample->data[sample->index % sample->info.frames];
        sample->index++;
        
        
        adsr->process(nframes);
        
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
};

#endif // FABLA_VOICE_H

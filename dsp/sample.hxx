
#ifndef FABLA_SAMPLE_H
#define FABLA_SAMPLE_H

class Sample
{
  public:
    Sample()
    {
      data = 0;
      path = 0;
      path_len = 0;
    }
    
    SF_INFO info;      // Info about sample from sndfile
    float*  data;      // Sample data in float
    char*   path;      // Path of file
    size_t  path_len;  // Length of path
};

#endif // FABLA_SAMPLE_H

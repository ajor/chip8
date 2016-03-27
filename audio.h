#ifndef AUDIO_H
#define AUDIO_H

#include <AL/al.h>
#include <AL/alc.h>

class Audio
{
  static const int bufferSize = 200;

  ALCdevice *dev;
  ALCcontext *ctx;
  ALuint source, buffer;

public:
  Audio();
  ~Audio();
  void play();
  void stop();
};

#endif

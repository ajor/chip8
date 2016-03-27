#include "audio.h"

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <AL/al.h>
#include <AL/alc.h>

Audio::Audio()
{
  dev = alcOpenDevice(NULL);
  if (!dev)
  {
    fprintf(stderr, "no device\n");
    return;
  }

  ctx = alcCreateContext(dev, NULL);
  alcMakeContextCurrent(ctx);
  if (!ctx)
  {
    fprintf(stderr, "no context\n");
    return;
  }

  int16_t buf[bufferSize];

  for (int i=0; i<bufferSize; i++)
  {
    buf[i] = 0x7fff*sin(2*M_PI*((double)i/(double)bufferSize));
  }

  alGenBuffers(1, &buffer);
  alGenSources(1, &source);
  if (alGetError() != AL_NO_ERROR)
  {
    fprintf(stderr, "error generating buffers\n");
    return;
  }

  alBufferData(buffer, AL_FORMAT_MONO16, buf, bufferSize, 44100);

  alSourcei(source, AL_BUFFER, buffer);
  alSourcei(source, AL_LOOPING, AL_TRUE);
}

Audio::~Audio()
{
  alDeleteSources(1, &source);
  alDeleteBuffers(1, &buffer);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(ctx);
  alcCloseDevice(dev);
}

void Audio::play()
{
  alSourcePlay(source);
}

void Audio::stop()
{
  alSourcePause(source);
}

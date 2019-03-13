#ifndef FFMPEG_OUTPUT_H
#define FFMPEG_OUTPUT_H

#include <jni.h>
#include <media/AudioTrack.h>
#include <media/AudioSystem.h>
#include "include/audiotrack.h"
#include "include/surface.h"

#include <media/AudioTrack.h>

namespace android {

class AudioTrack;

class Output
{
public:	
	Output();
	~Output();

     int					AudioDriver_register();
     int					AudioDriver_set(int streamType,
												uint32_t sampleRate,
												int format,
												int channels);
     int					AudioDriver_start();
     int					AudioDriver_flush();
	 int					AudioDriver_stop();
     int					AudioDriver_reload();
	 int					AudioDriver_write(void *buffer, int buffer_size);
	 int					AudioDriver_unregister();
	
	 int					VideoDriver_register(JNIEnv* env, jobject jsurface);
     int					VideoDriver_getPixels(int width, int height, void** pixels);
     int					VideoDriver_updateSurface();
     int					VideoDriver_unregister();

private:

    sp <AudioTrack> mAudioTrack;

};
}
#endif //FFMPEG_DECODER_H

/*
 * Aladdin.Zhong
 * Version 1.0
 * Date 2015-06-09
 * To playback media files can not decoder by hardware(eg. APE)
*/

#include <android/log.h>
#include "output.h"

#define TAG "FFMpegOutput"

//-------------------- Audio driver --------------------
namespace android {

Output::Output()
{
    mAudioTrack = NULL;
}
Output::~Output()
{
}

int Output::AudioDriver_register()
{

    return 0;
}

int Output::AudioDriver_unregister()
{
    return 0;
}

int Output::AudioDriver_start()
{
    if(mAudioTrack != NULL)  return mAudioTrack->start();
    return 0;
}

int Output::AudioDriver_set(int streamType,
							uint32_t sampleRate,
							int format,
							int channels)
{
	if(mAudioTrack == NULL)
        mAudioTrack = new AudioTrack((audio_stream_type_t)streamType, sampleRate, (audio_format_t)format, channels);

    return 0;
}

int Output::AudioDriver_flush()
{
    if(mAudioTrack != NULL) mAudioTrack->flush();
    return 0;
}

int Output::AudioDriver_stop()
{
    if(mAudioTrack != NULL) mAudioTrack->stop();
    return 0;
}

int Output::AudioDriver_reload()
{
    if(mAudioTrack != NULL) return mAudioTrack->reload();
    return 0;
}

int Output::AudioDriver_write(void *buffer, int buffer_size)
{
    if(mAudioTrack != NULL) return mAudioTrack->write(buffer,buffer_size);
    return 0;
}

//-------------------- Video driver --------------------

int Output::VideoDriver_register(JNIEnv* env, jobject jsurface)
{
    return 0;
}

int Output::VideoDriver_unregister()
{
    return 0;
}

int Output::VideoDriver_getPixels(int width, int height, void** pixels)
{
    return 0;
}

int Output::VideoDriver_updateSurface()
{
    return 0;
}

}

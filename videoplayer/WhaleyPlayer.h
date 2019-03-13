
#ifndef ANDROID_WHALEYPLAYER_H
#define ANDROID_WHALEYPLAYER_H

#include <media/MediaPlayerInterface.h>

#include "ffplayer.h"
#include "cueplayer.h"

namespace android {

class WhaleyPlayer : public MediaPlayerInterface {
public:
    WhaleyPlayer();
    virtual ~WhaleyPlayer();

    virtual status_t initCheck();

    virtual status_t setUID(uid_t uid);

    virtual status_t setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char *url,
            const KeyedVector<String8, String8> *headers);

    virtual status_t setDataSource(int fd, int64_t offset, int64_t length);

    virtual status_t setDataSource(const sp<IStreamSource> &source);

    virtual status_t setVideoSurfaceTexture(
            const sp<IGraphicBufferProducer> &bufferProducer);
    virtual status_t prepare();
    virtual status_t prepareAsync();
    virtual status_t start();
    virtual status_t stop();
    virtual status_t pause();
    virtual bool isPlaying();
    virtual status_t seekTo(int msec);
    virtual status_t getCurrentPosition(int *msec);
    virtual status_t getDuration(int *msec);
    virtual status_t reset();
    virtual status_t setLooping(int loop);
    virtual player_type playerType() { return WHALEY_PLAYER; }
    virtual status_t invoke(const Parcel &request, Parcel *reply);
    virtual void setAudioSink(const sp<AudioSink> &audioSink);
    virtual status_t setParameter(int key, const Parcel &request);
    virtual status_t getParameter(int key, Parcel *reply);

    virtual status_t getMetadata(
            const media::Metadata::Filter& ids, Parcel *records);

    virtual status_t dump(int fd, const Vector<String16> &args) const;

private:
    FFPlayer *ffPlayer;
    CuePlayer *cuePlayer;

    bool      isCueFile;

    WhaleyPlayer(const WhaleyPlayer &);
    WhaleyPlayer &operator=(const WhaleyPlayer &);
};

}  // namespace android

#endif  // ANDROID_WHALEYPLAYER_H

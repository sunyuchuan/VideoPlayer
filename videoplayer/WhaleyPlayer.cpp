/*
 * Aladdin.Zhong
 * Version 1.0
 * Date 2015-06-09
 * To playback media files can not decoder by hardware(eg. APE)
*/
//#define LOG_NDEBUG 0
#define LOG_TAG "WhaleyPlayer"
#include <utils/Log.h>

#include "WhaleyPlayer.h"

#include <media/Metadata.h>

#define HMP_DBG   ALOGI

namespace android {

WhaleyPlayer::WhaleyPlayer()
    : ffPlayer(new FFPlayer()),
    cuePlayer(new CuePlayer()),
    isCueFile(false){
    HMP_DBG("[HMP]WhaleyPlayer");
    if(ffPlayer != NULL){
       if(ffPlayer->setListener(this) != NO_ERROR)
           HMP_DBG("[HMP]ffplayer set setListener fail");
    }
    if(cuePlayer != NULL){
       if(cuePlayer->setListener(this) != NO_ERROR)
           HMP_DBG("[HMP]cueplayer set setListener fail");
    }
}

WhaleyPlayer::~WhaleyPlayer() {
    HMP_DBG("[HMP]~WhaleyPlayer");
    reset();

    delete ffPlayer;
    ffPlayer = NULL;

    delete cuePlayer;
    cuePlayer = NULL;
}


status_t WhaleyPlayer::initCheck() {
    HMP_DBG("initCheck");

    if(ffPlayer != NULL)    return ffPlayer->initCheck();

    return NO_ERROR;
}

status_t WhaleyPlayer::setUID(uid_t uid) {
    HMP_DBG("[HMP]setUID %d",uid);

    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->setUID(uid);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->setUID(uid);
    }

    return INVALID_OPERATION;
}

status_t WhaleyPlayer::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers) {
    HMP_DBG("[HMP]setDataSource (%s)",url);

    //Temp method: use CuePlayer playback Ape
    if (strcmp(url+strlen(url)-4, ".cue") == 0) {
        HMP_DBG("[HMP]This is CUE file and go CuePlayer!");
        isCueFile = true;
        delete ffPlayer;
        ffPlayer = NULL;

        if(cuePlayer != NULL)
            return cuePlayer->setDataSource(strdup(url), headers);
    }else{
        isCueFile = false;
        delete cuePlayer;
        cuePlayer = NULL;

        if(ffPlayer != NULL)
            return ffPlayer->setDataSource(strdup(url), headers);
    }

    return INVALID_OPERATION;
}

// Warning: The filedescriptor passed into this method will only be valid until
// the method returns, if you want to keep it, dup it!
status_t WhaleyPlayer::setDataSource(int fd, int64_t offset, int64_t length) {
    HMP_DBG("[HMP]setDataSource(%d, %lld, %lld)", fd, offset, length);

    char s[256], name[256];
    memset(s,0,sizeof(s));
    memset(name,0,sizeof(s));
    snprintf(s, 255, "/proc/%d/fd/%d", getpid(), fd);
    readlink(s, name, 255);
    //close(fd);

    //Temp method: use CuePlayer playback Ape
    if (strcmp(name+strlen(name)-4, ".cue") == 0) {
        HMP_DBG("[HMP]This is CUE file and go CuePlayer!");
        isCueFile = true;
        delete ffPlayer;
        ffPlayer = NULL;

        if(cuePlayer != NULL)
            return cuePlayer->setDataSource(strdup(name), NULL);
    }else{
        isCueFile = false;
        delete cuePlayer;
        cuePlayer = NULL;

        if(ffPlayer != NULL)
            return ffPlayer->setDataSource(strdup(name), NULL);
    }

    return INVALID_OPERATION;
}

status_t WhaleyPlayer::setDataSource(const sp<IStreamSource> &source) {
    HMP_DBG("[HMP]setDataSource");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->setDataSource(source);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->setDataSource(source);
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::setVideoSurfaceTexture(
        const sp<IGraphicBufferProducer> &bufferProducer) {
    HMP_DBG("[HMP]setVideoSurfaceTexture");
    if(ffPlayer != NULL)
        return ffPlayer->setVideoSurfaceTexture(bufferProducer);
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::prepare() {
    HMP_DBG("[HMP]prepare");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->prepare();
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->prepare();
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::prepareAsync() {
    HMP_DBG("[HMP]prepareAsync");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->prepareAsync();
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->prepareAsync();
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::start() {
    ALOGV("[HMP]start");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->start();
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->start();
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::stop() {
    HMP_DBG("[HMP]stop");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->stop();
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->stop();
    }
    return INVALID_OPERATION;  // what's the difference?
}

status_t WhaleyPlayer::pause() {
    HMP_DBG("[HMP]pause");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->pause();
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->pause();
    }
    return INVALID_OPERATION;
}

bool WhaleyPlayer::isPlaying() {
    //HMP_DBG("[HMP]isPlaying");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->isPlaying();
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->isPlaying();
    }
    return false;
}

status_t WhaleyPlayer::seekTo(int msec) {
    HMP_DBG("[HMP]seekTo %.2f secs", msec / 1E3);
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->seekTo(msec);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->seekTo(msec);
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::getCurrentPosition(int *msec) {
    //HMP_DBG("[HMP]getCurrentPosition");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->getCurrentPosition(msec);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->getCurrentPosition(msec);
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::getDuration(int *msec) {
    //HMP_DBG("[HMP]getDuration");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->getDuration(msec);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->getDuration(msec);
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::reset() {
    HMP_DBG("[HMP]reset");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->reset();
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->reset();
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::setLooping(int loop) {
    HMP_DBG("[HMP]setLooping");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->setLooping(loop);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->setLooping(loop);
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::invoke(const Parcel &request, Parcel *reply) {
    HMP_DBG("[HMP]invoke");
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->invoke(request, reply);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->invoke(request, reply);
    }
    return INVALID_OPERATION;
}

void WhaleyPlayer::setAudioSink(const sp<AudioSink> &audioSink) {
    MediaPlayerInterface::setAudioSink(audioSink);
    HMP_DBG("[HMP]setAudioSink");
}

status_t WhaleyPlayer::setParameter(int key, const Parcel &request) {
    HMP_DBG("[HMP]setParameter(key=%d)", key);
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->setParameter(key, request);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->setParameter(key, request);
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::getParameter(int key, Parcel *reply) {
    HMP_DBG("[HMP]getParameter");
    if(isCueFile){
        if(cuePlayer != NULL)
             return cuePlayer->getParameter(key, reply);
    }else{
        if(ffPlayer != NULL)
             return ffPlayer->getParameter(key, reply);
    }
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::getMetadata(
        const media::Metadata::Filter& /* ids */, Parcel *records) {
    using media::Metadata;

    uint32_t flags = 0;//mPlayer->flags();

    Metadata metadata(records);
#if 0
    metadata.appendBool(
            Metadata::kPauseAvailable,
            flags & MediaExtractor::CAN_PAUSE);

    metadata.appendBool(
            Metadata::kSeekBackwardAvailable,
            flags & MediaExtractor::CAN_SEEK_BACKWARD);

    metadata.appendBool(
            Metadata::kSeekForwardAvailable,
            flags & MediaExtractor::CAN_SEEK_FORWARD);

    metadata.appendBool(
            Metadata::kSeekAvailable,
            flags & MediaExtractor::CAN_SEEK);
#endif
    return INVALID_OPERATION;
}

status_t WhaleyPlayer::dump(int fd, const Vector<String16> &args) const {
    HMP_DBG("[HMP]dump(%d)",fd);
    if(isCueFile){
        if(cuePlayer != NULL)
            return cuePlayer->dump(fd, args);
    }else{
        if(ffPlayer != NULL)
            return ffPlayer->dump(fd, args);
    }
    return INVALID_OPERATION;
}

}  // namespace android

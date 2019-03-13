

//#define LOG_NDEBUG 0
#define LOG_TAG "WhaleyMetadataRetriever"

#include <inttypes.h>

#include <utils/Log.h>

#include "WhaleyMetadataRetriever.h"

#include <media/IMediaHTTPService.h>
//#include <CharacterEncodingDetector.h>

namespace android {

extern "C" {

#include "ff_func.h"

} // end of extern C

WhaleyMetadataRetriever::WhaleyMetadataRetriever()
    : mParsedMetaData(false),
      mFilename(NULL),
      mAlbumArt(NULL),
      mCueParse(NULL),
      isCue(false)
{
    ALOGV("WhaleyMetadataRetriever()");
#ifndef FFMPEG_30
    av_register_all();
#endif

}

WhaleyMetadataRetriever::~WhaleyMetadataRetriever() {
    ALOGV("~WhaleyMetadataRetriever()");

    delete mFilename;
    mFilename = NULL;
    delete mAlbumArt;
    mAlbumArt = NULL;
}

status_t WhaleyMetadataRetriever::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *uri,
        const KeyedVector<String8, String8> *headers) {
    ALOGI("[HMP]setDataSource(%s)", uri);

    mParsedMetaData = false;
    mMetaData.clear();
    delete mAlbumArt;
    mAlbumArt = NULL;

    return OK;
}


// Warning caller retains ownership of the filedescriptor! Dup it if necessary.
status_t WhaleyMetadataRetriever::setDataSource(int fd, int64_t offset, int64_t length) {
    int fd_dup = dup(fd);

    ALOGI("[HMP]setDataSource(%d, %" PRId64 ", %" PRId64 ")", fd, offset, length);

    mParsedMetaData = false;
    mMetaData.clear();
    delete mAlbumArt;
    mAlbumArt = NULL;

    char s[256], name[256];
    memset(s,0,sizeof(s));
    memset(name,0,sizeof(s));
    snprintf(s, 255, "/proc/%d/fd/%d", getpid(), fd_dup);
    readlink(s, name, 255);
    mFilename = strdup(name);
    if (strcmp(name+strlen(name)-4, ".cue") == 0) {
        isCue = true;
        mCueParse = new cueParse();
        if(mCueParse){
            char cuepath[strlen(name)];
            strcpy(cuepath,name);
            mCueParse->parseCueFile(cuepath, &mCueFile, &mCueSongs);
            delete mCueParse;
        }
    }

    close(fd_dup);

    return OK;
}

VideoFrame *WhaleyMetadataRetriever::getFrameAtTime(int64_t timeUs, int option) {

    ALOGV("getFrameAtTime: %" PRId64 " us option: %d", timeUs, option);

    return NULL;
}

MediaAlbumArt *WhaleyMetadataRetriever::extractAlbumArt() {
    ALOGV("extractAlbumArt (extractor: %s)", "NO");

    if (!mParsedMetaData) {
        parseMetaData();

        mParsedMetaData = true;
    }

    if (mAlbumArt) {
        return mAlbumArt->clone();
    }

    return NULL;
}

const char *WhaleyMetadataRetriever::extractMetadata(int keyCode) {

    if (!mParsedMetaData) {
        parseMetaData();

        mParsedMetaData = true;
    }

    ssize_t index = mMetaData.indexOfKey(keyCode);

    if (index < 0) {
        return NULL;
    }

    return mMetaData.valueAt(index).string();
}

void WhaleyMetadataRetriever::parseMetaData() {

#ifndef FFMPEG_30
    const void *data;
    uint32_t type;
    size_t dataSize;
    mMetaData.clear();

    if(isCue){

        return;
    }

    AVFormatContext *fileInfo;
    //av_metadata_get();

    if(ff_open_input_file(&fileInfo, mFilename) != 0) {
        ALOGE("parseMetaData fail!");
        return;
    }

    if(ff_find_stream_info(fileInfo) < 0) {
        ALOGE("parseMetaData fail!");
        ff_close_input_file(fileInfo);
        return;
    }

    char str_tracknum[8];
    sprintf(str_tracknum,"%d",fileInfo->nb_streams);
    mMetaData.add(METADATA_KEY_NUM_TRACKS, String8(str_tracknum));
    mMetaData.add(METADATA_KEY_CD_TRACK_NUMBER, String8("N/A"));
    //mMetaData.add(METADATA_KEY_ARTIST, String8("N/A"));
    mMetaData.add(METADATA_KEY_COMPOSER, String8("N/A"));
    mMetaData.add(METADATA_KEY_WRITER, String8("N/A"));
    //mMetaData.add(METADATA_KEY_MIMETYPE, String8("N/A"));
    mMetaData.add(METADATA_KEY_ALBUMARTIST, String8("N/A"));
    mMetaData.add(METADATA_KEY_DISC_NUMBER, String8("N/A"));
    mMetaData.add(METADATA_KEY_COMPILATION, String8("N/A"));
    mMetaData.add(METADATA_KEY_VIDEO_WIDTH, String8("N/A"));
    mMetaData.add(METADATA_KEY_VIDEO_HEIGHT, String8("N/A"));
    char str_bitrate[8];
    sprintf(str_bitrate,"%d",fileInfo->bit_rate);
    mMetaData.add(METADATA_KEY_BITRATE, String8(str_bitrate));
    mMetaData.add(METADATA_KEY_TIMED_TEXT_LANGUAGES, String8("N/A"));
    mMetaData.add(METADATA_KEY_IS_DRM, String8("N/A"));
    mMetaData.add(METADATA_KEY_LOCATION, String8("N/A"));
    mMetaData.add(METADATA_KEY_VIDEO_ROTATION, String8("N/A"));

    mMetaData.add(METADATA_KEY_AUTHOR, String8(fileInfo->author));
    mMetaData.add(METADATA_KEY_TITLE, String8(fileInfo->title));
    char str_year[8];
    sprintf(str_year,"%d",fileInfo->year);
    mMetaData.add(METADATA_KEY_YEAR, String8(str_year));
    mMetaData.add(METADATA_KEY_GENRE, String8(fileInfo->genre));
    char str_duaration[12];
    sprintf(str_duaration,"%d",fileInfo->duration);
    mMetaData.add(METADATA_KEY_DURATION, String8(str_duaration));
    mMetaData.add(METADATA_KEY_ALBUM, String8(fileInfo->album));
    mMetaData.add(METADATA_KEY_HAS_AUDIO, String8("yes"));
    mMetaData.add(METADATA_KEY_HAS_VIDEO, String8("no"));
    char str_date[12];
    sprintf(str_date,"%d",fileInfo->data_offset);
    mMetaData.add(METADATA_KEY_DATE, String8(str_date));

    ff_close_input_file(fileInfo);
#endif
}

}  // namespace android


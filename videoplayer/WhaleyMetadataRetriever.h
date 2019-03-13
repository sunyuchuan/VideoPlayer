#ifndef WHALEY_METADATA_RETRIEVER_H_

#define WHALEY_METADATA_RETRIEVER_H_

#include <media/MediaMetadataRetrieverInterface.h>

#include "cueparse.h"

namespace android {

///struct DataSource;
class MediaExtractor;

struct WhaleyMetadataRetriever : public MediaMetadataRetrieverInterface {
    WhaleyMetadataRetriever();
    virtual ~WhaleyMetadataRetriever();

    virtual status_t setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char *url,
            const KeyedVector<String8, String8> *headers);

    virtual status_t setDataSource(int fd, int64_t offset, int64_t length);

    virtual VideoFrame *getFrameAtTime(int64_t timeUs, int option);
    virtual MediaAlbumArt *extractAlbumArt();
    virtual const char *extractMetadata(int keyCode);

private:

    bool mParsedMetaData;
    KeyedVector<int, String8> mMetaData;
    MediaAlbumArt *mAlbumArt;
    const char * mFilename;
    bool isCue;
    cueParse *                mCueParse;
    CueFileBean               mCueFile;
    Vector<CueSongBean>       mCueSongs;

    void parseMetaData();

    WhaleyMetadataRetriever(const WhaleyMetadataRetriever &);

    WhaleyMetadataRetriever &operator=( const WhaleyMetadataRetriever &);
};

}  // namespace android

#endif  // WHALEY_METADATA_RETRIEVER_H_

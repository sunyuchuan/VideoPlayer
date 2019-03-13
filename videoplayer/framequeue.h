#ifndef FFMPEG_FRAMEQUEUE_H
#define FFMPEG_FRAMEQUEUE_H

#include <pthread.h>

extern "C" {

#include "ff_func.h"

}

#if 0
#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))
#endif

typedef struct AVFrameList {
    AVFrame *frame;
//    AVSubtitle sub;
//    AVSubtitleRect **subrects;  /* rescaled subtitle rectangles in yuva */
    double pts;           /* presentation timestamp for the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    double clk;
    struct AVFrameList *next;
} AVFrameList;

class FrameQueue
{
public:
    FrameQueue();
    ~FrameQueue();

    void checkQueueFull();
    void checkQueueEmpty();
    void flush();
    int put(AVFrameList *frame);
    int get(AVFrameList **frame, bool block);
    int size();
    void abort();

private:
    AVFrameList*        mFirst;
    AVFrameList*        mLast;
    int                 mNbFrames;
    int                 mSize;
    bool                mAbortRequest;
    int                 rindex;
    int                 windex;
    int                 max_size;
    int                 keep_last;
    int                 rindex_shown;
    pthread_mutex_t     mLock;
    pthread_cond_t      mCondition;
};

#endif

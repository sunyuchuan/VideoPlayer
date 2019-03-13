#define LOG_TAG "FFframequeue"
#include <utils/Log.h>
#include "framequeue.h"

#define FRAME_QUEUE_MAX_SIZE 2
#define FRAME_QUEUE_MIN_SIZE 1

FrameQueue::FrameQueue():
    mFirst(NULL),
    mLast(NULL),
    mNbFrames(0),
    mSize(0),
    mAbortRequest(false),
    rindex(0),
    windex(0),
    max_size(0),
    keep_last(0),
    rindex_shown(0)
{
    ALOGI("[HMP]create FrameQueue\n");
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);
}

FrameQueue::~FrameQueue()
{
    ALOGI("[HMP]destory FrameQueue\n");
    flush();
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondition);
}

int FrameQueue::size()
{
    pthread_mutex_lock(&mLock);
    int size = mNbFrames;
    pthread_mutex_unlock(&mLock);
    return size;
}

void FrameQueue::flush()
{
    AVFrameList *Frame, *FrameNext;

    pthread_mutex_lock(&mLock);

    for(Frame = mFirst; Frame != NULL; Frame = FrameNext) {
        FrameNext = Frame->next;
        avpicture_free((AVPicture *)Frame->frame);
        av_frame_free(&Frame->frame);
        free(Frame);
    }
    mLast = NULL;
    mFirst = NULL;
    mNbFrames = 0;
    mSize = 0;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

void FrameQueue::checkQueueFull()
{
    pthread_mutex_lock(&mLock);
    if(FRAME_QUEUE_MAX_SIZE < mNbFrames && !mAbortRequest)
    {
        //ALOGI("wait because queue is full!\n");
        pthread_cond_wait(&mCondition, &mLock);
    }
    pthread_mutex_unlock(&mLock);
}

void FrameQueue::checkQueueEmpty()
{
    pthread_mutex_lock(&mLock);
    if(FRAME_QUEUE_MAX_SIZE >= mNbFrames)
    {
        //ALOGI("resume because queue is empty!\n");
        pthread_cond_signal(&mCondition);
    }
    pthread_mutex_unlock(&mLock);
}

int FrameQueue::put(AVFrameList* Frame)
{
    //AVFrameList *Frame = NULL;
    //Frame = (AVFrameList *) av_malloc(sizeof(AVFrameList));
    if (!Frame)
        return -1;
    //Frame->frame = frame->frame;
    Frame->next = NULL;

    pthread_mutex_lock(&mLock);

    if (!mLast) {
        mFirst = Frame;
    }
    else {
        mLast->next = Frame;
    }

    mLast = Frame;
    mNbFrames++;
    //mSize += Frame->frame.size + sizeof(*Frame);

    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
    return 0;
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int FrameQueue::get(AVFrameList **frame, bool block)
{
    AVFrameList *Frame;
    int ret;

    pthread_mutex_lock(&mLock);

    for(;;) {
        if (mAbortRequest) {
            ret = -1;
            break;
        }

        Frame = mFirst;
        if (Frame) {
            mFirst = Frame->next;
            if (!mFirst)
                mLast = NULL;
            mNbFrames--;
            //mSize -= Frame->frame.size + sizeof(*Frame);

            *frame = Frame;
            //av_freep(Frame);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            pthread_cond_wait(&mCondition, &mLock);
        }
    }
    pthread_mutex_unlock(&mLock);
    return ret;
}

void FrameQueue::abort()
{
    pthread_mutex_lock(&mLock);
    mAbortRequest = true;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

#if 0
static void frame_queue_unref_item(Frame *vp)
{
    int i;
    for (i = 0; i < vp->sub.num_rects; i++) {
        av_freep(&vp->subrects[i]->data[0]);
        av_freep(&vp->subrects[i]);
    }
    av_freep(&vp->subrects);
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

static int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last)
{
    int i;
    memset(f, 0, sizeof(FrameQueue));
    if (!(f->mutex = SDL_CreateMutex())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    if (!(f->cond = SDL_CreateCond())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = !!keep_last;
    for (i = 0; i < f->max_size; i++)
        if (!(f->queue[i].frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    return 0;
}

static void frame_queue_destory(FrameQueue *f)
{
    int i;
    for (i = 0; i < f->max_size; i++) {
        Frame *vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
        free_picture(vp);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

static void frame_queue_signal(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

static Frame *frame_queue_peek(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

static Frame *frame_queue_peek_next(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

static Frame *frame_queue_peek_last(FrameQueue *f)
{
    return &f->queue[f->rindex];
}

static Frame *frame_queue_peek_writable(FrameQueue *f)
{
    /* wait until we have space to put a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size >= f->max_size &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

    return &f->queue[f->windex];
}

static Frame *frame_queue_peek_readable(FrameQueue *f)
{
    /* wait until we have a readable a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

static void frame_queue_push(FrameQueue *f)
{
    if (++f->windex == f->max_size)
        f->windex = 0;
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

static void frame_queue_next(FrameQueue *f)
{
    if (f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if (++f->rindex == f->max_size)
        f->rindex = 0;
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

/* jump back to the previous frame if available by resetting rindex_shown */
static int frame_queue_prev(FrameQueue *f)
{
    int ret = f->rindex_shown;
    f->rindex_shown = 0;
    return ret;
}

/* return the number of undisplayed frames in the queue */
static int frame_queue_nb_remaining(FrameQueue *f)
{
    return f->size - f->rindex_shown;
}

/* return last shown position */
static int64_t frame_queue_last_pos(FrameQueue *f)
{
    Frame *fp = &f->queue[f->rindex];
    if (f->rindex_shown && fp->serial == f->pktq->serial)
        return fp->pos;
    else
        return -1;
}

#endif

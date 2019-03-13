#include "clock.h"
#include <utils/Log.h>
#include <math.h>

#define LOG_TAG "FFclock"

FFclock::FFclock()
{
    mVideoClock = 0.0;
    mAudioClock = 0.0;
    mAbortRequest = false;
    mSeekFlag = false;
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);
    pthread_cond_init(&mSubCondition, NULL);
}

FFclock::~FFclock()
{
    stop();
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondition);
    pthread_cond_destroy(&mSubCondition);
}

void FFclock::stop()
{
    pthread_mutex_lock(&mLock);
    mAbortRequest = true;
    pthread_cond_signal(&mCondition);
    pthread_cond_signal(&mSubCondition);
    pthread_mutex_unlock(&mLock);
}

double FFclock::getAudioClock()
{
    double ret = 0.0;
    pthread_mutex_lock(&mLock);
    ret = mAudioClock;
    pthread_mutex_unlock(&mLock);
    return ret;
}

double FFclock::getVideoClock()
{
    double ret = 0.0;
    pthread_mutex_lock(&mLock);
    ret = mVideoClock;
    pthread_mutex_unlock(&mLock);
    return ret;
}

void FFclock::getSubClock(double *startclk,double *endclk)
{
    pthread_mutex_lock(&mLock);
    *startclk = mSubStartClock;
    *endclk = mSubEndClock;
    pthread_mutex_unlock(&mLock);
}

void FFclock::setAudioClock(double clk)
{
    pthread_mutex_lock(&mLock);
    mAudioClock = clk;
    pthread_mutex_unlock(&mLock);
}

void FFclock::setVideoClock(double clk)
{
    pthread_mutex_lock(&mLock);
    mVideoClock = clk;
    pthread_mutex_unlock(&mLock);
}

void FFclock::setSubClock(double startclk,double endclk)
{
    pthread_mutex_lock(&mLock);
    mSubStartClock = startclk;
    mSubEndClock = endclk;
    pthread_mutex_unlock(&mLock);
}

void FFclock::syncVideoClock()
{
    pthread_mutex_lock(&mLock);
    double diff = mVideoClock- mAudioClock;
    if(diff > AV_SYNC_THRESHOLD && !mAbortRequest)
    {
        //ALOGI("video is quickly!\n");
        pthread_cond_wait(&mCondition,&mLock);
    }
    pthread_mutex_unlock(&mLock);
}

void FFclock::checkVideoClock()
{
    pthread_mutex_lock(&mLock);
    double diff = mVideoClock- mAudioClock;
    if(diff <= AV_SYNC_THRESHOLD)
    {
        //ALOGI("video is slow!\n");
        pthread_cond_signal(&mCondition);
    }
    pthread_mutex_unlock(&mLock);
}

void FFclock::notify()
{
    pthread_mutex_lock(&mLock);
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

void FFclock::wait()
{
    pthread_mutex_lock(&mLock);
    pthread_cond_wait(&mCondition,&mLock);
    pthread_mutex_unlock(&mLock);
}

void FFclock::subNotify()
{
    pthread_mutex_lock(&mLock);
    pthread_cond_signal(&mSubCondition);
    pthread_mutex_unlock(&mLock);
}

void FFclock::subWait()
{
    pthread_mutex_lock(&mLock);
    pthread_cond_wait(&mSubCondition,&mLock);
    pthread_mutex_unlock(&mLock);
}


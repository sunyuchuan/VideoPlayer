/*
 * Aladdin.Zhong
 * Version 1.0
 * Date 2015-06-09
 * To playback media files can not decoder by hardware(eg. APE)
*/

#include "thread.h"
#include <utils/Log.h>

#define LOG_TAG "FFMpegThread"

FFThread::FFThread()
{
    mPause = false;
    mLastFrame = false;
    mSleeping = false;
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);
}

FFThread::~FFThread()
{
}

void FFThread::start()
{
    handleRun(NULL);
}

void FFThread::startAsync()
{
    pthread_create(&mThread, NULL, startThread, this);
}

int FFThread::wait()
{
    if(!mRunning){
        return 0;
    }else{
        //if(mLastFrame)  mRunning = false;
    }
    return pthread_join(mThread, NULL);
}

void FFThread::stop()
{
}

void* FFThread::startThread(void* ptr)
{
    //ALOGI("starting thread");
    FFThread* thread = (FFThread *) ptr;
    thread->mRunning = true;
    thread->handleRun(ptr);
    thread->mRunning = false;
    //ALOGI("thread ended");
    return NULL; //Modified by Aladdin
}

void FFThread::waitOnNotify()
{
    pthread_mutex_lock(&mLock);
    pthread_cond_wait(&mCondition, &mLock);
    pthread_mutex_unlock(&mLock);
}

void FFThread::notify()
{
    pthread_mutex_lock(&mLock);
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

void FFThread::handleRun(void* ptr)
{
}

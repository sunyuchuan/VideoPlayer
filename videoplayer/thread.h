#ifndef FFMPEG_THREAD_H
#define FFMPEG_THREAD_H

#include <pthread.h>

class FFThread
{
public:
	FFThread();
	virtual ~FFThread(); //Modified : add virtual

	void						start();
    void						startAsync();
    int							wait();

    void 						waitOnNotify();
    void						notify();
    virtual void				stop();
    volatile bool               mPause;
    bool                        mLastFrame;
    bool                        mSleeping;

protected:
    bool						mRunning;

    virtual void                handleRun(void* ptr);
private:
    pthread_t                   mThread;
    pthread_mutex_t     		mLock;
    pthread_cond_t				mCondition;

	static void*				startThread(void* ptr);
};

#endif //FFMPEG_DECODER_H

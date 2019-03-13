#ifndef FFMPEG_CLOCK_H
#define FFMPEG_CLOCK_H

#include <pthread.h>
#define AS_SYNC_THRESHOLD 0.1
#define AV_SYNC_THRESHOLD 0.01

class FFclock
{
public:
    FFclock();
    ~FFclock();

    double getAudioClock();
    double getVideoClock();
    void getSubClock(double *,double *);
    void setAudioClock(double clk);
    void setVideoClock(double clk);
    void setSubClock(double ,double);
    void syncVideoClock();
    void checkVideoClock();
    void stop();
    void notify();
    void wait();
    void subNotify();
    void subWait();
    bool                    mSeekFlag;

private:
    double                  mVideoClock;
    double                  mAudioClock;
    double                  mSubStartClock;
    double                  mSubEndClock;
    pthread_mutex_t         mLock;
    pthread_cond_t          mCondition;
    pthread_cond_t          mSubCondition;
    bool                    mAbortRequest;
};

#endif

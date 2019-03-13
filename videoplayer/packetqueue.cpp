/*
 * Aladdin.Zhong
 * Version 1.0
 * Date 2015-06-09
 * To playback media files can not decoder by hardware(eg. APE)
*/

#define LOG_TAG "FFPktQ"

#include <utils/Log.h>
#include "packetqueue.h"

PacketQueue::PacketQueue()
{
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);
    mFirst = NULL;
    mLast = NULL;
    mNbPackets = 0;
    mSize = 0;
    mAbortRequest = false;
}

PacketQueue::~PacketQueue()
{
    flush();
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondition);
}

int PacketQueue::size()
{
    pthread_mutex_lock(&mLock);
    int size = mNbPackets;
    pthread_mutex_unlock(&mLock);
    return size;
}

void PacketQueue::flush()
{
    AVPacketList *pkt, *pkt1;

    pthread_mutex_lock(&mLock);

    for(pkt = mFirst; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
#ifndef FFMPEG_30
        av_free_packet(&pkt->pkt);
#else
        av_packet_unref(&pkt->pkt);
#endif
        av_freep(&pkt);
    }
    mLast = NULL;
    mFirst = NULL;
    mNbPackets = 0;
    mSize = 0;

    pthread_mutex_unlock(&mLock);
}

int PacketQueue::put(AVPacket* pkt)
{
    //ALOGI("[HMP]***put start! (pkt=%d|size=%d)",mNbPackets,mSize);

    AVPacketList *pkt1;

    /* duplicate the packet */
    if (av_dup_packet(pkt) < 0)
        return -1;

    pkt1 = (AVPacketList *) av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;

    pthread_mutex_lock(&mLock);

    if (!mLast) {
        mFirst = pkt1;
    }
    else {
        mLast->next = pkt1;
    }

    mLast = pkt1;
    mNbPackets++;
    mSize += pkt1->pkt.size + sizeof(*pkt1);

    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
    //ALOGI("[HMP]***put end! (pkt=%d|size=%d)",mNbPackets,mSize);
    return 0;
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int PacketQueue::get(AVPacket *pkt, bool block)
{
    //ALOGI("[HMP]===get start! (pkt=%d|size=%d)",mNbPackets,mSize);
    AVPacketList *pkt1;
    int ret;
	
    pthread_mutex_lock(&mLock);

    for(;;) {
        if (mAbortRequest) {
            ret = -1;
            break;
        }

        pkt1 = mFirst;
        if (pkt1) {
            mFirst = pkt1->next;
            if (!mFirst)
                mLast = NULL;
            mNbPackets--;
            mSize -= pkt1->pkt.size + sizeof(*pkt1);

            *pkt = pkt1->pkt;

            av_freep(&pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            ALOGI("don't get a packet,thread wait\n");
            pthread_cond_wait(&mCondition, &mLock);
            ALOGI("now,thread to be awakened\n");
        }
    }
    pthread_mutex_unlock(&mLock);
    //ALOGI("[HMP]===get end! (pkt=%d|size=%d|ret=%d)",mNbPackets,mSize,ret);
    return ret;
}

bool PacketQueue::isEmpty()
{
    return (mFirst == NULL) ? true : false;
}

bool PacketQueue::getAbortRequest()
{
    bool temp = false;
    pthread_mutex_lock(&mLock);
    temp = mAbortRequest;
    pthread_mutex_unlock(&mLock);
    return temp;
}

void PacketQueue::abort()
{
    pthread_mutex_lock(&mLock);
    mAbortRequest = true;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

#ifndef CUE_PARSE_H
#define CUE_PARSE_H

#include <utils/Log.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/Vector.h>

#define MAX_SIZE_OF_LINE   256

///#define OLD_TYPE

namespace android {

typedef struct {
    char title[256];
    char performer[256];
    char indexBegin[256];
    char indexEnd[256];
    int64_t start_time;
#ifdef OLD_TYPE
    int64_t end_time;
#else
    int64_t spare_time;
#endif
    int64_t duration;
    int idx;
    bool empty;
} CueSongBean;

typedef struct {
    char cuePath[256];
    char mediaPath[256];
    char performer[256];
    char albumName[256];
    char fileName[256];
    char title[256];
    int songNum;
} CueFileBean;

class cueParse  {
public:
    cueParse();
    ~cueParse();

    CueFileBean mCueFile;
    CueSongBean mCurSong;
    Vector<CueSongBean> CueSongs;

    int parseCueFile(char *filename, CueFileBean *cueFileBean, Vector<CueSongBean> *CueSongBeans);
    char* getCodeType(char*, int);

    CueSongBean getSongByIdx(int index);
    int getSongNum();

private:

    bool forEachSong;
    int totalSong;
    int curSongIdx;
    int mLineNums;

    int parseCueLine(char * strLine);

    int parseString(char* src, char * dst);

    int pickTitle(char* src, char * dst, bool needConvert);

    int64_t parseIndex(bool typeHMS, char* str);

    int addSong();

#ifndef OLD_TYPE
    int64_t getSongDur(int index);
#endif

    int getMediaPath(char * mediapath);

    void printInfo();
};

}

#endif  //CUE_PARSE_H

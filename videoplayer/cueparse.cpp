/*
 * Aladdin.Zhong
 * Version 1.2
 * Date 2015-07-26
 * To playback media files can not decoder by hardware(eg. APE)
*/

#include "cueparse.h"
#define LOG_TAG "CueParse"
#include "unicode/ucnv.h"


#define CUE_DBG   ALOGI
#define CUE_INFO  ALOGI
#define CUE_ERR   ALOGE

#define NEED_PURE_CONTEXT

namespace android {

cueParse::cueParse():
    totalSong(0),
    mLineNums(0),
    forEachSong(false)
{
    memset(&mCurSong, 0, sizeof(CueSongBean));
    memset(&mCueFile, 0, sizeof(CueFileBean));
    mCurSong.empty = true;
}

cueParse::~cueParse()
{

}

int cueParse::getSongNum()
{
    return totalSong;
}

CueSongBean cueParse::getSongByIdx(int index)
{
    if(totalSong <= 0 || index > (totalSong - 1)){
        CUE_ERR("No song or invalid index (%d  %d)",index, totalSong);
        return mCurSong;
    }

    return CueSongs[index];
}

/*Parse the whole CUE file*/
int cueParse::parseCueFile(char *filename, CueFileBean *cueFileBean, Vector<CueSongBean> *mCueSongs)
{
    if(filename == NULL)
        return -1;

    FILE *cuefile = fopen(filename, "r");
    if(cuefile == NULL)
        return -1;

    strcpy(mCueFile.cuePath, filename);

    char StrLine[MAX_SIZE_OF_LINE];
    while (!feof(cuefile))
    {
        fgets(StrLine,MAX_SIZE_OF_LINE,cuefile);  //读取一行
        parseCueLine(StrLine);
        mLineNums++;
        //CUE_DBG("%s\n", StrLine);      //输出
    }
    addSong();

    CUE_DBG("[parseCueFile] parse lines done(%d)!",mLineNums);
    fclose(cuefile);

    mCueFile.songNum = totalSong;
    if(getMediaPath(mCueFile.mediaPath) < 0){
        CUE_ERR("get media path fail");
    }

    if(cueFileBean != NULL){
        memcpy(cueFileBean, &mCueFile, sizeof(CueFileBean));
    }
    if(NULL != mCueSongs){
        for(int i=0; i<CueSongs.size(); i++){
            CueSongBean song = CueSongs[i];
#ifndef OLD_TYPE
            song.duration = getSongDur(i);
#endif
            mCueSongs->add(CueSongs[i]);
        }
    }
    printInfo();
    return 0;
}

/*parse per line of the CUE file*/
int cueParse::parseCueLine(char * strLine)
{
    if(strLine == NULL)
        return -1;

    ///CUE_DBG("----[%d]parseCueLine",mLineNums);
    const char* endl = strstr(strLine, "\n");
    if (!endl) return -1;

    int i = 0;
    int purelen = 0;
    char purestr[MAX_SIZE_OF_LINE] = {0};
    char str1[256] = {0};
    char str2[256] = {0};
    char str3[64]  = {0};

    bool needConvert = true;
    bool isGBK       = false;
    bool typeHMS     = false;

#if 1  //Convert string2 to UTF-8
    char str2_dup[256] = {0};

    int strNum = sscanf(strLine, "%s %s %s", str1, str2_dup, str3);

    //Convert code type of string (str2_dup -> str2)
    int ret = 0, str2_len = 0;
    UErrorCode ErrorCode = U_ZERO_ERROR;

    char *pstr2 = NULL;
    pstr2  = str2_dup;
    while(0 != *(pstr2++))
        str2_len++;

    const char* codeType = getCodeType(str2_dup,str2_len);
    ret = ucnv_convert("UTF-8",codeType,str2,sizeof(str2),str2_dup,str2_len,&ErrorCode);
    if (ErrorCode == U_BUFFER_OVERFLOW_ERROR) {
        SLOGE("[parseCueLine-ucnv_convert]the string is too long %s",str2_dup);
        //return -1;
        strcpy(str2, str2_dup);
    } else if (ErrorCode == U_STRING_NOT_TERMINATED_WARNING) {
        str2[ret] = '\0';
    }

    needConvert = ((strcmp(codeType,"UTF-8") == 0)
#if 0
         || (strcmp(codeType,"GBK") == 0)
#endif
    )? false : true;  //UTF-8 do not need convert again

    isGBK = (strcmp(codeType,"GBK") == 0)? true : false; //GBK type use str2 directly
    //Convert Done!
#else
    int strNum = sscanf(strLine, "%s %s %s", str1, str2, str3);
#endif

    CUE_DBG("*****[%d %s]parseCueLine:(%d) %s:  %s  %s",mLineNums,codeType, strNum, str1, str2, str3);

    if(strcmp(str1,"TITLE") == 0){

#ifndef NEED_PURE_CONTEXT //Get whole string of context
        purelen = parseString(str2,purestr);
#else
        if(isGBK) purelen = parseString(str2,purestr);
        else      purelen = pickTitle(strLine,purestr,needConvert);
#endif

        if(forEachSong){
            strcpy(mCurSong.title, purestr);
        }else{
            strcpy(mCueFile.title, purestr);
        }

    }else if(strcmp(str1,"PERFORMER") == 0){

#ifndef NEED_PURE_CONTEXT //Get whole string of context
        purelen = parseString(str2,purestr);
#else
        if(isGBK) purelen = parseString(str2,purestr);
        else      purelen = pickTitle(strLine,purestr,needConvert);
#endif

        if(forEachSong){
            strcpy(mCurSong.performer, purestr);
        }else{
            strcpy(mCueFile.performer, purestr);
        }
    }else if(strcmp(str1,"FILE") == 0){

        purelen = parseString(str2,purestr);

        strcpy(mCueFile.fileName, purestr);
    }else if(strcmp(str1,"SONGWRITER") == 0){

    }else if(strcmp(str1,"CATALOG") == 0){

    }else if(strcmp(str1,"INDEX") == 0){
        if(strlen(str3) < 8)
            return -1;
#if 0
        int hr = 0, min = 0, sec = 0, msec = 0;
        char time_str[2];
        strncpy(time_str, &str3[0], 2);
        if(typeHMS)
            hr = atoi(time_str);
        else
            min = atoi(time_str);

        strncpy(time_str, &str3[3], 2);
        if(typeHMS)
            min = atoi(time_str);
        else
            sec = atoi(time_str);

        strncpy(time_str, &str3[6], 2);
        if(typeHMS)
            sec = atoi(time_str);
        else
            msec = atoi(time_str);
#endif
        int64_t sTime = parseIndex(typeHMS,str3);//(hr * 3600 + min * 60 + sec)*1000 + msec * 10;
#ifdef OLD_TYPE
        if(strcmp(str2,"00") == 0){
            mCurSong.start_time = sTime;
        }else if(strcmp(str2,"01") == 0){
            mCurSong.end_time = sTime;
        }
#else
        if(strcmp(str2,"00") == 0){
        	mCurSong.spare_time = sTime;
        }else if(strcmp(str2,"01") == 0){
        	mCurSong.start_time = sTime;
        }
#endif
    }else if(strcmp(str1,"TRACK") == 0){
        forEachSong = true;

        addSong();

        curSongIdx = atoi(str2);
        mCurSong.idx = curSongIdx;
        mCurSong.empty = false;
    }else if(strcmp(str1,"REM") == 0){
        if(strcmp(str2,"GENRE") == 0){

        }else if(strcmp(str2,"DISCID") == 0){

        }else if(strcmp(str2,"DATE") == 0){

        }else if(strcmp(str2,"COMMENT") == 0){

        }
    }else if(strcmp(str1,"CDTEXTFILE") == 0){

    }
    ///CUE_DBG("----[%d]parseCueLine complete!!",mLineNums);
    return 0;
}

/*Parse a string inside  "" */
int cueParse::parseString(char* src, char * dst)
{
    if(src == NULL || dst == NULL){
        return -1;
    }

    memset(dst, 0, strlen(dst));
    int i;
    if(src[0] == '"'){
        for(i=1;i<strlen(src);i++){
            if(src[i] == '"'){
                break;
            }
        }
        int purelen = i-1;
        strncpy(dst,&src[1],purelen);
    }else{
        strcpy(dst, src);
    }
    ///CUE_INFO("CUE STRING PARSE: %s  %s",src,dst);
    return strlen(dst);
}

/*Pick up context from a line*/
int cueParse::pickTitle(char* src, char * dst, bool needConvert)
{
    if(src == NULL || dst == NULL){
        return -1;
    }
    memset(dst, 0, strlen(dst));
    int i=0, sPos=-1, ePos=-1;
    int lineLen = strlen(src);
    for(i=0;i<lineLen;i++){
        if((src[i] == '"') && (i < lineLen)){
            sPos = i + 1;
            break;
        }
    }

    if((sPos == -1) || (sPos >= (lineLen-1))) return -1;

    for(i=sPos;i<lineLen;i++){
        if((src[i] == '"') && (i < lineLen)){
            ePos = i + 1;
            break;
        }
    }

    if((ePos == -1) || (ePos <= sPos)) return -1;

    int dstLen = ePos - sPos - 1;

    if(needConvert){ //Need convert string -> UTF-8
        char context[dstLen];

        strncpy(context, &src[sPos], dstLen);

        //Convert code type of string (context -> dst)
        int ret = 0, str_len = 0;
        UErrorCode ErrCode = U_ZERO_ERROR;

        char *pstr = NULL;
        pstr  = context;
        while(0 != *(pstr++))
            str_len++;

        const char* codeType = getCodeType(context,str_len);
        ret = ucnv_convert("UTF-8",codeType,dst,sizeof(dst),context,sizeof(context),&ErrCode);
        if (ErrCode == U_BUFFER_OVERFLOW_ERROR) {
            SLOGE("[pickTitle-ucnv_convert]the string is too long %s",context);
            //return -1;
            strcpy(dst, context);
        } else if (ErrCode == U_STRING_NOT_TERMINATED_WARNING) {
            dst[ret] = '\0';
        }
        //Convert Done!
    }else{
        strncpy(dst, &src[sPos], dstLen);
    }

    //CUE_DBG("[pickTitle]%s | %s", dst, src);
    return dstLen;
}

int64_t cueParse::parseIndex(bool typeHMS, char* str)
{
    int hr=0,min=0,sec=0,msec=0;
    char time_str[2];
    strncpy(time_str, &str[0], 2);
    if(typeHMS)
        hr = atoi(time_str);
    else
        min = atoi(time_str);

    strncpy(time_str, &str[3], 2);
    if(typeHMS)
        min = atoi(time_str);
    else
        sec = atoi(time_str);

    strncpy(time_str, &str[6], 2);
    if(typeHMS)
        sec = atoi(time_str);
    else
        msec = atoi(time_str);

    return (hr * 3600 + min * 60 + sec)*1000 + msec * 10;
}

char* cueParse::getCodeType(char* str, int size)
{
    int i;
    int j;
    unsigned char* buf = (unsigned char*)str;

    if (size > 3 && buf[0] ==0xef && buf[1] ==0xbb && buf[2] ==0xbf) {
        buf += 3;
        size -= 3;
    }
    for (i = 0; i < size; ++i) {
        if ((buf[i] & 0x80) == 0) {
            continue;
        } else if ((buf[i] & 0x40) == 0) {
            return "GBK";
        } else {
            int following;

            if ((buf[i] & 0x20) == 0) {
                following = 1;
            } else if ((buf[i] & 0x10) == 0) {
                following = 2;
            } else if ((buf[i] & 0x08) == 0) {
                following = 3;
            } else if ((buf[i] & 0x04) == 0) {
                following = 4;
            } else if ((buf[i] & 0x02) == 0) {
                following = 5;
            } else
                return "GBK";

            if (following == 1) {
                return "GBK";
            }

            for (j = 0; j < following; j++) {
                i++;
                if (i >= size)
                    goto done;

                if ((buf[i] & 0x80) == 0 || (buf[i] & 0x40))
                    return "GBK";
            }
        }
    }
done:
    return "UTF-8";
}

/*Add one song info into the CUE file info*/
int cueParse::addSong()
{
    if(mCurSong.empty == true){
        CUE_ERR("CUE add song: no track to add");
        return -1;
    }

#ifdef OLD_TYPE
    if(!mCurSong.start_time || (mCurSong.end_time - mCurSong.start_time)<10000){
        int sSize = CueSongs.size();
        if(sSize > 0){
            mCurSong.start_time = CueSongs[sSize - 1].end_time;
            //ALOGI("&&&&(%d)start time=%d",sSize,mCurSong.start_time);
        }else{
            mCurSong.start_time = 0;
        }
    }
    mCurSong.duration = mCurSong.end_time - mCurSong.start_time;
#else
    mCurSong.duration = 0;
#endif

    CueSongs.add(mCurSong);
    totalSong++;
	memset(&mCurSong, 0, sizeof(CueSongBean));
    return totalSong;
}

#ifndef OLD_TYPE
int64_t cueParse::getSongDur(int index)
{
    int sSize = CueSongs.size();
    if(index < 0 || index >= sSize){
        CUE_ERR("CUE get duration: Invalid Index");
        return -1;
    }

    CueSongBean song = CueSongs[index];

    int64_t endTime = 0;
    int64_t duration;
    if(index == sSize - 1){
        endTime = song.start_time + 3 * 60 * 1000; //FIX ME (Total duration of APE)
    }else{
        CueSongBean nextSong = CueSongs[index+1];
        endTime = (nextSong.spare_time > 0) ? nextSong.spare_time : nextSong.start_time;
    }
	duration = endTime - song.start_time;
	if(duration < 0) return 0;
    return duration;
}
#endif

/*Get the whole media path (url) of relation APE file*/
int cueParse::getMediaPath(char * mediapath)
{
    if(mediapath == NULL)
        return -1;

    int namePos = 0;

    int str_len = strlen(mCueFile.cuePath);
    for(int i=str_len-1;i>=0;i--){
        if(mCueFile.cuePath[i] == '/'){
            namePos = i;
            break;
        }
    }

    char path[256];
    strncpy(path, mCueFile.cuePath, namePos+1);
    strncpy(&path[namePos+1], mCueFile.fileName, strlen(mCueFile.fileName));
    CUE_INFO("get origin media path: %s",path);

    strcpy(mediapath, path);

    return strlen(mediapath);
}


void cueParse::printInfo()
{
    CUE_INFO("------CUE File Info:");
    CUE_INFO("  ****media name:      %s",mCueFile.fileName);
    CUE_INFO("  ****media title:     %s",mCueFile.title);
    CUE_INFO("  ****media performer: %s",mCueFile.performer);
    CUE_INFO("  ****music number:    %d",mCueFile.songNum);
    CUE_INFO("  ****media path:      %s",mCueFile.mediaPath);
    for(int i=0; i<CueSongs.size(); i++){
        CueSongBean song = CueSongs[i];
        CUE_INFO("  ****[%d]:  %s",song.idx, song.title);
        CUE_INFO("  **********performer:  %s",song.performer);
        CUE_INFO("  **********position:   %lld",song.start_time);
#ifdef OLD_TYPE
        CUE_INFO("  **********endtime:    %lld",song.end_time);
        CUE_INFO("	**********duration:   %lld",song.duration);
#else
        CUE_INFO("  **********sparetime:  %lld",song.spare_time);
        CUE_INFO("  **********duration:   %lld",getSongDur(i));
#endif
    }
}

}  // namespace android

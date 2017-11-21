//
// Created by shoxive on 17/11/20.
//

#include <jni.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include "LameSDK.h"
#include "lameresource/lame.h"

static lame_global_flags *glf = NULL;

void Java_com_shoxive_lamemp3_LameSDK_close(JNIEnv *env, jobject obj) {
    lame_close(glf);
    glf = NULL;
}

jint Java_com_shoxive_lamemp3_LameSDK_encode(JNIEnv *env, jobject obj, jshortArray buffer_l_,
                                             jshortArray buffer_r_, jint samples,
                                             jbyteArray mp3buf_) {
    jshort *buffer_l = env->GetShortArrayElements(buffer_l_, NULL);
    jshort *buffer_r = env->GetShortArrayElements(buffer_r_, NULL);
    jbyte *mp3buf = env->GetByteArrayElements(mp3buf_, NULL);

    const jsize mp3buf_size = env->GetArrayLength(mp3buf_);

    int result = lame_encode_buffer(glf, buffer_l, buffer_r, samples, (u_char *) mp3buf,
                                    mp3buf_size);

    env->ReleaseShortArrayElements(buffer_l_, buffer_l, 0);
    env->ReleaseShortArrayElements(buffer_r_, buffer_r, 0);
    env->ReleaseByteArrayElements(mp3buf_, mp3buf, 0);

    return result;
}

jint Java_com_shoxive_lamemp3_LameSDK_flush(JNIEnv *env, jobject obj, jbyteArray mp3buf_) {
    jbyte *mp3buf = env->GetByteArrayElements(mp3buf_, NULL);

    const jsize mp3buf_size = env->GetArrayLength(mp3buf_);

    int result = lame_encode_flush(glf, (u_char *) mp3buf, mp3buf_size);

    env->ReleaseByteArrayElements(mp3buf_, mp3buf, 0);

    return result;
}

void Java_com_shoxive_lamemp3_LameSDK_init(JNIEnv *env, jobject obj, jint inSampleRate,
                                                  jint outChannel,
                                                  jint outSampleRate, jint outBitrate,
                                                  jint quality) {
    if (glf != NULL) {
        lame_close(glf);
        glf = NULL;
    }
    glf = lame_init();
    lame_set_in_samplerate(glf, inSampleRate);
    lame_set_num_channels(glf, outChannel);
    lame_set_out_samplerate(glf, outSampleRate);
    lame_set_brate(glf, outBitrate);
    lame_set_quality(glf, quality);
    lame_init_params(glf);
}

char *Jstring2CStr(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes",
                                     "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid,
                                                         strencode); // String .getByte("GB2312");
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1); //"\0"
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}
jstring Java_com_shoxive_lamemp3_LameSDK_getLameVersion
        (JNIEnv * env , jobject obj ){
    //const char*  CDECL get_lame_version       ( void );
    const char* versionName = get_lame_version();
    return env -> NewStringUTF(versionName); //jstring     (*NewStringUTF)(JNIEnv*, const char*);
}

//获取文件长度
int get_file_size(char* filename)
{
    struct stat statbuf;
    stat(filename,&statbuf);
    int size=statbuf.st_size;
    return size;
}

jclass clazz = 0;
jmethodID methodid = 0;
void publishJavaProgress(JNIEnv * env, jobject obj, jint progress) {
    // 调用java代码 更新程序的进度条
    // 1.找到java的LameActivity的class
    if(clazz == 0){
        //注意这里初始化clazz会分配一块内存，不能重复初始化，否则易导致内存溢出
        clazz = env->FindClass("com/shoxive/lamemp3/LameSDK");
    }
    //2 找到class 里面的方法定义
    //    jmethodID   (*GetMethodID)(JNIEnv*, jclass, const char*, const char*);
    if(methodid == 0){
        methodid = env->GetMethodID(clazz, "updateProgress", "(I)V");
    }
    if (methodid == 0) {
    }
    //  jclass      (*FindClass)(JNIEnv*, const char*);
    env->CallVoidMethod(obj, methodid, progress);
}


void Java_com_shoxive_lamemp3_LameSDK_convert
        (JNIEnv * env, jobject obj, jstring jpcm, jstring jmp3){
    // 3. 设置MP3的编码方式
    lame_set_VBR(glf, vbr_max_indicator);
    int initStatusCode = lame_init_params(glf);
    if(initStatusCode >= 0){
        //将Java的字符串转成C的字符串
        char* cpcm = Jstring2CStr(env, jpcm);
        char* cmp3 = Jstring2CStr(env, jmp3);

        FILE* fpcm = fopen(cpcm, "rb");
        FILE* fmp3 = fopen(cmp3, "wb");
        //获取文件总长度
        int length = get_file_size(cpcm);
        //每次读取的数据长度
        const int PCM_SIZE = 8192*2;//在模拟信号中每秒取8192*4信号点
        const int MP3_SIZE = 8192*2;
        short int pcm_buffer[PCM_SIZE*2];//这里乘以2是因为取双声道，同样也需要考虑到压缩率
        unsigned char mp3_buffer[MP3_SIZE];
        int read, write, total = 0;
        do{
            //从fwav中读取数据缓存到wav_buffer，每次读取sizeof(short int)*2，读8192次,
            // 取出数据长度sizeof(short int)*2*WAV_SIZE
            read = fread(pcm_buffer,sizeof(short int)*2,PCM_SIZE,fpcm);
            if(read != 0){
                total += read* sizeof(short int)*2;
                publishJavaProgress(env, obj, total);
                //第三个参数表示:每个通道取的数据长度
                write = lame_encode_buffer_interleaved(glf,pcm_buffer,PCM_SIZE,mp3_buffer,MP3_SIZE);
            }else{
                //读到末尾
                write = lame_encode_flush(glf,mp3_buffer,MP3_SIZE);
            }
            //将转换后的数据缓存mp3_buffer写到fmp3文件里
            fwrite(mp3_buffer,sizeof(unsigned char),write,fmp3);
        }while(read != 0);

        lame_close(glf);
        fclose(fpcm);
        fclose(fmp3);
        glf = NULL;
    }

}
//获取文件长度，有限制，且是简介获取（读内存）
int file_size(FILE* fp)
{
    //FILE *fp=fopen(filename,"r");
    if(!fp) return -1;
    fseek(fp,0L,SEEK_END);
    int size=ftell(fp);
    fclose(fp);
    return size;
}

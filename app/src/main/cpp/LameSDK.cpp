//
// Created by shoxive on 17/11/20.
//
#include <jni.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include "LameSDK.h"
#include "lameresource/lame.h"
#include <android/log.h>

#define TAG "lamesdk" // LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // LOGD类型
#define be_short(s) ((short) ((unsigned short) (s) << 8) | ((unsigned short) (s) >> 8))
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
//    LOGD("lameInit",
//         "inSampleRate= %d,outChannel= %d,outSampleRate= %d, outBitrate= %d,quality= %d",
//         inSampleRate, outChannel, outSampleRate, outBitrate, quality);
    glf = lame_init();
    lame_set_num_channels(glf, outChannel);
    lame_set_in_samplerate(glf, inSampleRate);
    lame_set_brate(glf, outBitrate);
//    lame_set_mode(glf, MONO);
    lame_set_quality(glf, quality);
    lame_set_out_samplerate(glf, outSampleRate);
    int res = lame_init_params(glf);
//    LOGD("lameInit", "res= %d", res);
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
        rtn = (char *) malloc((size_t) (alen + 1)); //"\0"
        memcpy(rtn, ba, (size_t) alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

jstring Java_com_shoxive_lamemp3_LameSDK_getLameVersion
        (JNIEnv *env, jobject obj) {
    //const char*  CDECL get_lame_version       ( void );
    const char *versionName = get_lame_version();
    return env->NewStringUTF(versionName); //jstring     (*NewStringUTF)(JNIEnv*, const char*);
}


jclass clazz = 0;
jmethodID methodid = 0;

void publishJavaProgress(JNIEnv *env, jobject obj, jint progress) {
    // 调用java代码 更新程序的进度条
    // 1.找到java的LameActivity的class
    if (clazz == 0) {
        //注意这里初始化clazz会分配一块内存，不能重复初始化，否则易导致内存溢出
        clazz = env->FindClass("com/shoxive/lamemp3/LameSDK");
    }
    //2 找到class 里面的方法定义
    //    jmethodID   (*GetMethodID)(JNIEnv*, jclass, const char*, const char*);
    if (methodid == 0) {
        methodid = env->GetMethodID(clazz, "updateProgress", "(I)V");
    }
    if (methodid == 0) {
    }
    //  jclass      (*FindClass)(JNIEnv*, const char*);
    env->CallVoidMethod(obj, methodid, progress);
}

int read_samples(FILE *input_file, short *input) {
    int nb_read;
    nb_read = fread(input, 1, sizeof(short), input_file) / sizeof(short);
    int i = 0;
    while (i < nb_read) {
        input[i] = be_short(input[i]);
        i++;
    }

    return nb_read;
}

void Java_com_shoxive_lamemp3_LameSDK_convert(JNIEnv *env, jobject obj, jstring jpcm,
                                              jstring jmp3) {

    //将Java的字符串转成C的字符串
    const char *source_path, *target_path;
    source_path = env->GetStringUTFChars(jpcm, NULL);
    target_path = env->GetStringUTFChars(jmp3, NULL);
    FILE *input_file, *output_file;
    input_file = fopen(source_path, "rb");
    output_file = fopen(target_path, "wb");
    //每次读取的数据长度
    short input[8192];
    unsigned char output[8192];

    int nb_read = 0;
    int nb_write = 0;
    int nb_total = 0;
//    LOGD("Encoding started");
    while ((nb_read = read_samples(input_file, input))) {
        nb_write = lame_encode_buffer(glf, input, input, nb_read, output, 8192);
//        LOGD("Encoding Step ===", " %d bytes", nb_write);
        fwrite(output, nb_write, 1, output_file);
        nb_total += nb_write;
    }
//    LOGD("Encoding total ===", " %d bytes", nb_total);
    nb_write = lame_encode_flush(glf, output, 8192);
    fwrite(output, nb_write, 1, output_file);
//    LOGD("Encoding Flushed ===", " %d bytes", nb_write);
    fclose(input_file);
    fclose(output_file);
}

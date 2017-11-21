
//
// Created by shoxive on 17/11/20.
//

#include <jni.h>

extern "C"
{
void Java_com_shoxive_lamemp3_LameSDK_close(JNIEnv *env, jobject obj);

jint Java_com_shoxive_lamemp3_LameSDK_encode(JNIEnv *env, jobject obj, jshortArray buffer_l_,
                                             jshortArray buffer_r_, jint samples, jbyteArray mp3buf_);

jint Java_com_shoxive_lamemp3_LameSDK_flush(JNIEnv *env, jobject obj, jbyteArray mp3buf_);

void Java_com_shoxive_lamemp3_LameSDK_init(JNIEnv *env, jobject obj, jint inSampleRate,
                                                  jint outChannel, jint outSampleRate, jint outBitrate, jint quality);
jstring Java_com_shoxive_lamemp3_LameSDK_getLameVersion(JNIEnv *env, jobject obj);

void Java_com_shoxive_lamemp3_LameSDK_convert(JNIEnv *env, jobject obj, jstring jpcm , jstring jmp3);

}
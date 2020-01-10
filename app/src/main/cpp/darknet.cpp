#include <jni.h>
#include <string>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include <android/log.h>

#include "DarknetAPI.h"
#include "image_opencv.h"
#include "image.h"


#define  LOG_TAG    "Darknet"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif
/*
* Class:     wildsheep_darknet_DarknetUtils
* Method:    inference
* Signature: (Ljava/lang/String;)[Lwildsheep/darknet/Result;
*/
JNIEXPORT jobjectArray JNICALL Java_com_recog_DarknetDao_detectLicensePlate
        (JNIEnv *, jobject, jlong);
JNIEXPORT jint JNICALL Java_com_recog_DarknetDao_Rotate
        (JNIEnv *, jobject, jlong, jlong);
JNIEXPORT jobjectArray JNICALL Java_com_recog_DarknetDao_Recognition
        (JNIEnv *, jobject, jlong, jfloat thresh_j, jint lptype_j);
JNIEXPORT jint JNICALL Java_com_recog_DarknetDao_fitAngleAndSize
        (JNIEnv *, jobject, jlong, jlong);
/*
 * Class:     wildsheep_darknet_DarknetUtils
 * Method:    load
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_recog_DarknetDao_load
        (JNIEnv *, jobject, jstring, jstring, jstring, jstring, jstring, jstring);

/*
 * Class:     wildsheep_darknet_DarknetUtils
 * Method:    unload
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_recog_DarknetDao_unload
        (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif

static DarknetAPI * api;

image readRawImageData(std::string image_path);
//image convertImage(int width, int height, byte[] elements);


/*
* Class:     wildsheep_darknet_DarknetUtils
* Method:    inference
* Signature: (Ljava/lang/String;)[Lwildsheep/darknet/Result;
*/
JNIEXPORT jobjectArray JNICALL Java_com_recog_DarknetDao_detectLicensePlate
        (JNIEnv * env, jobject, jlong image_j) {
    LOGI("JNI: LicensePlate Detect starting");

    Mat input = *((Mat*)image_j);
    std::vector<Result> results = api->LicensePlateDetect(input);
    LOGI("JNI: Detection results count: %d" , results.size());

    jclass c = (*env).FindClass("com/recog/Result");
    jmethodID mid = NULL;
    jsize size = results.size();
    LOGI("JNI: FindClass: %d" , (c==NULL));

    jobjectArray returnArr = (*env).NewObjectArray(size, c, NULL);
    if (NULL != c) {
        mid = (*env).GetMethodID(c, "<init>", "(IIIIFLjava/lang/String;)V");
        if (NULL != mid) {
            for (int i = 0; i < results.size(); ++i) {
                Result r = results[i];
                jint l = r.left;
                jint jr = r.right;
                jint t = r.top;
                jint b = r.bot;
                jstring jlabel = env->NewStringUTF(r.label.c_str());
                jfloat conf = r.confidence;

                jobject jObj = (*env).NewObject(c, mid, l, t, jr, b, conf, jlabel);
                (*env).SetObjectArrayElement(returnArr, i, jObj);
                LOGI("JNI: %s %d %d %d %d %f" ,r.label.c_str() ,  r.left , r.top , r.right , r.bot , r.confidence);
            }
        }
    }
    LOGI("JNI: License Plate Detect complete");
    return returnArr;
}

JNIEXPORT jint JNICALL Java_com_recog_DarknetDao_Rotate
        (JNIEnv * env, jobject, jlong input_j, jlong output_j) {
    LOGI("JNI: Plate Rotate starting");

    Mat input = *((Mat*)input_j);
    Mat &output = *((Mat *)output_j);
    int angle = api->getRotAngle(input, output);
    LOGI("JNI: Rotation Results: %d  complete" , angle);
    return angle;
}
JNIEXPORT jobjectArray JNICALL Java_com_recog_DarknetDao_Recognition
        (JNIEnv * env, jobject, jlong image_j, jfloat thresh_j, jint lptype_j) {
    LOGI("JNI: LicensePlate Recognition starting");

    Mat input = *((Mat*)image_j);

    std::vector<Result> results = api->LicensePlateRecognition(input, thresh_j, lptype_j);
    LOGI("JNI: Recognition results count: %d" , results.size());

    jclass c = (*env).FindClass("com/recog/Result");
    jmethodID mid = NULL;
    jsize size = results.size();
    LOGI("JNI: FindClass: %d" , (c==NULL));

    jobjectArray returnArr = (*env).NewObjectArray(size, c, NULL);
    if (NULL != c) {
        mid = (*env).GetMethodID(c, "<init>", "(IIIIFLjava/lang/String;)V");
        if (NULL != mid) {
            for (int i = 0; i < results.size(); ++i) {
                Result r = results[i];
                jint l = r.left;
                jint jr = r.right;
                jint t = r.top;
                jint b = r.bot;
                jstring jlabel = env->NewStringUTF(r.label.c_str());
                jfloat conf = r.confidence;

                jobject jObj = (*env).NewObject(c, mid, l, t, jr, b, conf, jlabel);
                (*env).SetObjectArrayElement(returnArr, i, jObj);
                LOGI("JNI: %s %d %d %d %d %f" ,r.label.c_str() ,  r.left , r.top , r.right , r.bot , r.confidence);
            }
        }
    }
    LOGI("JNI: License Plate Recognition complete");
    return returnArr;
}

/*
* Class:     wildsheep_darknet_DarknetUtils
* Method:    load
* Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
*/
JNIEXPORT jboolean JNICALL Java_com_recog_DarknetDao_load
        (JNIEnv * env, jobject obj, jstring detect_cfgfile_j, jstring detect_weightfile_j,
         jstring carlp_cfgfile_j, jstring carlp_weightfile_j,
         jstring motorlp_cfgfile_j, jstring motorlp_weightfile_j) {

    LOGI("JNI: Loading networks and weights..");

    std::string detect_cfgfile = env->GetStringUTFChars(detect_cfgfile_j, (jboolean *)false);
    std::string detect_weightfile = env->GetStringUTFChars(detect_weightfile_j, (jboolean *)false);
    std::string carlp_cfgfile = env->GetStringUTFChars(carlp_cfgfile_j, (jboolean *)false);
    std::string carlp_weightfile = env->GetStringUTFChars(carlp_weightfile_j, (jboolean *)false);
    std::string motorlp_cfgfile = env->GetStringUTFChars(motorlp_cfgfile_j, (jboolean *)false);
    std::string motorlp_weightfile = env->GetStringUTFChars(motorlp_weightfile_j, (jboolean *)false);

    api = new DarknetAPI(&detect_cfgfile[0u], &detect_weightfile[0u],
                         &carlp_cfgfile[0u], &carlp_weightfile[0u],
                         &motorlp_cfgfile[0u], &motorlp_weightfile[0u]);
    assert(api && "Failed to initialize DarknetAPI");
    jboolean b = 1;
    LOGI("JNI: Done.");
    return b;
}

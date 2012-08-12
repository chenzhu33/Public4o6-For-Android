#include <string.h>
#include <jni.h>
#include "tunnel.h"

/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    getTunnelInfos
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_getTunnelInfos
  (JNIEnv *env, jobject thiz)
{
    int ret = 0;
    char show_string[512];
    if((ret = get_tunnel_info(show_string)) < 0) {
        return (*env)->NewStringUTF(env, "-1");
    }
    jstring result = (*env)->NewStringUTF(env, show_string);
    return result;
}

/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    getLogs
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_getLogs
  (JNIEnv *env, jobject thiz)
{
//    char loglist_string[128*8];
//    get_logs(loglist_string);
//    jstring result = (*env)->NewStringUTF(env, loglist_string);
//    return result;
    return NULL;
}

/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    conTunnel
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_conTunnel
  (JNIEnv *env, jobject thiz, jstring tc_addr, jstring ti_addr)
{
    char* ti;
    ti = (char*)(*env)->GetStringUTFChars(env,ti_addr,0);
    char* tc;
    tc = (char*)(*env)->GetStringUTFChars(env,tc_addr,0);  
    return init_tunnel(tc,ti);
}

/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    disconTunnel
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_disconTunnel
  (JNIEnv *env, jobject thiz)
{
    return close_tunnel();
}

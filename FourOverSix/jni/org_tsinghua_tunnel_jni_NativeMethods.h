/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_tsinghua_tunnel_jni_NativeMethods */

#ifndef _Included_org_tsinghua_tunnel_jni_NativeMethods
#define _Included_org_tsinghua_tunnel_jni_NativeMethods
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    getTunnelInfos
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_getTunnelInfos
  (JNIEnv *, jobject);

/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    getLogs
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_getLogs
  (JNIEnv *, jobject);

/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    conTunnel
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_conTunnel
  (JNIEnv *, jobject, jstring);

/*
 * Class:     org_tsinghua_tunnel_jni_NativeMethods
 * Method:    disconTunnel
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_tsinghua_tunnel_jni_NativeMethods_disconTunnel
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif

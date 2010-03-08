/*
* Copyright (c) 2000 - 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
 *
*/


/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_nokia_comm_wap_wps_Session */

#ifndef _Included_com_nokia_comm_wap_wps_Session
#define _Included_com_nokia_comm_wap_wps_Session
#ifdef __cplusplus
extern "C" {
#endif
#undef com_nokia_comm_wap_wps_Session_DATA_UNKNOWN
#define com_nokia_comm_wap_wps_Session_DATA_UNKNOWN 0L
#undef com_nokia_comm_wap_wps_Session_DATA_SERVER_HEADERS
#define com_nokia_comm_wap_wps_Session_DATA_SERVER_HEADERS 1L
#undef com_nokia_comm_wap_wps_Session_DATA_CLIENT_HEADERS
#define com_nokia_comm_wap_wps_Session_DATA_CLIENT_HEADERS 2L
#undef com_nokia_comm_wap_wps_Session_DATA_CAPABILITIES
#define com_nokia_comm_wap_wps_Session_DATA_CAPABILITIES 3L
#undef com_nokia_comm_wap_wps_Session_DATA_REDIRECT_OPTIONS
#define com_nokia_comm_wap_wps_Session_DATA_REDIRECT_OPTIONS 4L
#undef com_nokia_comm_wap_wps_Session_DATA_REDIRECT_ADDRESSES
#define com_nokia_comm_wap_wps_Session_DATA_REDIRECT_ADDRESSES 5L
#undef com_nokia_comm_wap_wps_Session_DATA_GROUP_QUEUE_LEN
#define com_nokia_comm_wap_wps_Session_DATA_GROUP_QUEUE_LEN 6L
#undef com_nokia_comm_wap_wps_Session_DATA_ERROR_HEADERS
#define com_nokia_comm_wap_wps_Session_DATA_ERROR_HEADERS 7L
#undef com_nokia_comm_wap_wps_Session_DATA_ERROR_BODY
#define com_nokia_comm_wap_wps_Session_DATA_ERROR_BODY 8L
#undef com_nokia_comm_wap_wps_Session_DATA_DISCONNECT_REASON
#define com_nokia_comm_wap_wps_Session_DATA_DISCONNECT_REASON 9L
#undef com_nokia_comm_wap_wps_Session_DATA_SUSPEND_REASON
#define com_nokia_comm_wap_wps_Session_DATA_SUSPEND_REASON 10L
#undef com_nokia_comm_wap_wps_Session_DATA_PROTOCOL_VERSION
#define com_nokia_comm_wap_wps_Session_DATA_PROTOCOL_VERSION 11L
#undef com_nokia_comm_wap_wps_Session_STATE_UNKNOWN
#define com_nokia_comm_wap_wps_Session_STATE_UNKNOWN 0L
#undef com_nokia_comm_wap_wps_Session_STATE_CONNECTING
#define com_nokia_comm_wap_wps_Session_STATE_CONNECTING 1L
#undef com_nokia_comm_wap_wps_Session_STATE_CONNECTED
#define com_nokia_comm_wap_wps_Session_STATE_CONNECTED 2L
#undef com_nokia_comm_wap_wps_Session_STATE_SUSPENDED
#define com_nokia_comm_wap_wps_Session_STATE_SUSPENDED 3L
#undef com_nokia_comm_wap_wps_Session_STATE_RESUMING
#define com_nokia_comm_wap_wps_Session_STATE_RESUMING 4L
#undef com_nokia_comm_wap_wps_Session_STATE_DISCONNECTED
#define com_nokia_comm_wap_wps_Session_STATE_DISCONNECTED 5L
#undef com_nokia_comm_wap_wps_Session_STATE_DISCONNECTING
#define com_nokia_comm_wap_wps_Session_STATE_DISCONNECTING 6L
#undef com_nokia_comm_wap_wps_Session_STATE_REPLYING
#define com_nokia_comm_wap_wps_Session_STATE_REPLYING 7L
#undef com_nokia_comm_wap_wps_Session_MAX_QUEUE_LENGTH
#define com_nokia_comm_wap_wps_Session_MAX_QUEUE_LENGTH 100L
/* Inaccessible static: exceptionMessage */
/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspConnect
 * Signature: (ILcom/nokia/comm/wap/wps/ByteBuffer;Lcom/nokia/comm/wap/wps/ByteBuffer;)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspConnect
  (JNIEnv *, jobject, jint, jobject, jobject);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspCreateSession
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspCreateSession
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspDisconnect
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspDisconnect
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspGetConnection
 * Signature: (ILcom/nokia/comm/wap/wps/Connection;)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspGetConnection
  (JNIEnv *, jobject, jint, jobject);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspGetEvent
 * Signature: (IILcom/nokia/comm/wap/wps/Event;)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspGetEvent
  (JNIEnv *, jobject, jint, jint, jobject);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspGetNrOfEvents
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspGetNrOfEvents
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspGetSessionData
 * Signature: (IILcom/nokia/comm/wap/wps/ByteBuffer;)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspGetSessionData
  (JNIEnv *, jobject, jint, jint, jobject);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspGetSessionState
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspGetSessionState
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspLoad
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspLoad
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspReleaseSession
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspReleaseSession
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspResume
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspResume
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspSave
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspSave
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_nokia_comm_wap_wps_Session
 * Method:    wspSuspend
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_nokia_comm_wap_wps_Session_wspSuspend
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif
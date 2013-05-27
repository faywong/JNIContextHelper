/*
**                Copyright 2012, MARVELL SEMICONDUCTOR, LTD.
** THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.
** NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
** OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
** DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
** THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
** IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
**
** MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES,
** MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL
** SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.
** (MJKK), MARVELL ISRAEL LTD. (MSIL).
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "JNIContextHelper"
#include <utils/Log.h>
#include "jni.h"

#include "JNIContextHelper.h"

static void ThreadDestructor(void *context)
{
    ALOGD("thread_destructor() in context:0x%x", context);
    TLStore* tlstore = (TLStore*)context;
    if (NULL != tlstore && NULL != tlstore->context) {
        pthread_setspecific(tlstore->context->tls_key, NULL);
        JavaVM * jvm = tlstore->context->jvm;
        ALOGD("JVM:%p", jvm);
        if ((NULL != jvm) && jvm->DetachCurrentThread() != JNI_OK) {
            ALOGD("FATAL ERROR! DetachCurrentThread() failed!");
        }
        free(tlstore);
    }
}

void InitJNIContext(JNIContext& context, JavaVM* vm)
{
    context.jvm = vm;
    context.thread_destructor = ThreadDestructor;
    pthread_key_create(&context.tls_key, context.thread_destructor);
}

void RecordJNIEnv(JNIContext& context, JNIEnv* env)
{
    if ((pthread_getspecific(context.tls_key)) == NULL) {
         ALOGD("pthread_getspecific() NULL case");
         TLStore* tlstore = (TLStore*)malloc(sizeof(TLStore));
         memset(tlstore, 0, sizeof(TLStore));
         if (NULL != tlstore) {
             tlstore->jni_env = env;
             tlstore->context = &context;
             pthread_setspecific(context.tls_key, tlstore);
         }
    }
}

JNIEnv* GetJNIEnv(JNIContext& context) {
    JNIEnv* env;
    TLStore* tlstore = NULL;
    void* value = pthread_getspecific(context.tls_key);
    tlstore = reinterpret_cast<TLStore*>(value);

    if (NULL == context.jvm) {
        ALOGD("context.jvm NULL");
        return NULL;
    }
    if (NULL == tlstore || (NULL == tlstore->jni_env)) {
        //case:native thread, so attach it to retrieve the JNIEnv object
        ALOGD("native thread so attach it to retrieve the JNIEnv object");
        JavaVMAttachArgs thread_args;
        thread_args.name = "HDMI_JNI";
        thread_args.version = context.vmversion;
        thread_args.group = NULL;
        context.jvm->AttachCurrentThread(&env, &thread_args);
        RecordJNIEnv(context, env);
    } else {
        ALOGD("already have a JNIEnv, use it directly");
        env = tlstore->jni_env;
    }
    ALOGD("native thread GetJNIEnv()=%08x\n", (unsigned int)env);
    return env;
}

#ifndef JNICTX_H
#define JNICTX_H

#include "jni.h"

typedef enum JNICtxResult {
    UNKNOW_ERROR = -3,
    BAD_PARAMETER = -2,
    MALLOC_FAILED = -1,
    OK = 0,
    SUCCESS = OK,
} JNICtxResult;

typedef struct JNICtx {
    JavaVM *jvm;
    int vmversion;
    pthread_key_t tls_key;
    void (*thread_dtr)(void *context);
    struct JNICtx *self;
} JNICtx;

typedef struct TLStore {
    JNIEnv* jni_env;
    JNICtx* context;
} TLStore;

int InitJNICtx(JavaVM* vm, JNICtx **jni_ctx);
JNIEnv* GetJNIEnv(JNICtx* context);
int LogJNIEnv(JNICtx* context, JNIEnv* env);

#endif  // JNICTX_H

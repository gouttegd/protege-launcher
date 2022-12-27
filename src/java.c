/*
 * Protégé launcher
 * Copyright (C) 2022 Damien Goutte-Gattat
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "java.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dlfcn.h>

#include <jni.h>

#include <xmem.h>

typedef jint (JNICALL CreateJavaVM_t)(JavaVM **vm, JNIEnv **env, JavaVMInitArgs *args);

void*
load_jre_from_path(const char *base_path, const char *lib_path)
{
    void *lib;
    char *full_path = NULL;

    (void) xasprintf(&full_path, "%s/%s", base_path, lib_path);
    lib = dlopen(full_path, RTLD_LAZY);
    free(full_path);

    return lib;
}

void*
load_jre(const char *path, int bundled)
{
    void *lib = NULL;

    if ( path )
        lib = load_jre_from_path(path, bundled ? BUNDLED_JAVA_LIB_PATH : JAVA_LIB_PATH);

    if ( ! lib && (path = getenv("JAVA_HOME")) )
        lib = load_jre_from_path(path, JAVA_LIB_PATH);

    return lib;
}

static jobjectArray
get_arguments(JNIEnv *env, const char **args)
{
    size_t n;
    jobjectArray java_args = NULL;

    for ( n = 0; args && args[n]; n++ ) ;

    java_args = (*env)->NewObjectArray(env, n,
                                       (*env)->FindClass(env, "java/lang/String"),
                                       (*env)->NewStringUTF(env, ""));
    for ( n = 0; java_args && args && args[n]; n++ ) {
        jstring arg = (*env)->NewStringUTF(env, args[n]);
        if ( arg )
            (*env)->SetObjectArrayElement(env, java_args, n, arg);
        else
            java_args = NULL;   /* Exit loop. */
    }

    return java_args;
}

static int
start_java_main(JNIEnv *env, const char  *main_class_name, const char **args)
{
    jclass main_class;
    jmethodID main_method;
    jobjectArray main_args;

    if ( ! (main_class = (*env)->FindClass(env, main_class_name)) )
        return JAVA_CLASS_NOT_FOUND;

    if ( ! (main_method = (*env)->GetStaticMethodID(env, main_class, "main",
                                                    "([Ljava/lang/String;)V")) )
        return JAVA_METHOD_NOT_FOUND;

    if ( ! (main_args = get_arguments(env, args)) )
        return JAVA_OUT_OF_MEMORY;

    (*env)->CallStaticVoidMethod(env, main_class, main_method, main_args);

    return 0;
}

static JavaVMOption *
get_java_options(const char **options, int *n_options)
{
    JavaVMOption *jvm_opts = NULL;
    size_t n;

    for ( n = 0; options && options[n]; n++ );
    jvm_opts = xcalloc(n, sizeof(JavaVMOption));
    for ( n = 0; options && options[n]; n++ )
        jvm_opts[n].optionString = (char *) options[n];

    *n_options = n;

    return jvm_opts;
}

int
start_java(void        *jre,
           const char **vm_args,
           const char  *main_class,
           const char **main_args)
{
    JavaVM *jvm = NULL;
    JavaVMInitArgs jvm_args;
    JNIEnv *env;
    CreateJavaVM_t *create_java_vm = NULL;
    int ret = 0;

    jvm_args.version = JNI_VERSION_1_2;
    jvm_args.ignoreUnrecognized = JNI_TRUE;
    jvm_args.options = get_java_options(vm_args, &(jvm_args.nOptions));

    if ( ret == 0 && ! (create_java_vm = (CreateJavaVM_t *)dlsym(jre, "JNI_CreateJavaVM")) )
        ret = JAVA_SYMBOL_NOT_FOUND;

    if ( ret == 0 && create_java_vm(&jvm, &env, &jvm_args) == JNI_ERR )
        ret = JAVA_CREATE_VM_ERROR;

    if ( ret == 0 )
        ret = start_java_main(env, main_class, main_args);

    if ( ret == 0 ) {
        if ( (*env)->ExceptionCheck(env) ) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }

        (*jvm)->DetachCurrentThread(jvm);
    }

    if ( jvm )
        (*jvm)->DestroyJavaVM(jvm);

    if ( jvm_args.options )
        free(jvm_args.options);

    return ret;
}

const char *
get_java_error(int code)
{
    switch ( code ) {
    case JAVA_CLASS_NOT_FOUND: return "Cannot find Java class";
    case JAVA_METHOD_NOT_FOUND: return "Cannot find Java method";
    case JAVA_OUT_OF_MEMORY: return "Cannot allocate Java objects";
    case JAVA_SYMBOL_NOT_FOUND: return "Cannot find JNI symbol";
    case JAVA_CREATE_VM_ERROR: return "Cannot create Java virtual machine";
    default: return "Unknown error";
    }
}

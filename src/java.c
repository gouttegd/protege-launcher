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

#include <dlfcn.h>

#include <jni.h>

#include <xmem.h>

#if defined(PROTEGE_WIN32)
#include <windows.h>    /* SetDllDirectory */
#include <shlwapi.h>    /* PathFileExists */
#include <string.h>     /* strrchr */
#endif


/*
 * JAVA_LIB_PATH is the name and location of the main Java library
 * (the library containing the JNI_CreateJavaVM function) within the
 * JRE. This seems to change depending on the JRE version, the locations
 * used here are correct for the JRE 11 used by Protégé 5.6.0.
 */
#if   defined(PROTEGE_LINUX)
#define JAVA_LIB_PATH "/lib/server/libjvm.so"
#elif defined(PROTEGE_MACOS)
#define JAVA_LIB_PATH "/lib/jli/libjli.dylib"
#elif defined(PROTEGE_WIN32)
#define JAVA_LIB_PATH "\\bin\\server\\jvm.dll"
#endif

/*
 * BUNDLED_JAVA_LIB_PATH is the location of the Java library within
 * Protégé's directory, when using the bundled JRE.
 */
#if defined(PROTEGE_WIN32)
#define BUNDLED_JAVA_LIB_PATH "\\jre" JAVA_LIB_PATH
#else
#define BUNDLED_JAVA_LIB_PATH "/jre" JAVA_LIB_PATH
#endif


typedef jint (JNICALL CreateJavaVM_t)(JavaVM **vm, JNIEnv **env, JavaVMInitArgs *args);


/*
 * Catenate the two specified path components and attempt to load the
 * Java library from the resulting full pathname.
 *
 * Returns a handle to the loaded library if loading was successful,
 * otherwise NULL.
 */
void*
load_jre_from_path(const char *base_path, const char *lib_path)
{
    void *lib;
    char *full_path = NULL;

    (void) xasprintf(&full_path, "%s%s", base_path, lib_path);

#if defined(PROTEGE_WIN32)
    /*
     * On Windows, loading the jvm.dll library may fail (with a not very
     * helpful error message) because it may require loading some other
     * JDK libraries that are located in the JRE's bin directory, but
     * the loader doesn't know it has to look for them in that
     * directory. So we need to explicitly add that directory to the
     * DLL search path before attempting to use LoadLibrary.
     */
    if ( PathFileExists(full_path) ) {
        char *bin_path, *last_slash;
        int n = 2;

        bin_path = xstrdup(full_path);
        while ( n-- > 0 && (last_slash = strrchr(bin_path, '\\')) )
            *last_slash = '\0';

        (void) SetDllDirectory(bin_path);
        free(bin_path);
    }
#endif

    lib = dlopen(full_path, RTLD_LAZY);
    free(full_path);

#if defined(PROTEGE_WIN32)
    /* If the library couldn't be loaded, reset the DLL search path. */
    if ( ! lib )
        (void) SetDllDirectory(NULL);
#endif

    return lib;
}

/**
 * Attempt to load the Java library.
 *
 * @param[in] path    The base directory from where the Java library
 *                    should be loaded; if NULL, JAVA_HOME will be used
 *                    if it is defined in the environment.
 * @param[in] bundled If non-zero, @a path is expected to be the
 *                    Protégé directory, and the Java library will be
 *                    looked for in the jre/ subdirectory; otherwise,
 *                    @a path is assumed to be a JRE directory.
 * @param[out] jre    A pointer that will receive the handle to the
 *                    Java library after it has been loaded.
 *
 * @return
 * - 0 if the Java library has been successfully loaded;
 * - JAVA_DLOPEN_ERROR if an error occured.
 */
int
load_jre(const char *path, int bundled, void **jre)
{
    void *lib = NULL;

    if ( path )
        lib = load_jre_from_path(path, bundled ?
                                 BUNDLED_JAVA_LIB_PATH : JAVA_LIB_PATH);

    if ( ! lib && (path = getenv("JAVA_HOME")) )
        lib = load_jre_from_path(path, JAVA_LIB_PATH);

    *jre = lib;
    return lib ? 0 : JAVA_DLOPEN_ERROR;
}

/*
 * Convert the specified char ** array into an equivalent Java array.
 *
 * May return NULL if the Java array itself or one of its String
 * elements could not be allocated.
 */
static jobjectArray
get_arguments(JNIEnv *env, const char **args)
{
    size_t n;
    jobjectArray java_args = NULL;

    for ( n = 0; args && args[n]; n++ ) ;   /* Number of items in array. */

    java_args = (*env)->NewObjectArray(env, n,
                                       (*env)->FindClass(env, "java/lang/String"),
                                       (*env)->NewStringUTF(env, ""));
    for ( n = 0; java_args && args && args[n]; n++ ) {
        /* Populate each array item. */
        jstring arg = (*env)->NewStringUTF(env, args[n]);
        if ( arg )
            (*env)->SetObjectArrayElement(env, java_args, n, arg);
        else
            java_args = NULL;   /* Exit loop. */
    }

    return java_args;
}

/*
 * Call the main method of a Java class.
 *
 * Returns 0 if successful, or a JAVA_* error value.
 */
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

/*
 * Convert a strings array into an array of JavaVMOption structures.
 *
 * The options array must be NULL-terminated. The number of options
 * will be returned to the n_options parameter.
 */
static JavaVMOption *
get_java_options(const char **options, jint *n_options)
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

/**
 * Start the Java virtual machine.
 *
 * @param jre        A handle to the Java library.
 * @param vm_args    A NULL-terminated list of Java options.
 * @param main_class The name of the Java main class.
 * @param main_args  A NULL-terminated list of arguments for the main
 *                   method; may be NULL itself for no arguments.
 *
 * @return 0 if successful, or one of the JAVA_* error values.
 */
#if !defined(PROTEGE_MACOS)
int
start_java(void        *jre,
           const char **vm_args,
           const char  *main_class,
           const char **main_args)
#else
static int
start_java_impl(void        *jre,
                const char **vm_args,
                const char  *main_class,
                const char **main_args)
#endif
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

#if defined(PROTEGE_MACOS)

/*
 * On macOS, the Java virtual machine must be started in a separate
 * thread, while the first thread runs an infinite loop. This is needed
 * to properly receive events from the operating system.
 */

#include <err.h>
#include <pthread.h>
#include <CoreFoundation/CoreFoundation.h>

/*Dummy callback for the main thread loop. */
static void
dummy_callback(void *info) { }

/* Wrap the parameters to the real start_java function. */
struct java_start_info
{
    void        *jre;
    const char **vm_args;
    const char  *main_class;
    const char **main_args;
};

/*
 * The function that will run in the dedicated thread. Merely a wrapper
 * for the start_java_impl function above. Explicitly calls exit at the
 * end (whether start_java_impl was successful or not) to cause the
 * dummy loop in the first thread to stop.
 */
static void *
start_java_wrapper(void *info)
{
    struct java_start_info *jinfo;
    int ret;

    jinfo = (struct java_start_info *)info;
    if ( (ret = start_java_impl(jinfo->jre, jinfo->vm_args, jinfo->main_class,
                                jinfo->main_args)) != 0 )
        errx(EXIT_FAILURE, "Cannot start Java: %s", get_java_error(ret));

    exit(EXIT_SUCCESS);
}

/*
 * Start the Java virtual machine in a dedicated thread.
 */
int
start_java(void        *jre,
           const char **vm_args,
           const char  *main_class,
           const char **main_args)
{
    pthread_t jvm_thread;
    pthread_attr_t jvm_thread_attr;
    CFRunLoopSourceContext loop_context;
    CFRunLoopSourceRef loop_ref;
    struct java_start_info info;

    info.jre = jre;
    info.vm_args = vm_args;
    info.main_class = main_class;
    info.main_args = main_args;

    /* Start the thread where the JVM will run. */
    pthread_attr_init(&jvm_thread_attr);
    pthread_attr_setscope(&jvm_thread_attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&jvm_thread_attr, PTHREAD_CREATE_DETACHED);
    if ( pthread_create(&jvm_thread, &jvm_thread_attr, start_java_wrapper,
                        (void *)&info) != 0 )
        return JAVA_CREATE_THREAD_ERROR;

    /* Run a dummy loop in the main thread. */
    memset(&loop_context, 0, sizeof(loop_context));
    loop_context.perform = &dummy_callback;
    loop_ref = CFRunLoopSourceCreate(NULL, 0, &loop_context);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), loop_ref, kCFRunLoopCommonModes);
    CFRunLoopRun();

    return 0;
}

#endif

/**
 * Get an error message from a JAVA_* error value.
 *
 * @param code The error value returned by load_jre or start_java.
 * @return An error message; the caller should not attempt to either
 *         free or modify the buffer.
 */
const char *
get_java_error(int code)
{
    switch ( code ) {
    case JAVA_CLASS_NOT_FOUND: return "Cannot find Java class";
    case JAVA_METHOD_NOT_FOUND: return "Cannot find Java method";
    case JAVA_OUT_OF_MEMORY: return "Cannot allocate Java objects";
    case JAVA_SYMBOL_NOT_FOUND: return "Cannot find JNI symbol";
    case JAVA_CREATE_VM_ERROR: return "Cannot create Java virtual machine";
    case JAVA_CREATE_THREAD_ERROR: return "Cannot create Java thread";
    case JAVA_DLOPEN_ERROR: return dlerror();
    default: return "Unknown error";
    }
}

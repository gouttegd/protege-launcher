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

#ifndef ICP20221226_JAVA_H
#define ICP20221226_JAVA_H

#include <jni.h>

#if defined(PROTEGE_LINUX)
#define JAVA_LIB_PATH "/lib/server/libjvm.so"
#define BUNDLED_JAVA_LIB_PATH "/jre" JAVA_LIB_PATH
#elif defined(PROTEGE_MACOS)
#define JAVA_LIB_PATH "/lib/jli/libjli.dylib"
#define BUNDLED_JAVA_LIB_PATH "/jre" JAVA_LIB_PATH
#elif defined(PROTEGE_WIN32)
#define JAVA_LIB_PATH "\\bin\\server\\jvm.dll"
#define BUNDLED_JAVA_LIB_PATH "\\jre" JAVA_LIB_PATH
#endif

#define JAVA_CLASS_NOT_FOUND        -1
#define JAVA_METHOD_NOT_FOUND       -2
#define JAVA_OUT_OF_MEMORY          -3
#define JAVA_SYMBOL_NOT_FOUND       -4
#define JAVA_CREATE_VM_ERROR        -5
#define JAVA_CREATE_THREAD_ERROR    -6
#define JAVA_DLOPEN_ERROR           -7

#ifdef __cplusplus
extern "C" {
#endif

int
load_jre(const char *path, int bundled, void **jre);

int
start_java(void        *jre,
           const char **vm_args,
           const char  *main_class_name,
           const char **main_args);

const char *
get_java_error(int code);

#ifdef __cplusplus
}
#endif

#endif /* !ICP20221226_JAVA_H */

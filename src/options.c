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

#include "options.h"

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include <xmem.h>

#include "util.h"

#if defined(PROTEGE_WIN32)
#define JAVA_CLASSPATH_SEPARATOR ";"
#else
#define JAVA_CLASSPATH_SEPARATOR ":"
#endif

/*
 * Default options. They are needed for Protégé to start and run
 * correctly and are always used.
 */
static const char *default_options[] = {
    "-Dlogback.configurationFile=conf/logback.xml",
    "-DentityExpansionLimit=100000000",
    "-Dfile.encoding=UTF-8",
    "-XX:CompileCommand=exclude,javax/swing/text/GlyphView,getBreakSpot",
#if defined(PROTEGE_MACOS)
    "-Dapple.laf.useScreenMenuBar=true",
    "-Dcom.apple.mrj.application.apple.menu.about.name=Protege",
    "-Xdock:name=Protege",
    "-Xdock:icon=Resources/Protege.icns",
#endif
    "-Djava.class.path=bundles/guava.jar"
      JAVA_CLASSPATH_SEPARATOR "bundles/logback-classic.jar"
      JAVA_CLASSPATH_SEPARATOR "bundles/logback-core.jar"
      JAVA_CLASSPATH_SEPARATOR "bundles/slf4j-api.jar"
      JAVA_CLASSPATH_SEPARATOR "bundles/glassfish-corba-orb.jar"
      JAVA_CLASSPATH_SEPARATOR "bundles/org.apache.felix.main.jar"
      JAVA_CLASSPATH_SEPARATOR "bundles/maven-artifact.jar"
      JAVA_CLASSPATH_SEPARATOR "bundles/protege-launcher.jar",
    NULL
};

static size_t n_default_options = sizeof(default_options) / sizeof(char**) - 1;

/*
 * Fill the list structure with the default options.
 * The string themselves are *not* duplicated, the list will only
 * contain a pointer to them.
 */
static void
copy_default_options(struct option_list *list)
{
    size_t n;

    list->allocated = n_default_options + 10;
    list->options = xmalloc(list->allocated * sizeof(char *));
    for ( n = 0; default_options[n]; n++ )
        list->options[n] = (char *)default_options[n];
    list->count = n;
}

/*
 * Append one option to the options list.
 */
static void
append_option(struct option_list *list, char *option)
{
    /* If it's the first non-default option that we add, we need to
     * fill the list with the default options first. */
    if ( ! list->allocated )
        copy_default_options(list);

    /* Grow the list as needed. */
    if ( list->count >= list->allocated - 1 ) {
        list->allocated += 10;
        list->options = xrealloc(list->options, list->allocated);
    }

    /* Append the option. */
    list->options[list->count++] = option;
    list->options[list->count] = NULL;
}

/*
 * Try to find a jvm.conf configuration file. We look into the user's
 * home directory, then into the Protégé directory.
 *
 * Returns a newly allocated buffer containing the path to the
 * configuration file that was found, or NULL if we didn't find any.
 */
static char *
find_configuration_file(const char *app_dir)
{
    char *home, *conf_file = NULL;
    struct stat statbuf;

    if ( (home = getenv("HOME")) ) {
        (void) xasprintf(&conf_file, "%s/.protege/conf/jvm.conf", home);
        if ( stat(conf_file, &statbuf) == -1 ) {
            free(conf_file);
            conf_file = NULL;
        }
    }

    if ( ! conf_file ) {
        (void) xasprintf(&conf_file, "%s/conf/jvm.conf", app_dir);
        if ( stat(conf_file, &statbuf) == -1 ) {
            free(conf_file);
            conf_file = NULL;
        }
    }

    return conf_file;
}

#if defined(PROTEGE_MACOS)

#include <CoreFoundation/CoreFoundation.h>

/*
 * Expand the list of options with options found in a JVMOptions key in
 * the application's Info.plist file.
 *
 * This is the "legacy" method of passing Java options on macOS.
 */
static void
get_options_from_bundle(struct option_list *list)
{
    CFBundleRef main_bundle;
    CFDictionaryRef info_dict;
    CFArrayRef jvmopts_array;
    CFIndex length, i;

    if ( ! (main_bundle = CFBundleGetMainBundle()) )
        return;

    if ( ! (info_dict = CFBundleGetInfoDictionary(main_bundle)) )
        return;

    if ( ! (jvmopts_array = (CFArrayRef)CFDictionaryGetValue(info_dict, CFSTR("JVMOptions"))) )
        return;

    length = CFArrayGetCount(jvmopts_array);
    for ( i = 0; i < length; i++ ) {
        CFStringRef option = CFArrayGetValueAtIndex(jvmopts_array, i);
        if ( CFStringHasPrefix(option, CFSTR("-")) ) {
            CFIndex option_length;
            char *option_string;

            option_length = CFStringGetLength(option);
            option_string = xmalloc(option_length + 1);
            CFStringGetCString(option, option_string, option_length + 1,
                               kCFStringEncodingMacRoman);

            append_option(list, option_string);
        }
    }
}

#elif defined(PROTEGE_WIN32)

/*
 * Expand the list of options with options found in a Protege.l4j.ini
 * file in Protégé directory.
 *
 * This is the "legacy" method of passing Java options on Windows.
 */
static void
get_options_from_l4j_file(const char *app_dir, struct option_list *list)
{
    char *l4j_filename, line[100];
    FILE *l4j_file;
    ssize_t n;

    (void) xasprintf(&l4j_filename, "%s\\Protege.l4j.ini", app_dir);
    if ( (l4j_file = fopen(l4j_filename, "r")) ) {
        while ( ! feof(l4j_file) ) {
            if ( (n = get_line(l4j_file, line, sizeof(line))) > 0 ) {
                if ( line[0] == '-' )
                    append_option(list, xstrdup(line));
            }
        }

        fclose(l4j_file);
    }

    free(l4j_filename);
}

#endif

/**
 * Get a list of all options that should be passed to the Java virtual
 * machine.
 *
 * This function returns at least a set of hard-coded default options.
 * In addition, it attempts to get user-specified additional options
 * from a 'jvm.conf' file (located either in ~/.protege/conf/jvm.conf'
 * or in $app_dir/conf/jvm.conf) and from "legacy" locations on macOS
 * and Windows.
 *
 * @param app_dir The directory where Protégé is installed.
 * @param list    A pointer to a structure to be filled with the
 *                complete set of options. The contents of that
 *                structure should be free with free_option_list.
 */
void
get_option_list(const char *app_dir, struct option_list *list)
{
    char *conf_file;

    /*
     * At first we make the list point to the static list of default
     * options. That way we don't need to allocate anything if there is
     * no additional options.
     */
    list->allocated = 0;
    list->count = n_default_options;
    list->options = (char **) default_options;

    /* Then we look for a configuration file with extra options. */
    if ( (conf_file = find_configuration_file(app_dir)) ) {
        FILE *f;
        char line[100], *opt_value, *opt_string;
        ssize_t n;

        if ( (f = fopen(conf_file, "r")) ) {

            while ( ! feof(f) ) {
                if ( (n = get_line(f, line, sizeof(line))) > 0 ) {
                    if ( line[0] == '#' )
                        continue;

                    if ( ! (opt_value = strchr(line, '=')) )
                        continue;

                    *opt_value++ = '\0';
                    opt_string = NULL;
                    if ( strcmp(line, "max_heap_size") == 0 )
                        (void) xasprintf(&opt_string, "-Xmx%s", opt_value);
                    else if ( strcmp(line, "min_heap_size") == 0 )
                        (void) xasprintf(&opt_string, "-Xms%s", opt_value);
                    else if ( strcmp(line, "stack_size") == 0 )
                        (void) xasprintf(&opt_string, "-Xss%s", opt_value);
                    else if ( strcmp(line, "append") == 0 )
                        opt_string = xstrdup(opt_value);

                    if ( opt_string )
                        append_option(list, opt_string);
                }
            }

            fclose(f);
        }

        free(conf_file);
    }
    /*
     * Without a configuration file, on macOS and Windows we fallback to
     * some "legacy" locations where options used to be found in older
     * versions of Protégé.
     */
#if defined(PROTEGE_MACOS)
    else
        get_options_from_bundle(list);
#elif defined(PROTEGE_WIN32)
    else
        get_options_from_l4j_file(app_dir, list);
#endif
}

/**
 * Free the list of options.
 *
 * @param list The options list to free.
 */
void
free_option_list(struct option_list *list)
{
    /* Nothing to free if the list only contains the default options. */
    if ( list->allocated ) {
        size_t n;

        /* Free the non-default, additional options. */
        for ( n = n_default_options; list->options[n]; n++ )
            free(list->options[n]);

        /* Free the list itself. */
        free(list->options);
    }
}

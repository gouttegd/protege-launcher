#!/bin/bash
# JRE helper for Protégé
# © 2024 Damien Goutte-Gattat
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

set -e

workdir=
local_repo=
java_orig_version=
java_version=
do_install=0

die() {
    echo "{0##*/}: $@" >&2
    exit 1
}

make_archive_name() {
    # $1: system
    # $2: is arm64?
    case $1 in
        linux)
            echo OpenJDK11U-jre_x64_linux_hotspot_$java_version.tar.gz
            ;;
        os-x)
            if [ $2 -eq 1 ]; then
                echo OpenJDK11U-jre_aarch64_mac_hotspot_$java_version.tar.gz
            else
                echo OpenJDK11U-jre_x64_mac_hotspot_$java_version.tar.gz
            fi
            ;;
        win)
            echo OpenJDK11U-jre_x64_windows_hotspot_$java_version.zip
            ;;
    esac
}

download() {
    # $1: archive name
    if [ ! -f $1 ]; then
        curl -L -O https://github.com/adoptium/temurin11-binaries/releases/download/jdk-$java_orig_version/$1
    fi
}

extract() {
    # $1: archive file
    case $1 in
        *.tar.?z|*.t?z)
            tar xf $1
        ;;
        *.zip)
            unzip $1
        ;;
    esac
}

while [ -n "$1" ]; do
    case "$1" in
    -h|--help)
        echo "Usage: $0 [--workdir DIR] [--install] [--repository PATH] <JAVA_VERSION>"
        exit 0
        ;;

    -w|--workdir)
        [ -n "$2" ] || die "Missing argument for --workdir"
        workdir=$2
        shift 2
        ;;

    -i|--install)
        do_install=1
        shift
        ;;

    -r|--repository)
        [ -n "$2" ] || die "Missing argument for --repository"
        local_repo=$2
        shift 2
        ;;

    *)
        java_orig_version=$1
        java_version=${1//+/_}
        shift
        ;;
        
    esac
done

[ -n "$java_version" ] || die "No Java version specified"

if [ -n "$workdir" ]; then
    [ ! -d "$workdir" ] || die "--workdir directory already exists!"
    mkdir -p "$workdir"
    cd $workdir
fi

echo "Preparing GNU/Linux and Windows JRE..."
for os in linux win ; do
    if [ ! -f jre.$os-$java_version.jar ]; then
        mkdir -p $os
        (cd $os
         archive=$(make_archive_name $os)
         download $archive
         extract $archive
         mv jdk-$java_orig_version-jre jre
         jar --create --file ../jre.$os-$java_version.jar jre
         rm -rf jre
        )
    fi
done

echo "Preparing universal MacOS JRE..."
if [ ! -f jre.os-x-$java_version.jar ]; then
    mkdir -p os-x
    (cd os-x
     x64_archive=$(make_archive_name os-x 0)
     arm_archive=$(make_archive_name os-x 1)
     download $x64_archive
     download $arm_archive
     mkdir x86_64 arm64 universal
     tar xf $x64_archive -C x86_64
     tar xf $arm_archive -C arm64

     find arm64 -type f | while read arm_file ; do
         noarch_file=${arm_file#arm64/}
         mkdir -p universal/${noarch_file%/*}
         if file $arm_file | grep 'Mach-O.\+arm64' ; then
             lipo -create -output universal/$noarch_file x86_64/$noarch_file $arm_file
             if file $arm_file | grep executable ; then
                 chmod 755 universal/$noarch_file
             fi
         else
             cp $arm_file universal/$noarch_file
         fi
     done

     (cd universal/jdk-$java_orig_version-jre/Contents
      rm -rf Info.plist MacOS _CodeSignature
      mv Home jre)
     jar --create --file ../jre.os-x-$java_version.jar -C universal/jdk-$java_orig_version-jre/Contents .
     rm -rf x86_64 arm64 universal
    )
fi

if [ $do_install -eq 1 ]; then
    repository_option=
    [ -n "$local_repo" ] && repository_option=-DlocalRepositoryPath=$local_repo/releases

    for os in linux os-x win ; do
        mvn org.apache.maven.plugins:maven-install-plugin:2.5.2:install-file \
            -Dfile=jre.$os-$java_version.jar -Dpackaging=jar \
            $repository_option -DcreateChecksum=true \
            -DgroupId=edu.stanford.protege -DartifactId=jre.$os -Dversion=$java_version
    done
fi

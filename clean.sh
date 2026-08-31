#!/bin/sh
# undo everything build.sh creates. No make.
set -e
TOP="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
cd "$TOP"

find . -name '*.o' -delete
rm -f include/applets.h include/usage.h
find . -mindepth 2 -name Config.in -delete
find . -mindepth 2 -name Kbuild -delete
rm -f applets/usage applets/usage.exe applets/applet_tables applets/applet_tables.exe applets/usage_pod applets/usage_pod.exe
rm -f include/BB_VER.h include/bbconfigopts.h include/bbconfigopts_bz2.h \
      include/common_bufsiz.h include/embedded_scripts.h \
      include/usage_compressed.h include/applet_tables.h include/NUM_APPLETS.h
rm -f busybox_unstripped.exe busybox_unstripped.exe.out busybox_unstripped.exe.map busybox.exe
echo "cleaned"

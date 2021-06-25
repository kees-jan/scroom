#! /bin/sh

# This script can be called on the directory to which scroom as
# installed on Windows and will remove unnecessary files in it and
# copy over required DLLs and themes. After executing this script,
# Scroom in the directory will be fully portable.

if [ $# -ne 1 ] || ! [ -f $1/bin/scroom.exe ] ; then
    echo 'Error: Need directory to which scroom was installed as the only arg'
    exit 1
fi

# exit when any command fails
set -e

mv $1/bin/* $1;
mv $1/share/scroom/scroom.glade $1/scroom.glade;
mkdir -p $1/plugins;
mv $1/lib/scroom/*.dll $1/plugins;
mkdir -p $1/colormaps;
rm -r $1/bin;
rm -r $1/lib;
rm -r $1/include;
rm -r $1/share;

mkdir -p $1/lib/gdk-pixbuf-2.0/
if ! cp -r /mingw64/lib/gdk-pixbuf-2.0/ $1/lib/; then
  echo "Warning: Failed to copy GTK theme";
fi

mkdir -p $1/share/glib-2.0/
if ! cp -r /mingw64/share/glib-2.0/ $1/share/; then
  echo "Warning: Failed to copy GTK theme";
fi

mkdir -p $1/share/icons/
if ! cp -r /mingw64/share/icons/ $1/share/; then
  echo "Warning: Failed to copy GTK theme";
fi
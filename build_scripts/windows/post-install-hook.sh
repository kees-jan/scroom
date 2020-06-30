#! /bin/sh
# This script can be called on the directory to which scroom as installed on Windows and will remove unnecessary files in it
# and copy over required DLLs and themes. After executing this script, Scroom in the directory will be fully portable.

if [ $# -ne 1 ] || ! [ -f $1/bin/scroom.exe ] ; then
    echo 'Error: Need directory to which scroom was installed as the only arg'
    exit 1
fi

# exit when any command fails
set -e

mv $1/bin/scroom.exe $1/scroom.exe;
mv $1/bin/* $1;
mv $1/share/scroom/scroom.glade $1/scroom.glade;
mkdir -p $1/plugins;
mv $1/lib/bin/* $1/plugins;
mkdir -p $1/colormaps;
rm -r $1/bin;
rm -r $1/lib;
rm -r $1/include;
rm -r $1/share;

cp /mingw64/bin/{\
libatk-1.0-0.dll,libboost_filesystem-mt.dll,libboost_program_options-mt.dll,libbz2-1.dll,libcairo-2.dll,libdatrie-1.dll,\
libexpat-1.dll,libffi-7.dll,libfontconfig-1.dll,libfreetype-6.dll,libfribidi-0.dll,libgcc_s_seh-1.dll,libgdk-win32-2.0-0.dll,\
libgdk_pixbuf-2.0-0.dll,libgio-2.0-0.dll,libglade-2.0-0.dll,libglib-2.0-0.dll,libgmodule-2.0-0.dll,libgobject-2.0-0.dll,libgraphite2.dll,\
libgtk-win32-2.0-0.dll,libharfbuzz-0.dll,libiconv-2.dll,libintl-8.dll,liblzma-5.dll,libpango-1.0-0.dll,libpangocairo-1.0-0.dll,\
libpangoft2-1.0-0.dll,libpangowin32-1.0-0.dll,libpcre-1.dll,libpixman-1-0.dll,libpng16-16.dll,libstdc++-6.dll,libthai-0.dll,\
libwinpthread-1.dll,libxml2-2.dll,zlib1.dll,libtiff-5.dll,libzstd.dll,libjpeg-8.dll,libboost_thread-mt.dll,libgthread-2.0-0.dll,\
libbrotlidec.dll,libbrotlicommon.dll\
} $1/;

mkdir -p $1/share/gtk-2.0;
if ! cp /mingw64/share/themes/MS-Windows/gtk-2.0/gtkrc $1/share/gtk-2.0/gtkrc; then
  echo "Warning: Failed to copy GTK theme";
fi;
mkdir -p $1/lib/gtk-2.0/2.10.0/engines;
if ! cp /mingw64/lib/gtk-2.0/2.10.0/engines/*.dll $1/lib/gtk-2.0/2.10.0/engines/; then
  echo "Warning: Failed to copy GTK theme";
fi

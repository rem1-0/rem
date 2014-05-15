vlclibdir = $(libdir)/vlc

vlclib_LTLIBRARIES = librem_plugin.la
librem_plugin_la_SOURCES = module.c macros.h rgb_image.h filter_sys.h \
                           filter_picture.h cpp_win32_ugly_fixup.h \
                           stats_engine.h stats_engine.cpp \
                           I420toRGB.h I420toRGB.c \
                           YUYVtoRGB.h YUYVtoRGB.c \
                           drawing.h drawing.c
librem_plugin_la_CFLAGS = $(VLC_PLUGIN_CFLAGS) \
                          -DMODULE_STRING=\"rem\" \
                          @INCLTDL@
librem_plugin_la_CXXFLAGS = $(VLC_PLUGIN_CFLAGS) \
                            -DMODULE_STRING=\"rem\" \
                            @INCLTDL@
librem_plugin_la_LIBADD = $(VLC_PLUGIN_LIBS) @LIBLTDL@
librem_plugin_la_LDFLAGS = -no-undefined -avoid-version -module \
                           -export-symbol-regex ^vlc_entry

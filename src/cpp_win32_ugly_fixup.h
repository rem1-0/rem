#ifndef CPP_WIN32_UGLY_FIXUP_H
#define CPP_WIN32_UGLY_FIXUP_H

// In vlc_threads.h, the function poll is used whereas it is not defined
// on Windows. With x86_64-w64-mingw32-gcc, there is just a warning,
// but with x86_64-w64-mingw32-g++, there is an error. Here is
// a workaround.

#if defined(__cplusplus) && defined(_WIN32)
struct pollfd;
int poll(struct pollfd*, unsigned long, int);
#endif

#endif

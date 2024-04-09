
#ifndef _FCNTL_H
#define _FCNTL_H

#define O_RDONLY                0
#define O_WRONLY                1
#define O_RDWR                  2
#define O_APPEND                0x0008
#define O_CREAT                 0x0200
#define O_TRUNC                 0x0400
#define O_EXCL                  0x0800
#define O_SYNC                  0x2000
#define O_NONBLOCK              0x4000
#define O_NOCTTY                0x8000

#define O_ACCMODE               (O_RDONLY | O_WRONLY | O_RDWR)

//extern int open (const char *, int, ...);
extern int open (const char * path, int flags,...);

#endif


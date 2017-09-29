#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#define OUTFILE_NAME	"capture.bmp"
#define COUNT_IGNORE	10	// frame count for initilize camera

#define IMAGE_WIDTH	640
#define IMAGE_HEIGHT	480

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define uchar unsigned char


/* sequence
*open_device();
*init_device();
*start_capturing();
*mainloop();
*stop_capturing();
*uninit_device();
*close_device();
*exit(EXIT_SUCCESS);
*/

static void open_device(void);
static void init_device(void);
static void start_capturing(void);
static void mainloop(void);
static void stop_capturing (void);
static void uninit_device(void);
static void close_device(void);
void captureBM(void);

#endif //capture.h

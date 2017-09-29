/*
 * V4L2 video simple capture one frame program Ver.1.00 by saibara
 *
 * Ver.1.00 simple write one yuv file.
 */
#include "capture.h"

struct buffer {
  void *                  start;
  size_t                  length;
};

static char *           dev_name        = "/dev/video0";
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;

static void errno_exit(const char *s)
{
  fprintf(stderr, "%s error %d, %s\n",s, errno, strerror(errno));
  exit(EXIT_FAILURE);
}

static int xioctl(int fd,int request,void *arg)
{
  int r;
  do{ r = ioctl(fd, request, arg); }
  while(-1 == r && EINTR == errno);
  return r;
}

void saveBitmapFile(char* fileName, uchar* R, uchar* G, uchar* B, int width, int height){
  FILE* fp;
  int i;

  fp = fopen(OUTFILE_NAME,"wb");
  if(fp == NULL) return;

  //file header
  fprintf(fp, "BM");
  //If (IMAGE_WIDTH * 3) is not the multiple of 4, some 0(s) must be added to make it multiple of 4.
  uint32_t fileSize = 14 + 40 + (IMAGE_WIDTH*3)*IMAGE_HEIGHT;

  fwrite(&fileSize, 4, 1, fp);
  uint32_t reserved = 0;
  fwrite(&reserved, 4, 1, fp);
  uint32_t offset = 54;
  fwrite(&offset, 4, 1, fp);

  //information header
  uint32_t size = 40;
  fwrite(&size, 4, 1, fp);
  int32_t tWidth =(int32_t)width;
  fwrite(&tWidth, 4, 1, fp);
  int32_t tHeight = (int32_t)height;
  fwrite(&tHeight, 4, 1, fp);
  uint16_t plane = 1;
  fwrite(&plane, 2, 1, fp);
  uint16_t bit = 24;
  fwrite(&bit, 2, 1, fp);
  uint32_t compression = 0;
  fwrite(&compression, 4, 1, fp);
  //No additional 0(s);
  size = (IMAGE_WIDTH*3)*IMAGE_HEIGHT;
  fwrite(&size, 4, 1, fp);
  uint32_t zero = 0;
  fwrite(&zero, 4, 1, fp);
  fwrite(&zero, 4, 1, fp);
  fwrite(&zero, 4, 1, fp);
  fwrite(&zero, 4, 1, fp);

  //image data
  //no additional 0(s)
  for(i = 0; i < IMAGE_WIDTH*IMAGE_HEIGHT; i++){
    fwrite(B+i, 1, 1, fp);
    fwrite(G+i, 1, 1, fp);
    fwrite(R+i, 1, 1, fp);
  }

  fclose(fp);
  printf("Saved\n");

  return;
}

static void process_image(const void *p_buf,const int len_buf)
{
  int i, width, height;
  uchar *d1,*d2,buf;
  uchar Y[IMAGE_WIDTH * IMAGE_HEIGHT];
  uchar Cb[IMAGE_WIDTH * IMAGE_HEIGHT];
  uchar Cr[IMAGE_WIDTH * IMAGE_HEIGHT];
  uchar R[IMAGE_WIDTH * IMAGE_HEIGHT];
  uchar G[IMAGE_WIDTH * IMAGE_HEIGHT];
  uchar B[IMAGE_WIDTH * IMAGE_HEIGHT];

  printf("Capture size : %d\n",len_buf);

  // mmm... swap byte order
  /*
  for(i=0;i<len_buf;i+=2)
    {
      d1 = (char *)(p_buf) + i;
      d2 = (char *)(p_buf) + i + 1;
      buf = *d1;
      *d1 = *d2;
      *d2 = buf;
    }
  */
  //The format must be Y0CbY1Cr
  d1 = (uchar *)(p_buf);
  int index = 0;
  for(height = 0; height < IMAGE_HEIGHT; height++){
    for(width = 0; width < IMAGE_WIDTH; width += 2){
      Y[index] = *d1;
      d1++;
      Cb[index] = *d1;
      index++;
      Cb[index] = *d1;
      d1++;
      Y[index] = *d1;
      d1++;
      Cr[index-1] = *d1;
      Cr[index] = *d1;
      d1++;
      index++;
    }
  }

  int imageSize = IMAGE_WIDTH*IMAGE_HEIGHT;
  for(index = 0; index < imageSize; index++){
    double U = Cb[index] - 128;
    double V = Cr[index] - 128;
    R[index] = (uchar)(Y[index] + 1.402*V);
    G[index] = (uchar)(Y[index] - 0.344*U -0.714*V);
    B[index] = (uchar)(Y[index] + 1.772*U);
    //printf("%d %d %d\n", R[index], G[index], B[index]);
  }

  saveBitmapFile(OUTFILE_NAME, R, G, B, IMAGE_WIDTH, IMAGE_HEIGHT);

}

static int read_frame(int count)
{
  struct v4l2_buffer buf;

  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
      {
	switch(errno)
	    {
	    case EAGAIN:
	      return 0;
	    case EIO:
	      /* Could ignore EIO, see spec. */
	      /* fall through */
	    default:
	      errno_exit("VIDIOC_DQBUF");
	    }
      }
  assert(buf.index < n_buffers);
  if(count == 0)
    process_image(buffers[buf.index].start,buffers[buf.index].length);
  if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    errno_exit("VIDIOC_QBUF");

  return 1;
}

static void mainloop(void)
{
  unsigned int count;
  count = COUNT_IGNORE;
  while(count-- > 0)
    {
      for(;;)
	{
	  fd_set fds;
	  struct timeval tv;
	  int r;
	  FD_ZERO(&fds);
	  FD_SET(fd, &fds);
	  /* Timeout. */
	  tv.tv_sec = 2;
	  tv.tv_usec = 0;
	  r = select(fd + 1, &fds, NULL, NULL, &tv);
	  if(-1 == r)
	    {
	      if(EINTR == errno)
		continue;
	      errno_exit("select");
	    }
	  if(0 == r)
	    {
	      fprintf(stderr, "select timeout\n");
	      exit(EXIT_FAILURE);
	    }
	  if(read_frame(count))
	    break;
	  /* EAGAIN - continue select loop. */
	}
    }
}

static void stop_capturing (void)
{
  enum v4l2_buf_type type;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
    errno_exit("VIDIOC_STREAMOFF");
}

static void start_capturing(void)
{
  unsigned int i;
  enum v4l2_buf_type type;

  for(i = 0; i < n_buffers; ++i)
      {
	struct v4l2_buffer buf;
	CLEAR(buf);
	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory      = V4L2_MEMORY_MMAP;
	buf.index       = i;

	if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
	  errno_exit("VIDIOC_QBUF");
      }
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(-1 == xioctl(fd, VIDIOC_STREAMON, &type))
    errno_exit("VIDIOC_STREAMON");
}

static void uninit_device(void)
{
  unsigned int i;

  for(i = 0; i < n_buffers; ++i)
    if(-1 == munmap(buffers[i].start, buffers[i].length))
      errno_exit("munmap");
  free(buffers);
}

static void init_mmap(void)
{
  struct v4l2_requestbuffers req;

  CLEAR(req);

  req.count               = 8;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_MMAP;

  if(-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
      if(EINVAL == errno)
	{
	  fprintf(stderr, "%s does not support "
		  "memory mapping\n", dev_name);
	  exit(EXIT_FAILURE);
	}
      else
	{
	  errno_exit("VIDIOC_REQBUFS");
	}
    }
  if(req.count < 2)
    {
      fprintf(stderr, "Insufficient buffer memory on %s\n",
	      dev_name);
      exit(EXIT_FAILURE);
    }
  buffers = calloc(req.count, sizeof(*buffers));

  if(!buffers)
    {
      fprintf(stderr, "Out of memory\n");
      exit(EXIT_FAILURE);
    }

  for(n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
      struct v4l2_buffer buf;
      CLEAR(buf);
      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = n_buffers;

      if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
	errno_exit("VIDIOC_QUERYBUF");

      // printf("Buffer length : %d : %d\n",n_buffers,buf.length);

      buffers[n_buffers].length = buf.length;
      buffers[n_buffers].start =
	mmap(NULL /* start anywhere */,
	     buf.length,
	     PROT_READ | PROT_WRITE /* required */,
	     MAP_SHARED /* recommended */,
	     fd, buf.m.offset);

      if(MAP_FAILED == buffers[n_buffers].start)
	errno_exit("mmap");
    }
}

static void init_device(void)
{
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if(-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
      if(EINVAL == errno)
	{
	  fprintf(stderr, "%s is no V4L2 device\n",dev_name);
	  exit(EXIT_FAILURE);
	}
      else
	{
	  errno_exit("VIDIOC_QUERYCAP");
	}
    }
  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
      fprintf(stderr, "%s is no video capture device\n",dev_name);
      exit(EXIT_FAILURE);
    }
  if(!(cap.capabilities & V4L2_CAP_STREAMING))
      {
	fprintf(stderr, "%s does not support streaming i/o\n",
		dev_name);
	exit(EXIT_FAILURE);
      }
  /* Select video input, video standard and tune here. */
  CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if(0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
      crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      crop.c = cropcap.defrect; /* reset to default */

      if(-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
	{
	  switch(errno)
	    {
	    case EINVAL:
	      /* Cropping not supported. */
	      break;
	    default:
	      /* Errors ignored. */
	      break;
	    }
	}
    }
  else
    {
      /* Errors ignored. */
    }
  CLEAR(fmt);

  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = IMAGE_WIDTH;
  fmt.fmt.pix.height      = IMAGE_HEIGHT;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

  if(-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) errno_exit("VIDIOC_S_FMT");

  /* Note VIDIOC_S_FMT may change width and height. */

  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  //min = fmt.fmt.pix.width * 3;
  if(fmt.fmt.pix.bytesperline < min) fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if(fmt.fmt.pix.sizeimage < min) fmt.fmt.pix.sizeimage = min;

  printf("%d\n", fmt.fmt.pix.sizeimage);
  init_mmap();
}

static void close_device(void)
{
  if(-1 == close(fd)) errno_exit("close");
  fd = -1;
}

static void open_device(void)
{
  struct stat st;

  if(-1 == stat(dev_name, &st))
    {
      fprintf(stderr, "Cannot identify '%s': %d, %s\n",
	      dev_name, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

  if(!S_ISCHR(st.st_mode))
    {
      fprintf(stderr, "%s is no device\n", dev_name);
      exit(EXIT_FAILURE);
    }
  fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
  if(-1 == fd)
    {
      fprintf(stderr, "Cannot open '%s': %d, %s\n",
	      dev_name, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
}

/*
int main (int argc,char **argv)
{
  dev_name = "/dev/video0";

  open_device();
  init_device();
  start_capturing();
  mainloop();
  stop_capturing();
  uninit_device();
  close_device();
  exit(EXIT_SUCCESS);
  return 0;
}
*/
void captureBM(void)
{
  open_device();
  init_device();
  start_capturing();
  mainloop();
  stop_capturing();
  uninit_device();
  close_device();
}

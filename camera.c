#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>
#include <pthread.h>
#include "camera.h"





int camera_open(void)    //打开摄像头
{
        int fd;
        fd = open(CAMERA_DEVICE, O_RDWR, 0);
        if (fd < 0) {
                printf("Open %s failed...\n", CAMERA_DEVICE);
                return -1;
        }
        return fd;
}

struct v4l2_capability camera_capability(int camera_fd) //获取驱动信息
{
        struct v4l2_capability cap;
        int ret = ioctl(camera_fd, VIDIOC_QUERYCAP, &cap);
        if (ret < 0) {
                printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
                return;
        }
        // 打印驱动信息
        printf("Capability Informations:\n");
        printf(" driver: %s\n", cap.driver);
        printf(" card: %s\n", cap.card);
        printf(" bus_info: %s\n", cap.bus_info);
        printf(" version: %08X\n", cap.version);
        printf(" capabilities: %08X\n", cap.capabilities);
        printf("\n");
        
        return cap;
}

struct v4l2_format camera_setformat(int camera_fd)             //设置视频格式
{
        struct v4l2_format fmt;
        int ret;
        memset(&fmt, 0, sizeof(fmt));
        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = VIDEO_WIDTH;
        fmt.fmt.pix.height      = VIDEO_HEIGHT;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
        ret = ioctl(camera_fd, VIDIOC_S_FMT, &fmt);
        if (ret < 0) {
                printf("VIDIOC_S_FMT failed (%d)\n", ret);
                return;
        }
        return fmt;
}

void camera_getformat(int camera_fd,struct v4l2_format fmt)    //获取视频格式
{
        int ret;
        ret = ioctl(camera_fd, VIDIOC_G_FMT, &fmt);
        if (ret < 0) {
                printf("VIDIOC_G_FMT failed (%d)\n", ret);
                return;
        }
        // 打印视频格式信息
        printf("Stream Format Informations:\n");
        printf(" type: %d\n", fmt.type);
        printf(" width: %d\n", fmt.fmt.pix.width);
        printf(" height: %d\n", fmt.fmt.pix.height);
        char fmtstr[8];
        memset(fmtstr, 0, 8);
        memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
        printf(" pixelformat: %s\n", fmtstr);
        printf(" field: %d\n", fmt.fmt.pix.field);
        printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
        printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
        printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
        printf(" priv: %d\n", fmt.fmt.pix.priv);
        printf(" raw_date: %s\n", fmt.fmt.raw_data);
        printf("\n");
}

struct v4l2_requestbuffers camera_requestbuffers(int camera_fd)//请求分配内存
{
        struct v4l2_requestbuffers reqbuf;
        int ret;
        reqbuf.count = BUFFER_COUNT;
        reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        reqbuf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl(camera_fd , VIDIOC_REQBUFS, &reqbuf);
        if(ret < 0) {
                printf("VIDIOC_REQBUFS failed (%d)\n", ret);
                return;
        }
        return reqbuf;
}

struct v4l2_buffer camera_getbuffers(int camera_fd,struct v4l2_requestbuffers reqbuf)            //获取空间
{
        VideoBuffer*  buffers = calloc( reqbuf.count, sizeof(*buffers) );
        struct v4l2_buffer buf;
        int ret,i;
        for (i = 0; i < reqbuf.count; i++) 
        {
                buf.index = i;
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                ret = ioctl(camera_fd , VIDIOC_QUERYBUF, &buf);
                if(ret < 0) {
                        printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
                        return;
                }

                // mmap buffer
                framebuf[i].length = buf.length;
                framebuf[i].start = (char *) mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, camera_fd, buf.m.offset);
                if (framebuf[i].start == MAP_FAILED) {
                        printf("mmap (%d) failed: %s\n", i, strerror(errno));
                        return;
                }

                // Queen buffer
                ret = ioctl(camera_fd , VIDIOC_QBUF, &buf);
                if (ret < 0) {
                        printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
                        return;
                }

                printf("Frame buffer %d: address=0x%x, length=%d\n", i, (unsigned int)framebuf[i].start, framebuf[i].length);
        }
        
        
        return buf;
}

int camera_work(int camera_fd,struct v4l2_buffer buf)         //开始录制
{
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        int ret = ioctl(camera_fd, VIDIOC_STREAMON, &type);
        if (ret < 0) {
                printf("VIDIOC_STREAMON failed (%d)\n", ret);
                return 0;
        }
       
        while(1)
        {
                // Get frame
                ret = ioctl(camera_fd, VIDIOC_DQBUF, &buf);
                if (ret < 0) {
                        printf("VIDIOC_DQBUF failed (%d)\n", ret);
                        return 0;
                }
                pthread_mutex_lock(&mutex);
                memcpy(pic,framebuf[buf.index].start,framebuf[buf.index].length);
                //printf("pic head:%s\n",pic);
                pthread_mutex_unlock(&mutex);
                
                usleep(TIME_USLEEP);
                // Re-queen buffer
                ret = ioctl(camera_fd, VIDIOC_QBUF, &buf);
                if (ret < 0) {
                        printf("VIDIOC_QBUF failed (%d)\n", ret);
                        return 0;
                }
                
                
                
                
        }
                int i;
                // Release the resource
                for (i=0; i< 4; i++) 
                {
                        munmap(framebuf[i].start, framebuf[i].length);
                }

}

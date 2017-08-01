#ifndef CAMERA_H
#define CAMERA_H


#include <asm/types.h>        
#include <linux/videodev2.h>
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

#define CAMERA_DEVICE "/dev/video0"
#define CAPTURE_FILE "frame.jpg"
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define VIDEO_FORMAT V4L2_PIX_FMT_MJPEG
#define BUFFER_COUNT 4
#define TIME_USLEEP 120
#define POINT 8888
#define INIP "0.0.0.0"
#define N 10
#define M0_BUF "start"
#define CAMERA_OPEN "pic"
#define CAMERA_CLOSE "spg"
#define LOG_IN "dl"
#define LOG_UP "zc"
#define FAN_OPEN "fsk"
#define FAN_CLOSE "fsg"
#define BUZZ_OPEN "fmqk"
#define BUZZ_CLOSE "fmqg"
#define LED_OPEN "ledk"
#define LED_CLOSE "ledg"




typedef struct CameraInfomation{
       int fd;
       struct v4l2_capability cap; 
       struct v4l2_format fmt;
       struct v4l2_requestbuffers reqbuf;
       struct v4l2_buffer buf;
}CAINFO;

struct env
{
	char temp[10];
	char humidity[10];
	char light[10];
};



typedef struct VideoBuffer {
    void   *start;
    size_t  length;
} VideoBuffer;

pthread_mutex_t mutex;
char pic[307200];
CAINFO camera;


int m0_fd;

VideoBuffer framebuf[BUFFER_COUNT];  

int camera_open(void);                                           //打开摄像头
struct v4l2_capability camera_capability(int camera_fd);         //获取驱动信息
struct v4l2_format camera_setformat(int camera_fd);              //设置视频格式
void camera_getformat(int camera_fd,struct v4l2_format fmt);     //获取视频格式
struct v4l2_requestbuffers camera_requestbuffers(int camera_fd); //请求分配内存
struct v4l2_buffer camera_getbuffers(int camera_fd,struct v4l2_requestbuffers reqbuf);             //获取空间
int camera_work(int camera_fd,struct v4l2_buffer buf);          //开始录制


//M0
int open_port(void);
int set_opt(int nSpeed, int nBits, char nEvent, int nStop) ;
void m0_ctl(char cmd);
void m0_env(struct env *p);
int uart_init(void);

//注册登陆
/*
void user_opendb(void);

int user_login(int rws);

int user_logup(int rws);*/


#endif 

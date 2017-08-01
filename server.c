#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "camera.h"




//初始化服务器
int initserver(int *len)  
{
        //创建套结字socketfd
        int socketfd = socket(AF_INET,SOCK_STREAM,0);
        if(socketfd == -1)
        {
                printf("create socket fail...\n");
                return -1;
        }
        
        //设置套结字
        struct sockaddr_in server_addr;
        memset(&server_addr,0,sizeof(server_addr));
        server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(POINT);
	server_addr.sin_addr.s_addr = inet_addr(INIP);
	
	//绑定套结字
	*len = sizeof(server_addr);
	if(-1 == bind(socketfd, (struct sockaddr *)&server_addr, *len ) )
	{
		printf("bind socket fail...\n");
		return -1;
	}
	
	//监听套结字
	int ret = listen(socketfd,10);
	if (ret == -1)
	{
	        printf("listen socket fail...\n");
	        close(socketfd);
	        return -1;
	}
        printf("server ready...\n");
        return socketfd;
}

//摄像头线程程序
void *p_camera_work(void *p)
{  
        int ret = camera_work(camera.fd,camera.buf);  //开始录制 
        if (ret == 0)
        {
                printf("camera work fail..\n");
        } 
}


void *fun(void *p)
{
        printf("in fun\n");
        int rws = *(int*)p; 
        int ret = 0;
        char a;
        char buf[N];
        memset(buf,0,N);
        char n;
        struct env en;
	
        while(1)
        {
                //判断是获取M0消息还是获取摄像头
                memset(buf,0,N);
                bzero(&en, sizeof(en));
	        ret = read(rws,buf,N);
	        
	        
	        if(ret == 0)
	        {
	                close(rws);
	                printf("client close...\n");
	                return;
	        }
	        if(strcmp(CAMERA_OPEN,buf) == 0)
                {
                        if(camera.fd)
                        {
                                pthread_mutex_lock(&mutex);
                                ret = write(rws,pic,sizeof(pic));
                                if (ret < 0)
                                {
                                         printf("write pic fail...\n");
                                        return;
                                }
                                if(ret == 0)
                                {
                                    printf("camera close\n");
                                    close(rws);
                                    return;
                                }
                                
                                pthread_mutex_unlock(&mutex);
                                
                                usleep(TIME_USLEEP);
                        }     
                        
                }else if(strcmp(CAMERA_CLOSE,buf) == 0)
                {
                        printf("close camera\n");
                }else if(strcmp(LOG_IN,buf) == 0)
                {
                        //user_login(rws);
                }else if(strcmp(LOG_UP,buf) == 0)
                {
                        //user_logup(rws);
                }else if(strcmp(FAN_OPEN,buf) == 0)
                {
                        printf("cammand is:%s\n",buf);
                        memset(buf,0,N);
                        n = 0x16;
                        m0_ctl(n);
                }else if(strcmp(FAN_CLOSE,buf) == 0)
                {
                        printf("cammand is:%s\n",buf);
                        memset(buf,0,N);
                        n = 0x17;
                        m0_ctl(n);
                }else if(strcmp(BUZZ_OPEN,buf) == 0)
                {
                        printf("cammand is:%s\n",buf);
                        memset(buf,0,N);
                        n = 0x14;
                        m0_ctl(n);
                }else if(strcmp(BUZZ_CLOSE,buf) == 0)
                {
                        printf("cammand is:%s\n",buf);
                        memset(buf,0,N);
                        n = 0x15;
                        m0_ctl(n);
                }else if(strcmp(LED_OPEN,buf) == 0)
                {
                        printf("cammand is:%s\n",buf);
                        memset(buf,0,N);
                        n = 0x10;
                        m0_ctl(n);
                }else if(strcmp(LED_CLOSE,buf) == 0)
                {
                        printf("cammand is:%s\n",buf);
                        memset(buf,0,N);
                        n = 0x11;
                        m0_ctl(n);
                }else if(strcmp(M0_BUF,buf) == 0)
                {
                        printf("cammand is:%s\n",buf);
                        m0_env(&en);
                        
                        printf("temp:%s  ,humidity:%s  ,light:%s\n",en.temp,en.humidity,en.light);
                        
                        /*char info[30];
                        memset(info,0,30);
                        strcpy(info,en.temp);
                        strcat(info,en.humidity);
                        strcat(info,en.light);*/
                        ret = write(rws,&en,sizeof(en));
                        printf("ret:%d,size info:%d\n",ret,sizeof(en));       
                         if(ret < 0)
                        {
                                printf("write info fail..\n");
                        }
                }else 
                {
                        printf("cammand is:%s\n",buf);
                        printf("cammand is wrong...\n");
                }
                
        }
}

//主函数
int main()
{       
        char buf[N];
        memset(buf,0,N);
        memset(pic,0,sizeof(pic));
        int ret = -1;
        
        //初始化摄像头

        camera.fd = camera_open();//打开摄像头
        camera.cap = camera_capability(camera.fd);//获取驱动信息
        camera.fmt = camera_setformat(camera.fd); //设置视频格式
        camera_getformat(camera.fd,camera.fmt);    //获取视频格式
        camera.reqbuf = camera_requestbuffers(camera.fd);//请求分配内存
        camera.buf = camera_getbuffers(camera.fd,camera.reqbuf);   //获取空间
        
        
        
        
        //初始化互斥锁
        pthread_mutex_init(&mutex,NULL); 
        //创建线程
        
        
        //qidong shexiangtou
        pthread_t t;
        pthread_create(&t,NULL,p_camera_work,NULL);
        
        //初始化M0
        uart_init(); 
        
	
        //初始化服务器
        int len = 0;
        int socketfd = initserver(&len);
        if(socketfd == -1)
        {
                printf("initserver fail...\n");
                return -1;
        }
        
        //响应客户端
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
	
	
	while(1)
	{
	        memset(buf,0,N);
	        
	        int rws = accept(socketfd, (struct sockaddr*)&client_addr, &len);
	        if(rws == -1)
	        {
		        printf("accept fail...\n");
		        close(socketfd);
		        return -1;
	        }
	        printf("one user come...\n");
	        //printf("IP: %s\n", inet_ntoa( rws.sin_addr) );
	        pthread_t func;
	        pthread_create(&func,NULL,fun,&rws);
	        
	        char exit[10];
	        /*gets(exit);
	        if(strcmp(exit,"exit") == 0)
	        {
	                return;
	        }*/
	        
	        
	}
	
	
	close(m0_fd);
	close(camera.fd);
	close(socketfd);
        return 0;
}

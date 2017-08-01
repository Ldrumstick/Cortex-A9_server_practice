#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include<strings.h>
#include <pthread.h> 
#include "camera.h"



int open_port(void)  
{         
    m0_fd=open("/dev/ttyUSB0",O_RDWR | O_NOCTTY | O_NONBLOCK);
 
      
    if(m0_fd==-1)  
    {  
        perror("Can't Open SerialPort");  
    }  
      
    return m0_fd;  
}  
int set_opt(int nSpeed, int nBits, char nEvent, int nStop)   
{   
     struct termios newtio,oldtio;   

     if  ( tcgetattr( m0_fd,&oldtio)  !=  0) {    
      perror("SetupSerial 1");  
      printf("tcgetattr( fd,&oldtio) -> %d\n",tcgetattr( m0_fd,&oldtio));   
      return -1;   
     }   
     bzero( &newtio, sizeof( newtio ) );   

     newtio.c_cflag  |=  CLOCAL | CREAD;    
     newtio.c_cflag &= ~CSIZE;    
  
     switch( nBits )   
     {   
     case 7:   
      newtio.c_cflag |= CS7;   
      break;   
     case 8:   
      newtio.c_cflag |= CS8;   
      break;   
     }   

     switch( nEvent )   
     {   
     case 'o':  
     case 'O': 
      newtio.c_cflag |= PARENB;   
      newtio.c_cflag |= PARODD;   
      newtio.c_iflag |= (INPCK | ISTRIP);   
      break;   
     case 'e':  
     case 'E':  
      newtio.c_iflag |= (INPCK | ISTRIP);   
      newtio.c_cflag |= PARENB;   
      newtio.c_cflag &= ~PARODD;   
      break;  
     case 'n':  
     case 'N':   
      newtio.c_cflag &= ~PARENB;   
      break;  
     default:  
      break;  
     }   
  
switch( nSpeed )   
     {   
     case 2400:   
      cfsetispeed(&newtio, B2400);   
      cfsetospeed(&newtio, B2400);   
      break;   
     case 4800:   
      cfsetispeed(&newtio, B4800);   
      cfsetospeed(&newtio, B4800);   
      break;   
     case 9600:   
      cfsetispeed(&newtio, B9600);   
      cfsetospeed(&newtio, B9600);   
      break;   
     case 115200:   
      cfsetispeed(&newtio, B115200);   
      cfsetospeed(&newtio, B115200);   
      break;   
     case 460800:   
      cfsetispeed(&newtio, B460800);   
      cfsetospeed(&newtio, B460800);   
      break;   
     default:   
      cfsetispeed(&newtio, B9600);   
      cfsetospeed(&newtio, B9600);   
     break;   
     }   

     if( nStop == 1 )   
      newtio.c_cflag &=  ~CSTOPB;   
     else if ( nStop == 2 )   
      newtio.c_cflag |=  CSTOPB;   

     newtio.c_cc[VTIME]  = 0;   
     newtio.c_cc[VMIN] = 0;   

     tcflush(m0_fd,TCIFLUSH);   
  
if((tcsetattr(m0_fd,TCSANOW,&newtio))!=0)   
     {   
      perror("com set error");   
      return -1;   
     }   
     printf("line:%d,set done!\n",__LINE__);   
     return 0;   
}   

void m0_ctl(char cmd)
{	
	int ret = write(m0_fd,&cmd,1);
	char str[10];
	int count = 0;
	char isOK = '0';
	memset(str,0,10);
	str[0] = 'a';
	printf("-----------------ret:%d\n",ret);
	while(1)
	{	
	char c;	
	char n = read(m0_fd,&c,1);
		if(n==1)
		{
		        str[count] = c;
		        count ++;
			printf("line:%d,%c\n",__LINE__,c);
			if(c =='\n')
			{
				break;
				str[count - 1] = '\0';
			}
		}			
	}
	
	
	printf("---read------------\n");
	
}
void m0_env(struct env *p)
{	
	
	char cmd = 0x01;
	int ret;
	ret = write(m0_fd, &cmd,1);
	usleep(1000);
	printf("line:%d,ret=%d\n",__LINE__,ret);
	
	char c;
	int i=0;
	while(1)
	{
		ret = read(m0_fd, &c,1);
		if(ret == 1)
		{
			if(c == '\n')
			{
				i = 0;
				break;
			}
			p->temp[i] = c;
			i++;
		}	
		
	}
	cmd = 0x02;
	write(m0_fd, &cmd,1);
	ret = read(m0_fd, p->humidity ,100 );
	while(1)
	{
		ret = read(m0_fd, &c,1);
		if(ret == 1)
		{
			if(c == '\n')
			{
				i = 0;
				break;
			}
			p->humidity[i] = c;
			i++;
		}	
		
	}

	
	cmd = 0x03;
	write(m0_fd, &cmd,1);
	ret = read(m0_fd, p->light,100 );
	while(1)
	{
		ret = read(m0_fd, &c,1);
		if(ret == 1)
		{
			if(c == '\n')
			{
				i = 0;
				break;
			}
			p->light[i] = c;
			i++;
		}	
		
	}

	
}
int uart_init()
{
	open_port();
	set_opt(115200, 8, 'n', 1); 
}


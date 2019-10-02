#include "douyuAPI.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <errno.h>
#include <string.h>


#include "data.h"

typedef struct DOUYU_SOCKET
{
    int fd;
    unsigned int con_timeout;
    unsigned int write_timeout;
    unsigned int rev_timeout;
}DOUYU_SOCKET;



int douyusockt_init(void **handle,unsigned con_timeout,unsigned write_timeout,unsigned rev_timeout)
{
    int ret;
    if(handle == NULL)
    {
        ret = Sck_ErrParam;
        return ret;
    }
    DOUYU_SOCKET * myDOUYU;
    myDOUYU = (DOUYU_SOCKET *)malloc(sizeof(DOUYU_SOCKET));
    if(myDOUYU == NULL)
    {
        ret = Sck_ErrMalloc;
        return ret;
    }

    myDOUYU->con_timeout= con_timeout;
    myDOUYU->rev_timeout =rev_timeout;
    myDOUYU->write_timeout=write_timeout;

    

    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd == -1)
    {
       printf("socket error\n");
       return -1;
    }
    myDOUYU->fd = fd;

    *handle = myDOUYU;

    return 0;  /// 成功初始化handle
}


/**
*deactivate-nonblock-设置1/0为阻塞模式
*@fd：文件播符符*/
static int deactivate_nonblock(int fd)
{
 int ret =0 ;
 int flags = fcntl(fd,F_GETFL);
 if(flags ==-1)
	{
		ret = flags;
		printf("fcntl err \n");
		return ret;
	}
	flags&=~O_NONBLOCK;
	ret=fcntl(fd,F_SETFL,flags);
	if(ret ==-1)
	{
		ret = errno;
		printf(" deactivate_nonblock  fcntl err \n");
		return ret;
	}
}

/**
*deactivate-nonblock-设置1/0为非阻塞模式
*@fd：文件播符符*/
static int activate_nonblock(int fd)
{
 int ret=0;
 int flags = fcntl(fd,F_GETFL);
 if(flags ==-1)
	{
		ret = flags;
		printf("fcntl err \n");
		return ret;
	}
	flags|= O_NONBLOCK;
	ret=fcntl(fd,F_SETFL,flags);
	if(ret ==-1)
	{
		ret = errno;
		printf(" deactivate_nonblock  fcntl err \n");
		return ret;
	}
}


static int connect_tinmeout(int fd,struct sockaddr_in*addr,unsigned int wait_seconds)
{
 	int ret;
 	socklen_t addrlen=sizeof(struct sockaddr_in);

 	if(wait_seconds>0)
	activate_nonblock(fd);
    
	ret=connect(fd,(struct sockaddr*)addr,addrlen); ///先试试一次

	if(ret <0 && errno==EINPROGRESS)  ///0的话连接成功，
	{
		fd_set connect_fdset;   //select 操作
		struct timeval timeout;
		FD_ZERO(&connect_fdset);
		FD_SET(fd,&connect_fdset);
		timeout.tv_sec=wait_seconds;   ///秒数
		timeout.tv_usec=0;
						do
						{
								//一但连接建立，则套接字就可写所以connect_fdset放在了写集合中
								ret=select(fd+1,NULL,&connect_fdset,NULL,&timeout);
						}while(ret<0&&errno==EINTR);
				
						if(ret==0)
					{
					ret=-1;
					errno=ETIMEDOUT;
					}
					else if(ret<0 )  //select 出错
					return -1;

		else if(ret==1)  ///可以写套接字，接到了
		{
										//printf（"222222222222222\n"）；
								/*ret返回为1（表示套接字可写），可能有两种情况，一种是连接建立成功，一种是套接字产生错误，*
								/*此时错误信息不会葆荐errno变量中，因此，需要调用getsockopt来获取。*"*/
								int err;
								socklen_t socklen = sizeof(err);
								int sockoptret =getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&socklen);
								if(sockoptret ==1)
								{
								return -1;
								}
								
								if(err == 0)
								{
								ret = 0;   //成功
								}
								else
								{
								errno =err;
								ret = -1;
                                }	
		}
	}
	 if (wait_seconds > 0)
		deactivate_nonblock (fd);	
		return ret;	 
}

int connect_douyu(void *handle,const char *ip,int port)
{
    int ret;
    if(handle == NULL ||ip==NULL||port<0 || port >65535)
    {
        ret = Sck_ErrParam;
        return ret;
    }
    DOUYU_SOCKET * myDOUYU = (DOUYU_SOCKET *)handle;
    ///拿到handle

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);	

    ret = connect_tinmeout(myDOUYU->fd,&addr,myDOUYU->con_timeout);

    if(ret <0)
    {
        if(ret == -1 && errno ==ETIMEDOUT)
            ret = Sck_ErrTimeOut;	
        else
                printf("connect_tinmeout err \n");
    }


    return ret;
}





/*
*write_timeout-写超时检测函数，不含写操作
*fd：文件描述符
*@wait_seconds：等待超时秒数，如果为0表示不检测超时
*成功（未超时）返回，失败返回-1，超时返回-1并且errno=ETIMEDOUT */
static int write_timeout(int fd, unsigned int wait_seconds)
{ 
		int ret=0;
		if(wait_seconds >0)
		{
		fd_set write_fdset;
		struct timeval timeout;
		FD_ZERO(&write_fdset);
		FD_SET(fd,&write_fdset);
		timeout.tv_sec=wait_seconds;
		timeout.tv_usec=0;
				do {
				ret=select(fd+1,NULL,&write_fdset,NULL,&timeout);
				}while(ret<0&&errno==EINTR);
		if(ret==0)
		{
		ret=-1;
		errno=ETIMEDOUT;
		}
		else if(ret==1)
		ret=0;
		}
		return ret;
}


static ssize_t readn(int fd,void*buf,size_t count)
{
	size_t nleft=count;
	ssize_t nread =0 ;
	char* bufp=(char*)buf;

		while(nleft> 0)
		{
			if((nread = read(fd,bufp,nleft))<0)
		      {	
		      	if(errno ==EINTR)
								continue;
								printf("err %d\n",errno);
								perror("read");
								return -1;
				
					}
			else if(nread == 0)//若对方已关闭
					return  count-nleft;
					
			bufp += nread;
			nleft-= nread;
		}
			return count;	
}


/*
1/1一次全部读走1/2次读完数据//出错分析//对方已关闭
/1思想：tcpip是流协议，不能1次把指定长度数据，全部驾完
//按照count大小写数据
//若读取的长鹰ssizekcount说明读到了一个结束符，对方已关闭。
//assize_t，返回写的长度-1失败
//abuf，待写数据首地址
//acount，待写长度*/
static ssize_t writen(int fd,const void* buf,size_t count)
{
	size_t nleft =count;
	ssize_t nwritten;
	char* pufp =(char*)buf;
	while(nleft >0)
	{
	if((nwritten = write(fd,pufp,nleft))<0)
		{
			if(errno = EINTR)
				continue;
				return -1;
		}
		
	else if(nwritten == 0) 
	continue;
	
	pufp +=nwritten;
	nleft-= nwritten;
	}
	return count;

}



int send_to_douyu(void *handle,unsigned char*data, int datalen)
{
    int ret;
    if(handle == NULL ||data == NULL)
    {
        ret = Sck_ErrParam;
        return ret;
    }

    DOUYU_SOCKET * myDOUYU = (DOUYU_SOCKET *)handle;
    ///拿到handle
   ret = write_timeout(myDOUYU->fd,myDOUYU->write_timeout);  ///测下能不能写


   if(ret == 0)
		{
			ssize_t writed = 0;
			/*
			unsigned char* netdata = (unsigned char*)malloc(datalen +8);
			if(netdata == NULL)
				{
					ret =Sck_ErrMalloc;
					//printf("netdata err  %d\n",ret);
					return ret;
				}
			memset(netdata,0,datalen+8);  ///制作结构体  前8位是长度
			long netlen =htonl(datalen);
			memcpy(netdata, (void *)&netlen, 8);
			memcpy(netdata +8 ,data,datalen);*/
			writed =writen(myDOUYU->fd ,data ,datalen);

			if(writed < (datalen))
				{
					/*
					if(netdata != NULL)
					{
						free(netdata);
						netdata =NULL;
					}
					*/
					return writed;   //还有多少没写完
				}
				/*
				if(netdata != NULL)
					{
						free(netdata);
						netdata =NULL;
					}*/
		}
	if(ret <0 )
		{
			if(ret == -1 && errno == ETIMEDOUT)
				{
					ret = Sck_ErrTimeOut;
					
				}
				else
				printf("write_timeout err %d\n",ret);
		}
		

    return ret;    


}





int read_timeout(int fd,unsigned int wait_seconds)
{
		int ret=0;
		if(wait_seconds>0)
		{
		fd_set read_fdset;
		struct timeval timeout;
		FD_ZERO(&read_fdset);
		FD_SET(fd,&read_fdset);
		timeout.tv_sec=wait_seconds;
		timeout.tv_usec=0;
		//select返回值三态
		//1若timeout时间到（超时），没有检测到读事件ret返回=0
		//2若ret返回<a&errno==EiNTR 说明seiect的过程中被别的信号中断（可中断睡眠原理）
		//2-1若返回-1，select出错
		do
		{
		ret=select(fd+1,&read_fdset,NULL,NULL,&timeout);
		}while(ret<0 & errno==EINTR); 
		
		
		if(ret==0)
		{
		ret=-1;
		 errno=ETIMEDOUT; 
		}
		else if(ret==1)
		ret=0; 
		}
		
		return ret;
}

int rev_from_douyu(void*handle, unsigned char* out,int* outlen)
{
    int ret;
    int datalen=0;
    if(handle == NULL ||out == NULL)
    {
        ret = Sck_ErrParam;
        return ret;
    }

    DOUYU_SOCKET * myDOUYU = (DOUYU_SOCKET *)handle;
    ///拿到handle

    ret = read_timeout(myDOUYU->fd,myDOUYU->rev_timeout);///看看能不能读


    if(ret == 0)  //可以读了
		{
			ssize_t readed = 0;		
			memset(out,0,*outlen);  //清空传来的buf
			readed = readn(myDOUYU->fd,&datalen,4);  ///先把多少个数据读出来
			if(readed < 4)
				{
					printf("readn 4  errdd对方已经关闭\n");
					ret =Sck_ErrPeerClosed;
					return ret;
				}
			//datalen = ntohs(datalen);
			readed = readn(myDOUYU->fd,out,datalen); //把数据全部读出来
			if(readed<datalen)
			{
					printf("readn 4 readn err\n");
					return readed;  ///返回读了多少个
			}		
			
		}
	if(ret <0 )
		{
			if(ret == -1 && errno == ETIMEDOUT)
				{
					ret = Sck_ErrTimeOut;
					
				}
				else
				printf("write_timeout err %d\n",ret);
		}


	*outlen = datalen; //把读了多少个传出去
	
	return ret ;
}


int keep_heart(void*handle,long int usecond)   ///保持心跳
{
    int ret;
    if(handle == NULL ||usecond <=0)
    {
        ret = Sck_ErrParam;
        return ret;
    }

	encoder enc;
	enc.add_item("type", "keeplive");
    enc.add_item("tick", usecond);

	int datalen;
	string  str  = enc.get_result(&datalen);

	ret =  send_to_douyu(handle,(unsigned char *)str.data(), datalen);

	if(ret == Sck_ErrTimeOut) //写超时返回
	{
		return ret;
	}
	return	 ret;
}


int login_room(void * handle)
{
	int ret;
    if(handle == NULL )
    {
        ret = Sck_ErrParam;
        return ret;
    }

	encoder enc;
	enc.add_item("type", "loginreq");
	//enc.add_item("roomid", 1126960);

	int datalen;
	string  str  = enc.get_result(&datalen);
	
	unsigned buf[100]={0};
	memcpy(buf,(unsigned char *)str.data(),datalen);


	ret =  send_to_douyu(handle,(unsigned char *)str.data(), datalen);

	if(ret == Sck_ErrTimeOut) //写超时返回
	{
		return ret;
	}

	return 0;
}


int join_room(void*handle,int room_id, int group_id )
{
	   int ret;
    if(handle == NULL ||room_id <=0)
    {
        ret = Sck_ErrParam;
        return ret;
    }

	encoder enc;
    enc.add_item("type", "joingroup");
    enc.add_item("rid", room_id);
    enc.add_item("gid", group_id);

	int datalen;
	string  str  = enc.get_result(&datalen);

	ret =  send_to_douyu(handle,(unsigned char *)str.data(), datalen);

	if(ret == Sck_ErrTimeOut) //写超时返回
	{
		return ret;
	}

	return 0;
}

int rev_data(void*handle, vector<key_value>& data,DY_MESSAGE_TYPE *type )
{
    int datalen=0;
	int ret=0;
	unsigned char buf[1024];
    if(handle == NULL )
    {
        ret = Sck_ErrParam;  
        return ret;
    }
	datalen = sizeof(buf);

	ret = rev_from_douyu(handle,buf,&datalen);
	{
		if(ret==Sck_ErrTimeOut)
		{
			printf("接收数据超时\n");
			return -1;
		}
	}
	encoder enc;
	*type = enc.get_rev_type(buf,"type");


	data =enc.arr;  ///把键值对返回出去
	
	return 0;

}
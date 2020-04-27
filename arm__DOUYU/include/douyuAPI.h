#ifndef DOUYUAPI_H
#define DOUYUAPI_H
#include "data.h"
#include "errno.h"
//错误码定义
#define Sck_0k 0
#define Sck_BaseErr 3000

#define Sck_ErrParam (Sck_BaseErr+1)
#define Sck_ErrTimeOut (Sck_BaseErr+2)
#define Sck_ErrPeerClosed (Sck_BaseErr+3)
#define Sck_ErrMalloc (Sck_BaseErr+4)

#ifdef __cplusplus
extern "C"
{
#endif

int douyusockt_init(void **handle,unsigned int con_timeout,unsigned int write_timeout,unsigned int rev_timeout);
int connect_douyu(void *handle,const char *ip,int port);
int send_to_douyu(void *handle,unsigned char*data, int datalen);
int rev_from_douyu(void*handle, unsigned char* out,int* outlen);
int keep_heart(void*handle,long int usecond);
int login_room(void*  handle,int roomid);
int join_room(void*handle,int room_id, int group_id );
int rev_data(void*handle, vector<key_value>& data,DY_MESSAGE_TYPE *type );
int douyusockt_destroy(void *handle);


#ifdef __cplusplus
}
#endif

#endif
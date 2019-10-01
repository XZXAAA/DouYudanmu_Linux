#include "data.h"
#include <stdio.h>
#include <string.h>
encoder::encoder(/* args */)
{
}

encoder::~encoder()
{
}


void encoder::add_item(const char *key, const char *value)
{
	while (*key != '\0')
	{
		if (*key == '/')
		{
			buf.append("@S", 2);
		}
		else if (*key == '@')
		{
			buf.append("@A", 2);
		}
		else
		{
			buf += *key;
		}

		key++;
	}

	buf.append("@=", 2);

	while (*value != '\0')
	{
		if (*value == '/')
		{
			buf.append("@S", 2);
		}
		else if (*value == '@')
		{
			buf.append("@A", 2);
		}
		else
		{
			buf += *value;
		}

		value++;
	}
    buf.append("/", 1);

}

void encoder::add_item(const char *key, const int value)
{
	char str[64] = {0};
	snprintf(str, sizeof(str), "%d", value);
	add_item(key, static_cast<const char *>(str));
}


string encoder::get_result(int *size)
{
 

    char c = '\0';
    buf.append((const char *)&c, 1);        //should end with '\0'
    
    string str = pack_header(buf); /// 对数据打包成斗鱼协议包

    *size = str.size();

    return str;
}


string encoder::pack_header(string data_str)
{
    string pack_str;

    int data_len = data_str.length() + 8;
    short msg_type = 689;     //client message type is 689
    char encrypt = 0;
    char reserve =0;
//    char buf[20]={0};
//printf("int %ld\n",sizeof(data_len));
    pack_str.append((const char *)&data_len, sizeof(data_len));      // 4 bytes is len
  //  memcpy(buf,(const char *)&data_len,sizeof(data_len));
    //printf("%s \n",buf);
	//	printf("int %u\n",pack_str.size());
    pack_str.append((const char *)&data_len, sizeof(data_len));      // 4 bytes is len
//		printf("int %u\n",pack_str.size());
    pack_str.append((const char *)&msg_type, sizeof(msg_type));      // 2 bytes is message type
//		printf("int %u\n",pack_str.size());
    pack_str.append((const char *)&encrypt, sizeof(encrypt));      // 1 bytes is encrypt
//		printf("int %u\n",pack_str.size());
    pack_str.append((const char *)&reserve, sizeof(reserve));      // 1 bytes is reserve
//		printf("int %u\n",pack_str.size());
    pack_str.append(data_str.data(), data_str.size());        //data
//	printf("int %ud\n",pack_str.size());
    return pack_str;
}


void encoder::parse(const unsigned char * data)
{
    arr.clear();

	if (*data == '\0')
	{
		return;
	}

	key_value kv;
	string buf;

	while (*data != '\0')
	{
		if (*data ==  '/')    //end char
		{

			kv.value = buf;
			buf.clear();

			arr.push_back(kv);

			kv.key.clear();
			kv.value.clear();

		}
		else if (*data == '@')
		{
			data++;

			if (*data == 'A')
			{
				buf += '@';
			}
			else if (*data == 'S')  // char '/'
			{
				buf += '/';
			}
			else if (*data == '=')  // key value separator
			{
				kv.key = buf;
				buf.clear();
			}
		}
		else
		{
			buf += *data;
		}

		data++;
	}

	if (*data == '\0' && *(data - 1) != '/') //末尾漏掉斜线/的情况
	{
		kv.value = buf;
		buf.clear();

		arr.push_back(kv);
	}
}



DY_MESSAGE_TYPE encoder::get_rev_type(unsigned char * buf,const char * type)
{
    DY_MESSAGE_TYPE ret;
    parse(buf+8);//前8个字节是长度，不要
    string value;
    value = fine_keyvalue(type);
     if(value == "chatmsg")
    {
        ret = MSG_TYPE_BARRAGE;
    }
    else if (value == "loginres")
    {
        ret = MSG_TYPE_LOGIN_RESPONSE;
    }

    return ret;

}

string encoder::fine_keyvalue(const char *key)
{
    string value;
    

	for (int i = 0; i < arr.size(); i++)
	{
		if(arr[i].key == key)
		{
			value = arr[i].value;
			break;
		}
	}

	return value;
}
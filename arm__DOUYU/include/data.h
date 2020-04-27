#ifndef DATA_H
#define DATA_H
#include <iostream>
#include <string>
using namespace std;
#include <vector>


typedef enum DY_MESSAGE_TYPE
{
    MSG_TYPE_BARRAGE = 1,       //barrage
    MSG_TYPE_LOGIN_RESPONSE = 2 ,        //login response
    UNKONW_TYPE =3
}DY_MESSAGE_TYPE;

struct key_value
{
    public:
    string key;
    string value;
};

class encoder
{
    
public:
    /* data */
    string buf;
public:

 vector<key_value> arr;
    encoder(/* args */);
    ~encoder();
void add_item(const char *key, const char *value);
void add_item(const char *key, const int value);
string get_result(int *size);
string pack_header(string data_str);




int parse(const unsigned char * data);
DY_MESSAGE_TYPE get_rev_type(unsigned char * buf ,const char * type);
string fine_keyvalue(const char *key);




};




#endif
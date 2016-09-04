
#pragma once
#ifndef V5_H
#define V5_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
//说明点
#pragma comment(lib, "ws2_32.lib")
//boost  
#include "boost/thread.hpp"  
#include "boost/asio.hpp"  

#include<fstream>
#include<cstring>

using namespace std;

struct head
{
	short int version;
	short int count;
	unsigned long SysUptime;
	unsigned long unix_secs;
	unsigned long unix_nsecs;
	unsigned long flow_sequence;
	char engine_type;
	char engine_id;
	short int sampling_interval;
};

struct flow
{
	unsigned long srcaddr;
	unsigned long dstaddr;
	unsigned long nexthop;
	short int input;
	short int output;
	unsigned long dPkts;
	unsigned long dOctets;
	unsigned long First;
	unsigned long Last;
	short int srcport;
	short int dstport;
	char pad1;
	char tcp_flags;
	char prot;
	char tos;
	short int src_as;
	short int dst_as;
	char src_mask;
	char dst_mask;
	short int pad2;
};
class V5
{
public:
	V5(){};
	V5(char * receive_buffer);
	void print(ostream &output)const;//输出到文件，用于<<输出运算符重载
	
private:
	void convert_head();//把头部的网络字节序列转换为本机序列
	void convert_flow(flow *a);//把flow的网络字节序列转换为本机序列
	
	head *rec_head;
	flow *rec_flow[40];
};

ostream & operator<<(ostream & output, const V5 & v);

#endif
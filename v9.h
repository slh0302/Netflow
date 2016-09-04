#pragma once
#ifndef V9_H
#define V9_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
//boost  
#include "boost/thread.hpp"  
#include "boost/asio.hpp"  
#include<fstream>
#include<cstring>
#include<vector>
#include<map>
#include<string>
#include<numeric>
#include<queue>
#include "get_config.h"
using namespace std;
/*****V9头部包含20字节*****/
struct head_v9
{
	short int version;
	short int count;
	unsigned long SysUptime;
	unsigned long unix_secs;
	unsigned long package_sequence;
	unsigned long source_id;
};

struct template_record
{
	short int template_id;
	short int field_count;
	queue<short int>field_type;
	queue<short int>field_length;
};

/***一条templateFlowSet 可能有N条template record****/
struct templateFlowSet_v9
{
	short int flowset_id;
	short int length;
	//vector<template_record> trd;
};

struct dataFlowSet_v9
{
	short int flowset_id;
	short int length;
	//vector<char*> dataRecord;
	char * dataRecord;
};

class v9
{
public:
	v9()
	{
		head = new head_v9;
		tfs = new templateFlowSet_v9;
	}
	void receive(char* const receive_buffer,string ip);
	void v9::print(ostream &  out)const;
	unsigned long GetIP();
private:
	head_v9 * head;
	string v9ip;
	templateFlowSet_v9 * tfs;
	//dataFlowSet_v9 * dfs;
	void convert_head();
	void convert_tfs();
	void convert_dfs(dataFlowSet_v9 & adfs);
	void handleFlowSet(char * nowlocate);
};
typedef map<int, string> SchTyMap;  
extern SchTyMap Search_Type;
extern queue<dataFlowSet_v9> dfs;
typedef map<string, template_record> tfsmap;
    //查找type序号对应的内容

extern tfsmap tmap;
ostream & operator<<(ostream & output, const v9 & v);
bool isIPaddress(int type);

#endif
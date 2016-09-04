#ifndef DATASTRUCT_H

#define DATASTRUCT_H

//boost
#include "boost\thread.hpp"  
#include "boost\asio.hpp"  
#include "boost\date_time.hpp"
#include "boost\thread\mutex.hpp"
#include "boost\timer.hpp"
//stl   
#include<map>
#include<string>
#include<queue>
#include<vector>
#include <iostream>  
#include <fstream>

#include "get_config.h"
using namespace std;
//gobal Var
extern boost::mutex iomute;//IO锁
extern boost::timer gobalTime;
extern boost::mutex iofile;
class IpAndPort{
public:
	IpAndPort(){
		ipAddress = "";
		ipPort = 0;
	}
	IpAndPort(std::string ipa, unsigned short ipp):
		ipAddress(ipa), ipPort(ipp)
	{
		ipAndPort = ipa + getPort();
	}
	bool operator==(const IpAndPort& other){
		if (this->ipAndPort==other.ipAndPort ){
			return true;
		}
		else return false;
	}
	bool operator<(const IpAndPort& other)const{
		if (this->ipAndPort<other.ipAndPort){
			return true;
		}
		else return false;
	}
	bool operator>(const IpAndPort& other)const{
		if (this->ipAndPort>other.ipAndPort){
			return true;
		}
		else return false;
	}
	std::string getIp() const{
		return this->ipAddress;
	}
	std::string getPort() const
	{
		int x = this->ipPort;
		char y[10] = {0};
		sprintf_s(y, "%d", x);
		return string(y);
	}
private:
	std::string ipAddress;
	unsigned short ipPort;
	std::string ipAndPort;
};

class ClientInfo
{
private:
	boost::mutex ClientInfoMute;//三种情况锁定表示：put  get  print
	boost::condition_variable_any con_get;

	//用于客户端上下限度处理————————未处理

	boost::condition_variable_any con_newput;
	
	int currentClient;//当前连接的客户端数目
	int capacity;//客户端最大数目，默认为50
	long long waitSecondM;//等待的最大时间 默认8s
	int printSecondP;
	int handleSecond;
	//存放数据
	std::map<IpAndPort, std::queue<char*>*> queuelist;
	//存放时间
	std::map<IpAndPort, int> IpInsertTime;
	//存放锁
	std::map<IpAndPort, boost::mutex*> mutexList;
public:
	ClientInfo() :currentClient(0), capacity(50),waitSecondM(8000),printSecondP(2),handleSecond(3){}
	ClientInfo(int cur, int ca,long long wait,int print,int handle) 
		:currentClient(cur), capacity(ca),waitSecondM(wait),printSecondP(print),handleSecond(handle){}
	~ClientInfo(){}
	//get and set函数
	int getCapacity(){
		return this->capacity;
	}
	void setCapacity(int ca){
		this->capacity=ca;
	}
	//bool 判断Queuelist是否为空
	bool isMapEmpty(){
		return queuelist.empty();
	}
	int getPrintSecondP(){
		return this->printSecondP;
	}
	int getHandleSecond(){
		return this->handleSecond;
	}
	int getIpInsertTime(IpAndPort temp){
		return IpInsertTime.find(temp)->second;
	}
	std::queue<char*>* remainInQueue(IpAndPort);
	//bool 判断QueueList是否含有IpAndPort
	bool haveIpAndPort(IpAndPort);
	//新建一个
	void newMapQueue(IpAndPort, std::queue<char*>*,int);
	//IpAndPort: 为了search
	//char* 数据放入
	void putCharData(IpAndPort,char*,int time);
	char* getDoAnalysis(IpAndPort);
	//减少MAP
	bool deteMapQueue(IpAndPort);
	//打印时间
	void printMessage();
};

extern ClientInfo CliInfo;
void ConsumerOfData(IpAndPort);
void ProduceOfData(IpAndPort, const char*, queue < char* >*, int, bool);
void ThreadTimeInfo();
bool doFileSave(char* rec,IpAndPort ip);
int GetUseLen(int version,int length);
void properSet();
#endif
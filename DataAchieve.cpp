#include "DataStruct.h"
#include "V5.h"
#include "v9.h"
/*********************
1.队列长度可控
2.几个参数设置从文件读取.....Y
3.两台电脑的IP只读取到一个....Y
不同IP的区分计算，点分十进制的转换.....Y
4.输出都去掉，锁不要太多
5.V9协议  处理点开其它协议的崩溃问题.....Y
6.分块.....Y
7.使用trace
*********************/
/**********************
*实现ClientInfo
*@
*@
***********************/
ClientInfo CliInfo;
boost::mutex iomute;//IO锁
boost::timer gobalTime;
boost::mutex iofile;
void ClientInfo::newMapQueue(IpAndPort tempIp, std::queue<char*>* newQue, int time){
	//阻塞，并且等待
	{
		boost::mutex::scoped_lock lock(ClientInfoMute);
		while (currentClient >= this->capacity){
			{//作用域
				boost::mutex::scoped_lock lock(iomute);
				cout << "客户端数已满" << endl;
			}
			con_newput.wait(ClientInfoMute);
		}
		currentClient++;
		queuelist.insert(pair<IpAndPort, queue<char*>*>(tempIp, newQue));
		IpInsertTime.insert(pair<IpAndPort, int>(tempIp, time));
		mutexList.insert(pair<IpAndPort, boost::mutex*>(tempIp, new boost::mutex()));
	}
}
void ClientInfo::putCharData(IpAndPort tempIP, char* tempdata, int time){
	map<IpAndPort, boost::mutex*>::iterator iterMutex;
	iterMutex = mutexList.find(tempIP);
	{
		//锁IP锁
		boost::mutex::scoped_lock lock(*(iterMutex->second));
		map<IpAndPort, std::queue<char*>*>::iterator iterMap;
		iterMap = queuelist.find(tempIP);
		map<IpAndPort, int>::iterator iterTime;
		iterTime = IpInsertTime.find(tempIP);
		if (iterMap == queuelist.end() || iterTime == IpInsertTime.end()){
			return;//例行判断
		}
		else{
			//插入和修改信息
			iterTime->second = time;
			iterMap->second->push(tempdata);
		}
	}
	con_get.notify_all();//通知所有get
}
char* ClientInfo::getDoAnalysis(IpAndPort tempIP){
	char *receive_buffer;
	int tempTime;
	int countTime, elapsedTime = 0;

	map<IpAndPort, boost::mutex*>::iterator iterMutex;
	iterMutex = mutexList.find(tempIP);
	{
		boost::mutex::scoped_lock lock(*(iterMutex->second));
		map<IpAndPort, std::queue<char*>*>::iterator iterMap;
		iterMap = queuelist.find(tempIP);
		map<IpAndPort, int>::iterator iterTime;
		iterTime = IpInsertTime.find(tempIP);
		//int WaitCount = 0;
		if (iterMap == queuelist.end()){
			return "NOTFIND";//例行判断
		}

		while (iterMap->second->empty()){
			tempTime = (int)(gobalTime.elapsed() * 1000);
			//Handle
			{//作用域
				boost::mutex::scoped_lock lock(iomute);
				cout << iterMap->first.getIp() << " " << iterMap->first.getPort() << " "
					<< "的处理队列为空，现有时间" << tempTime << "ms" << endl;
			}
			/*******************************************
			*
			*判断空队列所等待的时间
			*
			*********************************************/
			//暂时交出ClientInfoMute
			//等待别人唤醒
			con_get.timed_wait(lock, boost::get_system_time() + boost::posix_time::milliseconds(100));//wait(ClientInfoMute);//每次等500ms
			tempTime = (int)(gobalTime.elapsed() * 1000);
			int remainx = iterTime->second;
			if ((tempTime - remainx) > waitSecondM){
				return "OVER";
			}
			//WaitCount++;
			//con_get.wait(ClientInfoMute);
		}
		countTime = (int)gobalTime.elapsed() * 1000;
		do{
			//线程只做取出的
			receive_buffer = iterMap->second->front();
			if (!doFileSave(receive_buffer, tempIP)){
				{
					boost::mutex::scoped_lock lock(iomute);
					cout << "File Save Wrong" << endl;
				}
			}
			delete[] iterMap->second->front();
			iterMap->second->pop();
			elapsedTime = (int)gobalTime.elapsed() - countTime;
		} while (elapsedTime < 1000 && !iterMap->second->empty());//线程运行两秒钟或者为空
	}//锁定应该结束
	///
	///HANDLE File

	/*for (int i = 0; i < 10000; i++){
		for (int j = 0; j < 10000; j++){}
		}*/
	return receive_buffer;
}
bool ClientInfo::deteMapQueue(IpAndPort tempIP){
	{
		/**************
		*执行M秒删除
		***************/
		boost::mutex::scoped_lock lock(ClientInfoMute);
		map<IpAndPort, std::queue<char*>*>::iterator iterMap;
		iterMap = queuelist.find(tempIP);
		map<IpAndPort, int>::iterator iterTime;
		iterTime = IpInsertTime.find(tempIP);
		if (iterMap == queuelist.end() || iterTime == IpInsertTime.end()){
			return false;//例行判断
		}
		else{
			queuelist.erase(tempIP);//删除
			IpInsertTime.erase(tempIP);
			mutexList.erase(tempIP);
			currentClient--;
		}
	}

	con_newput.notify_all();
	return true;
}
bool ClientInfo::haveIpAndPort(IpAndPort tempIP){
	{
		boost::mutex::scoped_lock lock(ClientInfoMute);
		map<IpAndPort, std::queue<char*>*>::iterator iterMap;
		iterMap = queuelist.find(tempIP);
		if (iterMap == queuelist.end()){
			return false;//例行判断
		}
		else{
			return true;
		}
	}
}
std::queue<char*>* ClientInfo::remainInQueue(IpAndPort tempIP){
	std::queue<char*>* x;
	{
		boost::mutex::scoped_lock lock(ClientInfoMute);

		map<IpAndPort, std::queue<char*>*>::iterator iterMap;
		iterMap = queuelist.find(tempIP);
		x = iterMap->second;//->size();
	}
	return x;
}
/***************
*Print 打印信息
****************/
void ClientInfo::printMessage() {
	//boost::mutex::scoped_lock lock(ClientInfoMute);
	{
		boost::mutex::scoped_lock lock(::iomute);
		cout << endl;
		cout << "当前连接的客户端数目: " << currentClient << endl;
		cout << "各客户端详细信息： " << endl;
		map<IpAndPort, std::queue<char*>*>::iterator it;//定义一个迭代指针it
		it = queuelist.begin();
		if (it == queuelist.end()) cout << "空" << endl;
		for (; it != queuelist.end(); ++it)
		{
			cout << it->first.getIp() + " " + it->first.getPort() + " " << "当前队列长度： " << CliInfo.remainInQueue(it->first)->size() << endl;
		}
		//test
		int PTime = (int)(gobalTime.elapsed() * 1000);
		cout << "当前时间: " << PTime << endl;
		cout << endl;
	}

}
/**********************
*实现生产者和消费者
*@
*@
***********************/
void ProduceOfData(IpAndPort tempIP, const char* tempdata, queue < char* >* tempList, int time, bool isNewComer){
	char* z = 0;
	if (isNewComer){
		CliInfo.newMapQueue(tempIP, tempList, time);
		//z = CliInfo.remainInQueue(tempIP);
		{
			boost::mutex::scoped_lock lock(::iomute);
			cout << tempIP.getIp() + " : " << tempIP.getPort() << "  " <<
				"is new Comer" << endl;
		}
		/**************************
		*
		*new Thread
		*new Consumer
		*
		***************************/
		boost::thread(ConsumerOfData, tempIP);
	}
	else{
		char *temp = new char[((int)tempdata[3]) * 50 + 30];
		memcpy(temp, tempdata, ((int)tempdata[3]) * 50 + 30);
		delete[] tempdata;
		CliInfo.putCharData(tempIP, temp, time);
		//z = CliInfo.remainInQueue(tempIP);
	}
}

void ConsumerOfData(IpAndPort tempIP){
	char* backInfo;
	while (true){
		//执行一次睡眠
		//boost::this_thread::sleep(boost::get_system_time() + boost::posix_time::seconds(CliInfo.getHandleSecond()));
		//
		backInfo = CliInfo.getDoAnalysis(tempIP);
		//继续执行的条件是阻塞结束
		if (string(backInfo) == string("OVER")){
			CliInfo.deteMapQueue(tempIP);
			{
				boost::mutex::scoped_lock lock(::iomute);
				cout << tempIP.getIp() + " " + tempIP.getPort() + " " + "线程结束" << endl;
			}
			return;//表示线程结束
		}
		else if (string(backInfo) == string("NOTFIND")){
			{
				boost::mutex::scoped_lock lock(::iomute);
				cout << tempIP.getIp() + " " + tempIP.getPort() + " " + "出错";
			}
			return;
		}
		//else{
		//	{
		//		boost::mutex::scoped_lock lock(iomute);
		//		cout << tempIP.getIp() + " " + tempIP.getPort() + " " << "还剩： " << CliInfo.remainInQueue(tempIP)->size()
		//			<< "上次有多少时间: " << CliInfo.getIpInsertTime(tempIP) << endl;
		//	}
		//}
	}
}
/*********************
*存储文件
**********************/
bool doFileSave(char* rec, IpAndPort tempIP){
	int version = (int)rec[1];
	//对于不同的IP仅有可能出现一次拿取
	//仅完成一次打开
	fstream output;
	output.open(tempIP.getIp() + " " + tempIP.getPort() + ".txt", ios::app);
	if (!output) return false;
	//打开文件
	if (version == 5){
		//boost::mutex::scoped_lock lock(iofile);
		V5 netflow_v5(rec);
		output << "V5 Begin" << endl;
		output << netflow_v5;
		output.close();
		return true;
	}
	else if (version == 9){
		v9 netFlowV9;
		//output << "V9  Begin" << endl;
		netFlowV9.receive(rec, tempIP.getIp() + tempIP.getPort());
		output << netFlowV9 << endl;
		output.close();
		return true;
	}
	else{
		return false;
	}
}
/************************
获得变化的缓冲区大小
*************************/
int GetUseLen(int version,int length){
	if (version == 1){
		return  30 + 50 * length;
	}
	else if (version== 5){
		return  30 + 50 * length;
	}
	else if (version == 9){
		//useLen=22+length*
		//存下来
		return  22 + length * 300;
	}
	else{
		return -1;
	}
}
/***************************
获得基本参数设置
****************************/

void properSet(){
	map<string, string> properFile;
	ReadConfig("./config/NetFlow.properties", properFile);
	map<string, string>::const_iterator mite;
	//暂时需要
	net_receive_buffer_size = atoi((properFile.find("net.receive.buffer.size")->second).c_str());
	net_bind_port = atoi(properFile.find("net.bind.port")->second.c_str());
	net_bind_host = properFile.find("net.bind.host")->second;
	flow_collector_V1_enabled = properFile.find("flow.collector.V1.enabled")->second == "true" ? true : false;
	flow_collector_V5_enabled = properFile.find("flow.collector.V5.enabled")->second == "true" ? true : false;
	flow_collector_V7_enabled = properFile.find("flow.collector.V7.enabled")->second == "true" ? true : false;
	flow_collector_V9_enabled = properFile.find("flow.collector.V9.enabled")->second == "true" ? true : false;
	//初始化的second
	flow_collector_statistics_interval = atoi(properFile.find("flow.collector.statistics.interval")->second.c_str());
	flow_collector_max_queue_length = atoi(properFile.find("flow.collector.max_queue_length")->second.c_str());
}
void ThreadTimeInfo(){
	while (1){
		boost::this_thread::sleep(boost::get_system_time() + boost::posix_time::seconds(CliInfo.getPrintSecondP()));
		CliInfo.printMessage();
	}
}//10.8.59.182
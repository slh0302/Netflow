#include "DataStruct.h"
#include "V5.h"
#include "v9.h"
/*********************
1.���г��ȿɿ�
2.�����������ô��ļ���ȡ.....Y
3.��̨���Ե�IPֻ��ȡ��һ��....Y
��ͬIP�����ּ��㣬���ʮ���Ƶ�ת��.....Y
4.�����ȥ��������Ҫ̫��
5.V9Э��  ����㿪����Э��ı�������.....Y
6.�ֿ�.....Y
7.ʹ��trace
*********************/
/**********************
*ʵ��ClientInfo
*@
*@
***********************/
ClientInfo CliInfo;
boost::mutex iomute;//IO��
boost::timer gobalTime;
boost::mutex iofile;
void ClientInfo::newMapQueue(IpAndPort tempIp, std::queue<char*>* newQue, int time){
	//���������ҵȴ�
	{
		boost::mutex::scoped_lock lock(ClientInfoMute);
		while (currentClient >= this->capacity){
			{//������
				boost::mutex::scoped_lock lock(iomute);
				cout << "�ͻ���������" << endl;
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
		//��IP��
		boost::mutex::scoped_lock lock(*(iterMutex->second));
		map<IpAndPort, std::queue<char*>*>::iterator iterMap;
		iterMap = queuelist.find(tempIP);
		map<IpAndPort, int>::iterator iterTime;
		iterTime = IpInsertTime.find(tempIP);
		if (iterMap == queuelist.end() || iterTime == IpInsertTime.end()){
			return;//�����ж�
		}
		else{
			//������޸���Ϣ
			iterTime->second = time;
			iterMap->second->push(tempdata);
		}
	}
	con_get.notify_all();//֪ͨ����get
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
			return "NOTFIND";//�����ж�
		}

		while (iterMap->second->empty()){
			tempTime = (int)(gobalTime.elapsed() * 1000);
			//Handle
			{//������
				boost::mutex::scoped_lock lock(iomute);
				cout << iterMap->first.getIp() << " " << iterMap->first.getPort() << " "
					<< "�Ĵ������Ϊ�գ�����ʱ��" << tempTime << "ms" << endl;
			}
			/*******************************************
			*
			*�жϿն������ȴ���ʱ��
			*
			*********************************************/
			//��ʱ����ClientInfoMute
			//�ȴ����˻���
			con_get.timed_wait(lock, boost::get_system_time() + boost::posix_time::milliseconds(100));//wait(ClientInfoMute);//ÿ�ε�500ms
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
			//�߳�ֻ��ȡ����
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
		} while (elapsedTime < 1000 && !iterMap->second->empty());//�߳����������ӻ���Ϊ��
	}//����Ӧ�ý���
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
		*ִ��M��ɾ��
		***************/
		boost::mutex::scoped_lock lock(ClientInfoMute);
		map<IpAndPort, std::queue<char*>*>::iterator iterMap;
		iterMap = queuelist.find(tempIP);
		map<IpAndPort, int>::iterator iterTime;
		iterTime = IpInsertTime.find(tempIP);
		if (iterMap == queuelist.end() || iterTime == IpInsertTime.end()){
			return false;//�����ж�
		}
		else{
			queuelist.erase(tempIP);//ɾ��
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
			return false;//�����ж�
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
*Print ��ӡ��Ϣ
****************/
void ClientInfo::printMessage() {
	//boost::mutex::scoped_lock lock(ClientInfoMute);
	{
		boost::mutex::scoped_lock lock(::iomute);
		cout << endl;
		cout << "��ǰ���ӵĿͻ�����Ŀ: " << currentClient << endl;
		cout << "���ͻ�����ϸ��Ϣ�� " << endl;
		map<IpAndPort, std::queue<char*>*>::iterator it;//����һ������ָ��it
		it = queuelist.begin();
		if (it == queuelist.end()) cout << "��" << endl;
		for (; it != queuelist.end(); ++it)
		{
			cout << it->first.getIp() + " " + it->first.getPort() + " " << "��ǰ���г��ȣ� " << CliInfo.remainInQueue(it->first)->size() << endl;
		}
		//test
		int PTime = (int)(gobalTime.elapsed() * 1000);
		cout << "��ǰʱ��: " << PTime << endl;
		cout << endl;
	}

}
/**********************
*ʵ�������ߺ�������
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
		//ִ��һ��˯��
		//boost::this_thread::sleep(boost::get_system_time() + boost::posix_time::seconds(CliInfo.getHandleSecond()));
		//
		backInfo = CliInfo.getDoAnalysis(tempIP);
		//����ִ�е���������������
		if (string(backInfo) == string("OVER")){
			CliInfo.deteMapQueue(tempIP);
			{
				boost::mutex::scoped_lock lock(::iomute);
				cout << tempIP.getIp() + " " + tempIP.getPort() + " " + "�߳̽���" << endl;
			}
			return;//��ʾ�߳̽���
		}
		else if (string(backInfo) == string("NOTFIND")){
			{
				boost::mutex::scoped_lock lock(::iomute);
				cout << tempIP.getIp() + " " + tempIP.getPort() + " " + "����";
			}
			return;
		}
		//else{
		//	{
		//		boost::mutex::scoped_lock lock(iomute);
		//		cout << tempIP.getIp() + " " + tempIP.getPort() + " " << "��ʣ�� " << CliInfo.remainInQueue(tempIP)->size()
		//			<< "�ϴ��ж���ʱ��: " << CliInfo.getIpInsertTime(tempIP) << endl;
		//	}
		//}
	}
}
/*********************
*�洢�ļ�
**********************/
bool doFileSave(char* rec, IpAndPort tempIP){
	int version = (int)rec[1];
	//���ڲ�ͬ��IP���п��ܳ���һ����ȡ
	//�����һ�δ�
	fstream output;
	output.open(tempIP.getIp() + " " + tempIP.getPort() + ".txt", ios::app);
	if (!output) return false;
	//���ļ�
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
��ñ仯�Ļ�������С
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
		//������
		return  22 + length * 300;
	}
	else{
		return -1;
	}
}
/***************************
��û�����������
****************************/

void properSet(){
	map<string, string> properFile;
	ReadConfig("./config/NetFlow.properties", properFile);
	map<string, string>::const_iterator mite;
	//��ʱ��Ҫ
	net_receive_buffer_size = atoi((properFile.find("net.receive.buffer.size")->second).c_str());
	net_bind_port = atoi(properFile.find("net.bind.port")->second.c_str());
	net_bind_host = properFile.find("net.bind.host")->second;
	flow_collector_V1_enabled = properFile.find("flow.collector.V1.enabled")->second == "true" ? true : false;
	flow_collector_V5_enabled = properFile.find("flow.collector.V5.enabled")->second == "true" ? true : false;
	flow_collector_V7_enabled = properFile.find("flow.collector.V7.enabled")->second == "true" ? true : false;
	flow_collector_V9_enabled = properFile.find("flow.collector.V9.enabled")->second == "true" ? true : false;
	//��ʼ����second
	flow_collector_statistics_interval = atoi(properFile.find("flow.collector.statistics.interval")->second.c_str());
	flow_collector_max_queue_length = atoi(properFile.find("flow.collector.max_queue_length")->second.c_str());
}
void ThreadTimeInfo(){
	while (1){
		boost::this_thread::sleep(boost::get_system_time() + boost::posix_time::seconds(CliInfo.getPrintSecondP()));
		CliInfo.printMessage();
	}
}//10.8.59.182
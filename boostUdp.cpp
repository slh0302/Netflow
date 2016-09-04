

//boost  

#include "DataStruct.h"

#include "v9.h"
//thread


int main()
{
	queue<char*>* listIP;//指针类型的队列方便随时new

	//properties File Read
	properSet();
	//file read 2
	ReadConfig("./config/v9.properties", Search_Type);

	//Inital
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket udp_socket(io_service);
	boost::asio::ip::udp::endpoint local_add(boost::asio::ip::address::from_string("0.0.0.0"), net_bind_port);
	udp_socket.open(local_add.protocol());//协议
	udp_socket.bind(local_add);
	//int x[1024];
	char* receive_buffer;
	char const_buffer[2048] = { '0' };

	/******************************************
	*Time Thread
	*P s/c
	********************************************/
	boost::thread te(&ThreadTimeInfo);
	while (true)
	{
		boost::asio::ip::udp::endpoint send_point;
		try{
			udp_socket.receive_from(boost::asio::buffer(const_buffer, 2048), send_point);
			//做内存初始化
			int length = (int)const_buffer[3];//获取长度
			int version_net = (int)const_buffer[1];//获取版本
			int useLen = 0;
			//uselen长度变化
			useLen = GetUseLen(version_net, length);
			if (useLen == -1)
				continue;
			receive_buffer = new char[useLen];
			memcpy(receive_buffer, const_buffer, useLen);
			//receive_from 会Block在这里
			//记录IP和PORT
			string strIp = send_point.address().to_string();
			unsigned short strPort = send_point.port();

			if (!CliInfo.haveIpAndPort(IpAndPort(strIp, strPort))){
				//not find : need a new queue
				cout << "未找到—————为新客户端" << " ";
				//创建新的队列
				listIP = new queue < char* > ;
				listIP->push(receive_buffer);//放入第一个元素
				//放入数据时所用的时间 ms 为单位
				int tempTime = (int)(gobalTime.elapsed() * 1000);
				//插入新元素表示已存在
				boost::thread(ProduceOfData, IpAndPort(strIp, strPort),
					"", listIP, tempTime, true);
			}
			else{
				//启动线程放入将recv放入
				int tempTime = (int)(gobalTime.elapsed() * 1000);
				
			/*	boost::thread(ProduceOfData, IpAndPort(strIp, strPort),
					receive_buffer, nullptr, tempTime, false);*/

				char *temp = new char[((int)receive_buffer[3]) * 50 + 30];
				memcpy(temp, receive_buffer, ((int)receive_buffer[3]) * 50 + 30);
				CliInfo.putCharData(IpAndPort(strIp, strPort), temp, tempTime);
				//iterMap->second->push(receive_buffer);
			}
			/*{
				boost::mutex::scoped_lock lock(::iomute);
				cout << strIp + " : " << strPort << "  " <<
					" recv:" << (int)const_buffer[3] << endl;
			}*/
			//memeset
			memset(const_buffer, '0', 2048);
		}
		catch (boost::system::system_error &e){
			{
				boost::mutex::scoped_lock lock(iomute);
				cout << "Process Fail: " << e.what() << endl;
			}
		}
	}
	delete[] listIP;
	delete[] receive_buffer;
	return 1;
}
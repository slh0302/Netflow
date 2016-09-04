

//boost  

#include "DataStruct.h"

#include "v9.h"
//thread


int main()
{
	queue<char*>* listIP;//ָ�����͵Ķ��з�����ʱnew

	//properties File Read
	properSet();
	//file read 2
	ReadConfig("./config/v9.properties", Search_Type);

	//Inital
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket udp_socket(io_service);
	boost::asio::ip::udp::endpoint local_add(boost::asio::ip::address::from_string("0.0.0.0"), net_bind_port);
	udp_socket.open(local_add.protocol());//Э��
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
			//���ڴ��ʼ��
			int length = (int)const_buffer[3];//��ȡ����
			int version_net = (int)const_buffer[1];//��ȡ�汾
			int useLen = 0;
			//uselen���ȱ仯
			useLen = GetUseLen(version_net, length);
			if (useLen == -1)
				continue;
			receive_buffer = new char[useLen];
			memcpy(receive_buffer, const_buffer, useLen);
			//receive_from ��Block������
			//��¼IP��PORT
			string strIp = send_point.address().to_string();
			unsigned short strPort = send_point.port();

			if (!CliInfo.haveIpAndPort(IpAndPort(strIp, strPort))){
				//not find : need a new queue
				cout << "δ�ҵ�����������Ϊ�¿ͻ���" << " ";
				//�����µĶ���
				listIP = new queue < char* > ;
				listIP->push(receive_buffer);//�����һ��Ԫ��
				//��������ʱ���õ�ʱ�� ms Ϊ��λ
				int tempTime = (int)(gobalTime.elapsed() * 1000);
				//������Ԫ�ر�ʾ�Ѵ���
				boost::thread(ProduceOfData, IpAndPort(strIp, strPort),
					"", listIP, tempTime, true);
			}
			else{
				//�����̷߳��뽫recv����
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
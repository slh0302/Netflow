#include"v9.h" 
#include "get_config.h"


queue<dataFlowSet_v9> dfs;
//查找type序号对应的内容
tfsmap tmap;
SchTyMap Search_Type;
/*****************************
未解决的问题：
map :主机地址+template——id
map :如果重复的话 覆盖问题
*****************************/

void v9::receive(char* const receive_buffer,string ip)
{
	memcpy(head, receive_buffer, sizeof(head_v9));
	convert_head();
	char* nowlocate;
	nowlocate = receive_buffer + sizeof(head_v9);
	v9ip = ip;
	for (int i = 0; i < head->count; i++)
	{
		//构建时放入IP
		handleFlowSet(nowlocate);
	}
}

void v9::convert_head()
{
	head->version = ntohs(head->version);
	head->count = ntohs(head->count);
	head->SysUptime = ntohl(head->SysUptime);
	head->unix_secs = ntohl(head->unix_secs);
	head->package_sequence = ntohl(head->package_sequence);
	head->source_id = ntohl(head->source_id);
}

void v9::convert_tfs()
{
	tfs->flowset_id = ntohs(tfs->flowset_id);
	tfs->length = ntohs(tfs->length);
}

void v9::convert_dfs(dataFlowSet_v9 & adfs)
{
	adfs.flowset_id = ntohs(adfs.flowset_id);
	adfs.length = ntohs(adfs.length);
}

void v9::handleFlowSet(char* nowlocate)
{
	short int Num = 0;//记录已经读取的字节数
	//judge this package with template or not
	short int temp;
	memcpy(&temp, nowlocate, 2);
	temp = ntohs(temp);
	if (temp < 256)                                                  //receive with template
	{
		memcpy(tfs, nowlocate, 4);
		convert_tfs();
		//left = tfs->length-4;
		nowlocate += 4;
		Num += 4;
		//templateFlowSet 下面可能有n条template record
		template_record atrd;
		for (int i = 0; Num < tfs->length; i++)
		{
			//nowlocate = nowlocate + tfs->length -left;
			memcpy(&atrd, nowlocate, 4);
			nowlocate += 4;
			Num += 4;
			//convert trd
			atrd.template_id = ntohs(atrd.template_id);
			atrd.field_count = ntohs(atrd.field_count);
			for (int k = 0; k < atrd.field_count; k++)
			{
				short int temp;
				memcpy(&temp, nowlocate, 2);
				nowlocate += 2;
				Num += 2;
				temp = ntohs(temp);
				atrd.field_type.push(temp);

				memcpy(&temp, nowlocate, 2);
				nowlocate += 2;
				Num += 2;
				temp = ntohs(temp);
				atrd.field_length.push(temp);
			}
			/*************************************
			put template_id and field_content to map
			要考虑到对应主机，
			故 template_id + head->source_id
			*************************************/
			char str[20];
			itoa(atrd.template_id, str, 10);
			string astr(str);
			/*

			加入IP区分

			*/
			//itoa(head->source_id, str, 10);
			astr += v9ip;
			tmap.insert(pair<string, template_record >(astr, atrd));
		}
	}
	else                                                                              //receive dataFlowSet
	{
		dataFlowSet_v9 adfs;
		memcpy(&adfs, nowlocate, 4);
		nowlocate += 4;
		convert_dfs(adfs);
		adfs.dataRecord = new char[adfs.length - 4];
		memcpy(adfs.dataRecord, nowlocate, adfs.length - 4);
		nowlocate += adfs.length - 4;
		dfs.push(adfs);
	}
}

void v9::print(ostream &  out)const
{
	char temp1[20];
	int dfsSize = dfs.size();
	//bool isfind=false;
	///
	///可能会很慢
	while(dfsSize > 0){
		//初始化
		dataFlowSet_v9 dataFlowSet_out = dfs.front();//弹出第一个值
		dfs.pop();
		dfsSize--;

		itoa(dataFlowSet_out.flowset_id, temp1, 10);
		string flowset_id_s(temp1);
		flowset_id_s += v9ip;
		tfsmap::iterator it;
		//test
		//out << "DataflowsetId: ";
		//out << dfsSize << endl;
		it = tmap.find(flowset_id_s);

		if (it != tmap.end()){
			out << "Record Template: " << endl;
			out << "Template ID: " << it->second.template_id << endl;
			out << "Template Field Count: " << it->second.field_count << endl;
			out << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			out << endl;
			short int print_data_begin = 0;//field_length
			for (int i = 0; i < it->second.field_count; i++)
				//for (int i = 0; print_data_begin != dataFlowSet_out.length; i++)
			{
				short int type;   //field_type;
				short int length;//field_length;
				short int type_length = 0;   //field_type;
				type = it->second.field_type.front();
				it->second.field_type.pop();
				length = it->second.field_length.front();
				it->second.field_length.pop();
				//it->second.field_count.pop();
				SchTyMap::iterator it_type;
				it_type = Search_Type.find(type);
				if (it != tmap.end()){
					/*****************************************************
						 输出Type对应Length的DataFlow的详细内容
					*****************************************************/
					//		out << type << "   " << length << endl;
					//****
					//实现方法1更简单
					//****
					out << "Type" << "		" << "Length" << "		" << "Velue" << "		" << endl;
					switch (length){
					case 1:               //Length (bytes)=1
					{
						char* Data_detail = new char;
						memcpy(Data_detail, dataFlowSet_out.dataRecord + print_data_begin, length);
						out << it_type->second << "		" << length << "		" << (int)*Data_detail << endl;
						break;
					}
					case 2:                  //Length (bytes)=2
					{
						short int* Data_detail = new short int;
						memcpy(Data_detail, dataFlowSet_out.dataRecord + print_data_begin, length);
						*Data_detail = ntohs(*Data_detail);
						out << it_type->second << "          " << length << "		" << *Data_detail << endl;
						break;
					}
					case 3:                  //Length (bytes)=3
					{
						//
						//Unhandle
						//
						short int Data_detail;
						memcpy(&Data_detail, dataFlowSet_out.dataRecord + print_data_begin, length);
						out << it_type->second << "          " << length << "		" << endl;
						break;
					}
					case 4:                  //Length (bytes)=4
					{
						struct in_addr Q;
						char ip[20];
						int* Data_detail = new int;
						memcpy(Data_detail, dataFlowSet_out.dataRecord + print_data_begin, length);
						if (isIPaddress(type)){
							Q.S_un.S_addr = *Data_detail;
							inet_ntop(AF_INET, (void *)&Q, ip, 16);
							out << it_type->second << "          " << length << "		" << ip << endl;
						}
						else
						{
							*Data_detail = ntohl(*Data_detail);
							out << it_type->second << "          " << length << "		" << *Data_detail << endl;
						}
						break;
					}
					case 6:                   //Length (bytes)=6
					{
						out << it_type->second << "          " << length << "		" << endl;
						break;
					}
					case 16:                  //Length (bytes)=16
					{
						out << it_type->second << "          " << length << "		" << endl;
						break;
					}
					default:				  //Length (bytes)=N
					{  //length = length * 8;
						out << it_type->second << "          " << length << "		" << endl;
						break;
					}
					}
					print_data_begin += length;
					//out << print_data_begin << ": (Bytes: " << length << ")： " << dataFlowSet_out.length << endl;
				}
				//out << "---------------------------------------------------------------------" << endl;
				else{
					out << "Error" << endl;
					break;
				}
				it->second.field_type.push(type);
				it->second.field_length.push(length);

			}
			out << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* " << endl;
		}
		else
			dfs.push(dataFlowSet_out);
	}
}

unsigned long v9::GetIP()
{
	return head->source_id;
}
ostream & operator<<(ostream & output, const v9 & v)
{
	v.print(output);
	return output;
}
bool isIPaddress(int type){
	if (type == 8 || type == 12 || type == 15 || type == 18)
	{
		return true;
	}
	else return false;
}
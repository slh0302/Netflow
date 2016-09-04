#include "get_config.h"

#include <fstream>
#include <iostream>
using namespace std;
int net_bind_port;
string net_bind_host;
bool flow_collector_V1_enabled;
bool flow_collector_V5_enabled;
bool flow_collector_V7_enabled;
bool flow_collector_V9_enabled;
int net_receive_buffer_size;
int flow_collector_max_queue_length;
int flow_collector_statistics_interval;

bool IsSpace(char c)
{
	if (' ' == c || '\t' == c)
		return true;
	return false;
}

bool IsCommentChar(char c)
{
	switch (c) {
	case COMMENT_CHAR:
		return true;
	default:
		return false;
	}
}

void Trim(string & str)
{
	if (str.empty()) {
		return;
	}
	int i, start_pos, end_pos;
	for (i = 0; i < str.size(); ++i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	if (i == str.size()) { // 全部是空白字符串
		str = "";
		return;
	}

	start_pos = i;

	for (i = str.size() - 1; i >= 0; --i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	end_pos = i;

	str = str.substr(start_pos, end_pos - start_pos + 1);
}

bool AnalyseLine(const string & line, string & key, string & value)
{
	if (line.empty())
		return false;
	int start_pos = 0, end_pos = line.size() - 1, pos;
	if ((pos = line.find(COMMENT_CHAR)) != -1) {
		if (0 == pos) {  // 行的第一个字符就是注释字符
			return false;
		}
		end_pos = pos - 1;
	}
	string new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分

	if ((pos = new_line.find('=')) == -1)
		return false;  // 没有=号

	key = new_line.substr(0, pos);
	value = new_line.substr(pos + 1, end_pos + 1 - (pos + 1));

	Trim(key);
	if (key.empty()) {
		return false;
	}
	Trim(value);
	return true;
}

bool ReadConfig(const string & filename, map<string, string> & m)
{
	m.clear();
	ifstream infile(filename.c_str());
	if (!infile) {
		cout << "file open error" << endl;
		return false;
	}
	string line, key, value;
	while (getline(infile, line)) {
		if (AnalyseLine(line, key, value)) {
			m[key] = value;
		}
	}

	infile.close();
	return true;
}

//rewrite
bool ReadConfig(const string & filename, map<int, string> & m)
{
	m.clear();
	ifstream infile(filename.c_str());
	if (!infile) {
		cout << "file open error" << endl;
		return false;
	}
	string line, key, value;
	int key_int;
	while (getline(infile, line)) {
		if (AnalyseLine(line, key, value)) {
			key_int = atoi(value.c_str());
			m[key_int] = key;
		}
	}

	infile.close();
	return true;
}



#ifndef _GET_CONFIG_H_

#define _GET_CONFIG_H_



#include <string>

#include <map>

using namespace std;

extern int net_bind_port;
extern int net_receive_buffer_size;
extern string net_bind_host;
extern bool flow_collector_V1_enabled;
extern bool flow_collector_V5_enabled;
extern bool flow_collector_V7_enabled;
extern bool flow_collector_V9_enabled;
extern int flow_collector_max_queue_length;
extern int flow_collector_statistics_interval;

#define COMMENT_CHAR '#'



bool ReadConfig(const string & filename, map<string, string> & m);
//void PrintConfig(const map<string, string> & m);
//жиди
//void PrintConfig(const map<string, int> & m);
bool ReadConfig(const string & filename, map<int, string> & m);
#endif


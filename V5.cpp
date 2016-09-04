#include"V5.h"


V5::V5(char * receive_buffer)
{
	rec_head = new head;
	memcpy(rec_head,receive_buffer,sizeof(head));			
	convert_head();
	for(int i=0; i<rec_head->count; i++)
	{
		rec_flow[i] = new flow; 
		memcpy(rec_flow[i], receive_buffer + sizeof(head) + sizeof(flow)*(i), sizeof(flow));
		convert_flow(rec_flow[i]);
	}
}

void V5::convert_head()
{
	rec_head->version = ntohs(rec_head->version);
	rec_head->count = ntohs(rec_head->count);
	rec_head->SysUptime = ntohl(rec_head->SysUptime);
	rec_head->unix_secs = ntohl(rec_head->unix_secs);
	rec_head->unix_nsecs = ntohl(rec_head->unix_nsecs);
	rec_head->flow_sequence = ntohl(rec_head->flow_sequence);
	rec_head->sampling_interval = ntohs(rec_head->sampling_interval);
	//rec_head->
}

void V5::convert_flow(flow *a)
{
	a->input = ntohs(a->input);
	a->output = ntohs(a->output);
	a->dPkts = ntohl(a->dPkts);
	a->dOctets = ntohl(a->dOctets);
	a->First = ntohl(a->First);
	a->Last = ntohl(a->Last);
	a->srcport = ntohs(a->srcport);
	a->dstport = ntohs(a->dstport);
	a->src_as = ntohs(a->src_as);
	a->dst_as = ntohs(a->dst_as);
	a->pad2 = ntohs(a->pad2);
}

void V5::print(ostream &output) const
{
	output<<"head:"<<endl;
	output<<"	version:"<<rec_head->version<<endl;
	output<<"	count:"<<rec_head->count<<endl;
	output << "	SysUptime:" << rec_head->SysUptime << endl;
	output << "	nix_secs:" << rec_head->unix_secs << endl;
	output << "	unix_nsecs:" << rec_head->unix_nsecs << endl;
	output << "	engine_type:" << (int)rec_head->engine_type << endl;
	output << "	engine_id:" <<(int) rec_head->engine_id << endl;
	output<<endl;
	for(int i=0; i<rec_head->count; i++)
	{
		output<<"flow"<<i+1<<":"<<endl;
		//对源ip进行转换
		struct in_addr Q;
		char ip[20];
		Q.S_un.S_addr = rec_flow[i]->srcaddr;
		inet_ntop(AF_INET, (void *)&Q, ip , 16);		
		output<<"	srcaddr:"<<ip<<"  "<<"	srcport:"<<rec_flow[i]->srcport<<endl;
		
		//对目的ip进行转换
		Q.S_un.S_addr = rec_flow[i]->dstaddr;
		inet_ntop(AF_INET, (void *)&Q, ip , 16);	
		output<<"	dstaddr:"<<ip<<"  "<<"	dstport:"<<rec_flow[i]->dstport<<endl;		
		
		//对下一条ip进行转换
		Q.S_un.S_addr = rec_flow[i]->nexthop;
		inet_ntop(AF_INET, (void *)&Q, ip , 16);
		output<<"	nexthop:"<<ip<<endl;
		
		output << "	input:" << rec_flow[i]->input << endl;
		output << "	output:" << rec_flow[i]->output<< endl;
		output << "	dPkts:" << rec_flow[i]->dPkts<< endl;
		output << "	dOctets:" << rec_flow[i]->dOctets << endl;
		output << "	First:" << rec_flow[i]->First<< endl;
		output << "	Last:" << rec_flow[i]->Last << endl;
		output << "	srcport:" << rec_flow[i]->srcport << endl;
		output << "	dstport:" << rec_flow[i]->dstport << endl;
		output << "	pad1:" << (int)rec_flow[i]->pad1 << endl;
		output << "	tcp_flags:" << (int)rec_flow[i]->tcp_flags << endl;
		//output << "	prot:" << (int)rec_flow[i]->prot << endl;
		output << "	tos:" << (int)rec_flow[i]->tos << endl;
		output << "	src_as:" << rec_flow[i]->src_as << endl;
	}
	output<<endl;
	output<<"-------------------------------------------------------------"<<endl;

}

//<<输出运算符重载
ostream & operator<<(ostream & output, const V5 & v)
{
	v.print(output);
	return output;
}

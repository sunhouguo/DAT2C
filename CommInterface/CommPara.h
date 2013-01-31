#pragma once
#ifndef CommPara_H
#define CommPara_H

#include <vector>

namespace FileSystem
{
	class CMarkup;
}

namespace CommInterface {

class CCommPara
{
public:
	CCommPara(void);
	CCommPara(const CCommPara & rhs);
	CCommPara & operator = (const CCommPara & rhs);
	~CCommPara(void);

	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);

	//file name api
	std::string getProtocolCfgFileName();
	bool getEnableSpecialProtocolCfg();
	std::string getCommChannelStatusLogFileName();
	bool getEnableChannelStatusLog();
	std::string getCommChannelFrameLogFileName();
	bool getEnableChannelFrameLog();
	std::string getCommChannelStatusLogType();
	std::string getCommChannelFrameLogType();
	std::string getCommChannelStatusLogLimit();
	std::string getCommChannelFrameLogLimit();

	bool getPrintFrameTerm();
	bool getPrintStatusTerm();

	std::string getLocalIP();
	std::string getLocalPort();
	std::string getBroadcastIP();
	std::string getBroadcastPort();
	std::string getMulticastIP();
	std::string getMulticastPort();
	std::string getRemoteID();
	std::string getMatchType();
	std::string getCommChannelType();
	std::string getProtocolType();

	int setRemoteID(std::string val);

	int getNextRemotePara(std::string & ip,std::string port);
	bool MatchRemoteIP(std::string val);
	int getRemoteIPSum();
	int getRemotePortSum();
	std::string getRemoteIP(int index);
	std::string getRemotePort(int index);
	bool getbDisableCommpointByCloseConnect();

	//para format api
	static bool AssertIPFormat(std::string val);
	static bool AssertNetPortFormat(std::string val);
	static bool AssertSerialPortFormat(std::string val);

private:
	int setProtocolCfgFileName(std::string val);
	int setCommChannelStatusLogFileName(std::string val);
	int setCommChannelFrameLogFileName(std::string val);
	int setCommChannelStatusLogType(std::string val);
	int setCommChannelFrameLogType(std::string val);
	int setCommChannelStatusLogLimit(std::string val);
	int setCommChannelFrameLogLimit(std::string val);

	int setPrintFrameTerm(bool val);
	int setPrintStatusTerm(bool val);

	int setLocalIP(std::string val);
	int setLocalPort(std::string val);
	int setBroadcastIP(std::string val);
	int setBoardcastPort(std::string val);
	int setMulticastIP(std::string val);
	int setMulticastPort(std::string val);
	int setCommChannelType(std::string val);
	int setProtocolType(std::string val);
	int setMatchType(std::string val);

	int AddRemoteIP(std::string val);
	int AddRemotePort(std::string val);
	void ClearRemoteIP();
	void ClearRemotePort();

	int setbDisableCommpointByCloseConnect(bool val);

private:
	std::string localIP_;                      //本地IP
	std::string localPort_;                    //本地端口
	std::string multicastIP_;                  //多播地址
	std::string multicastPort_;                //多播端口号
	std::string broadcastIP_;                  //广播地址
	std::string broadcastPort_;                //广播端口
	std::vector<std::string> remoteIPs_;       //对端IP集合
	std::vector<std::string> remotePorts_;     //对端端口集合
	int remoteParaIndex_;

	std::string remoteID_;                     //对端ID标识

	std::string commChannelType_;              //通讯通道类型
	std::string commServerMatchType_;          //服务器通道的匹配方式
	bool bPrintFrame_;                         //在显示终端打印报文
	bool bPrintStatus_;                        //在显示终端打印状态

	std::string commChannelStatusLog_;          //激活通道状态日志
	std::string commChannelFrameLog_;           //激活通道报文日志
	std::string commChannelStatusLogType_;
	std::string commChannelFrameLogType_;
	std::string commChannelStatusLogLimit_;
	std::string commChannelFrameLogLimit_;

	std::string protocolType_;                 //规约类型
	std::string protocolCfgFile_;              //规约配置文件名

	bool DisableCommpointByCloseConnect_;      //决定关闭通道的时候是否立刻更改通讯点的通讯状态，设为false的话也就意味着通道的状态不会立刻影响到该通道下通讯点的状态，给了通讯点恢复通讯而不必向上报告的缓冲时间。目前只有TcpClient使用了这一属性。
};

};//namespace CommInterface 

#endif //#ifndef CommPara_H

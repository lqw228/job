#pragma once

#include <string>
using namespace std;

class GobalInfo
{
public:
	GobalInfo();
	~GobalInfo();

	//读取配置文件
	bool ReadIni(const char* pPath);
	string& getBrokerURI();
	const char* getbrokeruri();
	const char* GetIp();
	int GetPort();
	const char* GetName();
	const char* GetPwd();
	const char* GetTrade2Jna();
	const char* GetJna2Trade();
	const char* getcer();
	const char* getappid();
	const char* getdelete2jna();
	const char* getjna2delete();
//private:
public:
	string _ip;
	int _port;
	int _acountComId;   //券商ID
	string _mqName;
	string _mqPwd;      //mq密码
	string _ver;        //版本号
	string _department; //营业部
	string _account;    //账号
	string _tradePass;   //交易密码
	string _commPass;    //通讯密码

	string _appid;
	string _cer;
	string _trade2jna;
	string _jna2trade;
	string _name;
	string _pwd;
	string _delete2Jna;
	string _Jna2delete;
	string _strBrokerURI;

	string _title; //窗体标题
};

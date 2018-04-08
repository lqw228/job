#pragma once

#include <string>
using namespace std;

class GobalInfo
{
public:
	GobalInfo();
	~GobalInfo();

	//��ȡ�����ļ�
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
	int _acountComId;   //ȯ��ID
	string _mqName;
	string _mqPwd;      //mq����
	string _ver;        //�汾��
	string _department; //Ӫҵ��
	string _account;    //�˺�
	string _tradePass;   //��������
	string _commPass;    //ͨѶ����

	string _appid;
	string _cer;
	string _trade2jna;
	string _jna2trade;
	string _name;
	string _pwd;
	string _delete2Jna;
	string _Jna2delete;
	string _strBrokerURI;

	string _title; //�������
};

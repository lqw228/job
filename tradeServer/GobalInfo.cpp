#include "StdAfx.h"
#include "GobalInfo.h"
#include <iostream>
#include<afxwin.h>

using namespace std;


GobalInfo::GobalInfo()
{
}


GobalInfo::~GobalInfo()
{
}

string& GobalInfo::getBrokerURI()
{
	return _strBrokerURI;
}
const char* GobalInfo::getbrokeruri()
{
	return _strBrokerURI.c_str();
}

const char* GobalInfo::GetIp()
{
	return _ip.c_str();
}

int  GobalInfo::GetPort()
{
	return _port;
}
const char* GobalInfo::GetName()
{
	return _name.c_str();
}
const char* GobalInfo::GetPwd()
{
	return _pwd.c_str();
}

const char* GobalInfo::GetTrade2Jna()
{
	return _trade2jna.c_str();
}
const char* GobalInfo::GetJna2Trade()
{
	return _jna2trade.c_str();
}

const char* GobalInfo::getcer()
{
	return _cer.c_str();
}
const char* GobalInfo::getappid()
{
	return _appid.c_str();
}

const char* GobalInfo::getdelete2jna()
{
	return _delete2Jna.c_str();
}
const char* GobalInfo::getjna2delete()
{
	return _Jna2delete.c_str();
}


bool GobalInfo::ReadIni(const char* pPath)
{
	char szApp[128] = { 0 };
	char szTemp[1024] = { 0 };

	memset(szTemp, 0, sizeof(szTemp));
	strcpy(szApp, "service");
	::GetPrivateProfileString(szApp, "ip", "127.0.0.1", szTemp, sizeof(szTemp) - 1, pPath);//59.40.79.53
	cout << "ip: " << szTemp << endl;
	_ip = szTemp;

	int port = ::GetPrivateProfileIntA(szApp, "port", 7708, pPath);
	_port = port;
	cout << "port: " << port << endl;

	_acountComId  = ::GetPrivateProfileIntA(szApp, "traderid", 0, pPath);
	cout << "traderid: " << _acountComId <<endl;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA(szApp, "version", "6.00", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "version: " << szTemp << endl;
	_ver = szTemp;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA(szApp, "department", "0", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "department: " << szTemp << endl;
	_department = szTemp;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA(szApp, "account", "0", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "account: " << szTemp << endl;
	_account = szTemp;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA(szApp, "tradePass", "0", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "tradePass: " << szTemp << endl;
	_tradePass = szTemp;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA(szApp, "commPass", "", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "commPass: " << szTemp << endl;
	_commPass = szTemp;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA("activemq", "addr", NULL, szTemp, sizeof(szTemp) - 1, pPath);
	cout << "activemq, addr: " << szTemp << endl;

	char szBrokerURI[128] = { 0 };
	sprintf(szBrokerURI, "failover:(tcp://%s)", szTemp);
	std::string brokerURI = szBrokerURI;
	cout << "brokerURI: " << brokerURI.c_str() << endl;
	_strBrokerURI = brokerURI;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA("activemq", "name", "", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "name: " << szTemp << endl;
	_mqName = szTemp;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA("activemq", "pwd", "", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "pwd: " << szTemp << endl;
	_mqPwd = szTemp;

	memset(szTemp, 0, sizeof(szTemp));
	::GetPrivateProfileStringA("windows", "title", "TradeService", szTemp, sizeof(szTemp) - 1, pPath);
	cout << "title: " << szTemp << endl;
	_title = szTemp;

	return true;

}

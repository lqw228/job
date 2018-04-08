// tradeServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <afx.h>
#include "TdxRowset.h"
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/System.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <json/json.h>
#include <algorithm>
#include <sstream>
#include <map>
#include "MqConsumer.h"
#include "MqProducer.h"
#include "gobal.h"
#include "GobalInfo.h"
#include "ILogEvent.h"
#include "Dir.h"
#include "IniFile.h"
#include "System.h"
#include "Func.h"

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

MqProducer tradeProcduct;        //����������
MqProducer deleteProcduct;       //����������
MqProducer qryMoneyProcduct;     //��ѯ�ʽ��˺�������
MqProducer qrySuccessProcduct;   //��ѯ�ɽ���¼������
MqProducer qryHoldProcduct;      //��ѯ�ֲ�
MqProducer qryHoldTopicProcduct; //��ѯ�ֲַ��͹㲥
MqProducer qryMoneyTopicProcduct; //��ѯ�ʽ����ݷ��͹㲥


GobalInfo* pInfo = NULL;

CRITICAL_SECTION cs;           //�����ٽ���ȫ�ֱ���
map<string, int> check;        //�ɽ�����
map<string, int> checkEntrust; //ί������

ILogEvent* pLog = NULL;
CIniFile* iniFile = NULL;
TradeSystem* sys = NULL;//����ϵͳ

//����������
void DealTotalReq(string& text, int funid)
{
	sys->tradeParse(text, funid);
}

//�̺߳�������ѯ�ɽ���ί�� 
DWORD WINAPI ThreadFunc(LPVOID lpParam)  
{  
    int n = (int)lpParam;
	int nCount = 0;

	while (1)
	{
		Sleep(5000);
		//�����ٽ���  
		EnterCriticalSection(&cs);
		sys->checkDay(); //������ռ�¼����
		sys->QryDeal();
		sys->QryEntrust();
		sys->QryHold(false);
		sys->QryMoney(false);
		//�뿪�ٽ���  
		LeaveCriticalSection(&cs); 
		
	}
	printf("Thread #%d returned successfully\n", n);  
	return 0;  
}

int _tmain(int argc, _TCHAR* argv[])
{
	activemq::library::ActiveMQCPP::initializeLibrary();	 
	InitializeCriticalSection(&cs);//��ʼ���ٽ��� 
	sys = new TradeSystem();
	if (!sys->init())
	{
		printf("system init failed\n");
		return -1;
	}

	if (-1 == sys->start())
	{
		printf("system start failed\n");
		return -2;
	}

	std::string brokerURI = pInfo->_strBrokerURI;
	cout << "------1-------"<<brokerURI << endl;
	//-----------------------����mq���׷���----------------
	std::string strDest3 = "query-order-success";
	qrySuccessProcduct.CreateProduct(brokerURI, strDest3);
	std::string strTradeToOrderDest = "query-entrust-cno";
	tradeProcduct.CreateProduct(brokerURI, strTradeToOrderDest);
	strTradeToOrderDest = "query-order-cancel";
	deleteProcduct.CreateProduct(brokerURI, strTradeToOrderDest);
	strTradeToOrderDest = "query-amount";
	qryMoneyProcduct.CreateProduct(brokerURI, strTradeToOrderDest);
	strTradeToOrderDest = "query-hold";
	qryHoldProcduct.CreateProduct(brokerURI, strTradeToOrderDest);
	std::stringstream ss;  
	ss << "query-hold-"<< pInfo->_acountComId << "-" <<pInfo->_account;  
	strTradeToOrderDest = ss.str();
	qryHoldTopicProcduct.CreateProduct(brokerURI, strTradeToOrderDest, true);
	std::stringstream ss1; 
	ss1 << "query-amount-"<< pInfo->_acountComId << "-" <<pInfo->_account;  
	strTradeToOrderDest = ss1.str();
	qryMoneyTopicProcduct.CreateProduct(brokerURI, strTradeToOrderDest, true);
	cout << "------2-------" << brokerURI << endl;

	//--------------����mq����-----------
	char szBuf[512]={0};
	sprintf(szBuf, "cmd-%d-%s", pInfo->_acountComId, pInfo->_account.c_str()); 
	printf("receive cmd: %s\n", szBuf);
	std::string strDest = szBuf;	
	MqConsumer testConsumer(brokerURI, strDest);
	Thread consumerThread1(&testConsumer);
	consumerThread1.start();

	DWORD ThreadID =0;  
	HANDLE hThread = NULL;  
	//�����̣߳���ѯ�ɽ� 
	hThread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)0, 0, &ThreadID);
	printf("Thread #%d has been created successfully.\n", ThreadID);

	getchar();
	getchar();
	getchar();
	sys->stop();
	sys->Uninit();
	pLog->PrintLog("program over\n");
	activemq::library::ActiveMQCPP::shutdownLibrary();
	
	return 0;
}

//std::string writeJson() 
//{
//	using namespace std;
//
//	Json::Value root;
//	Json::Value arrayObj;
//	Json::Value item;
//	Json::Value iNum;
//	
//
//	item["string"]    = "this is a string";
//	item["number"]    = 999;
//	item["aaaaaa"]    = "bbbbbb";
//	arrayObj.append(item);
//
//	//ֱ�Ӷ�jsoncpp����������������Ϊ�±���и�ֵ�����Զ���Ϊ����
//	iNum[1] = 1;
//	iNum[2] = 2;
//	iNum[3] = 3;
//	iNum[4] = 4;
//	iNum[5] = 5;
//	iNum[6] = 6;
//
//	//���Ӷ�������
//	root["array"]    = arrayObj;
//	//�����ַ���
//	root["name"]    = "json";
//	//��������
//	root["number"]    = 666;
//	//���Ӳ�������
//	root["value"]    = true;
//	//������������
//	root["arrnum"]    = iNum;
//
//	root.toStyledString();
//	string out = root.toStyledString();
//
//	return out;
//}

//cout << writeJson() <<endl;
//Json::Value root;
//Json::FastWriter writer;
//Json::Value person;

//person["name"] = "hello world1";
//person["age"] = 100;
//root.append(person);
//person["name"] = "hello world2";
//person["age"] = 200;
//root.append(person);

//string data= writer.write(root);
//cout<<data<<endl;   
//cout<<root.toStyledString()<<endl;
//getchar();

//FTdxInit TdxInit=NULL; //����ָ��
//FTdxExit TdxExit=NULL;
//FTdxLogin TdxLogin=NULL;
//FTdxLogout   TdxLogout=NULL;
//FTdxQueryData TdxQueryData=NULL;
//FTdxSendOrder TdxSendOrder=NULL;
//FTdxCancelOrder	TdxCancelOrder=NULL;
//FTdxGetQuote TdxGetQuote=NULL;
//FTdxQueryHistory TdxQueryHistory=NULL;
//FTdxSendOrderNew TdxSendOrderNew=NULL;
//FTdxCancelOrderNew	TdxCancelOrderNew=NULL;
//FGetSubmitUName GetSubmitUName=NULL;
//FGetSubmitUID GetSubmitUID=NULL;
//FGetDestName GetDestName=NULL;
//FGetBranchName GetBranchName=NULL;
//FGetBranchID GetBranchID=NULL;
//FGetClientConfig GetClientConfig=NULL;
//FGetOemQstag GetOemQstag=NULL;
//FGetServDesc GetServDesc=NULL;

//HINSTANCE hDll = NULL; //trade.dll
//int ClientID = -1;     //����ID��-1Ϊδ����
//char ErrInfo[256] = {0};

//char Account[100] = "3000087958";

//CString MAccount ;//��ǰ��¼���˺�
//CString FSubmitUID;
//
//CString GAccount ;//��Ʊ�˺�
//CString GSZAID ;//��A 0
//CString GSHAID ;//��A 1
//CString GSZBID ;//  B 2
//CString GSHBID ;//  B 3
//CString FGZID ;//  B 12
//
//CString RAccount ;//�����˺�
//CString RSZAID ;//��A 0 �����˺�
//CString RSHAID ;//��A 1
//CString RSZBID ;//  B 2
//CString RSHBID ;//  B 3

//void SendData(string& text)
//{
//	tradeProcduct.SendData(text);
//}
//
//
//
//void InsertOrder(Json::Value& root)
//{
//	////��ȡ�µ�ָ�����
//	std::string orderNo = root["orderNumber"].asString();
//	int num = root["volume"].asInt();
//	double price = root["price"].asDouble();
//	int TradeType = root["behavior"].asInt(); //0:����1:����
//	int TradeMode = 0;//root["orderMode"].asInt();
//	std::string strCode = root["stockcode"].asString();
//	int rt = -1;
//	char *Result = new char[1024 * 1024];
//	std::string out;
//
//	Json::Value new_item;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//	new_item["orderNumber"] = orderNo;
//
//	//����
//	char code[20] = {0};   //"002087";
//	strcpy(code, strCode.c_str());
//	if (strlen(code) != 6)
//		return ;
//
//	CString gdcode=GSZAID;
//	if (code[0]=='6')
//		gdcode=GSHAID;
//	pLog->PrintLog("InsertOrder, TradeType=%d, gdcode=%s, code=%s, price=%f, num=%d\n", TradeType, (LPSTR)(LPCTSTR)gdcode, code, price, num);
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) == 0)
//	{		
//		rt = TdxSendOrder(ClientID, TradeType, TradeMode, (LPSTR)(LPCTSTR)gdcode, code, price, num, Result, ErrInfo);
//	}else
//	{
//		rt = TdxSendOrderNew(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, TradeType, TradeMode, (LPSTR)(LPCTSTR)gdcode, code, price, num, Result, ErrInfo);
//	}
//
//	new_item["ret"] = rt;
//	if (rt == 1)
//	{
//		printf("trade ok, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//		CString s = tr.GetID(0, 146);
//		printf("s = %s\n", (LPSTR)(LPCTSTR)s);
//		new_item["cno"] = (LPSTR)(LPCTSTR)s;
//	}
//	else{
//		printf("trade failed, ErrInfo=%s\n", ErrInfo);
//		pLog->PrintLog("trade failed, ErrInfo=%s\n", ErrInfo);
//		new_item["errInfo"] = G2U(ErrInfo);
//	}
//
//	// ����޸�ʽjson�ַ���   
//	Json::FastWriter writer;
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("InsertOrder callback, out=%s\n", out.c_str());
//	SendData(out);
//
//	delete [] Result;
//}
//
//int OrderDelete(Json::Value& root)
//{
//	Json::Value new_item;
//	std::string out;
//	Json::FastWriter writer;
//	char *Result = new char[1024 * 1024];
//	std::string stockcode = root["stockcode"].asString();
//	std::string orderid = root["cno"].asString();
//	std::string mks = "";
//	int rt = 0;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//	new_item["cno"] = orderid;
//	new_item["rsp"] = 0;
//
//	pLog->PrintLog("OrderDelete, orderid=%s\n", orderid.c_str());
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) == 0)
//	{
//		rt = TdxCancelOrder(ClientID, (char*)stockcode.c_str(), (char*)orderid.c_str(), (char*)mks.c_str(), Result, ErrInfo);
//	}else
//	{
//		rt = TdxCancelOrderNew(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, (char*)stockcode.c_str(), (char*)orderid.c_str(), (char*)mks.c_str(), Result, ErrInfo);
//	}
//
//	new_item["ret"] = rt;
//	if (rt == 1)
//	{
//		printf("����, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//	}
//	else
//	{
//		printf("����,ErrInfo = %s\n", ErrInfo);
//		pLog->PrintLog("����,ErrInfo = %s\n", ErrInfo);
//		new_item["errInfo"] = G2U(ErrInfo);
//	}
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("OrderDelete callback, out=%s\n", out.c_str());
//	deleteProcduct.SendData(out);
//
//	delete [] Result;
//	return rt;
//}
//
//void QryMoney(Json::Value& root)
//{
//	Json::Value new_item;
//	std::string out;
//	Json::FastWriter writer;
//	char *Result = new char[1024 * 1024];
//	int rt = 0;
//
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) != 0)
//	{		
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 0, Result, ErrInfo);    //0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 0, Result, ErrInfo);  //0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}	
//
//	new_item["ret"] = rt;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//	if (rt == 1)
//	{
//		printf("�ʽ��б�, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//		CString assets = tr.GetID(0, 310);      //���ʲ�
//		CString marketValue = tr.GetID(0, 205); //������ֵ
//		CString balance = tr.GetID(0, 301);     //�����ʽ�
//		printf("%s, %s, %s\n", assets, marketValue, balance);
//		new_item["assets"] = (LPSTR)(LPCTSTR)assets;
//		new_item["marketValue"] = (LPSTR)(LPCTSTR)marketValue;
//		new_item["balance"] = (LPSTR)(LPCTSTR)balance;
//	}
//	else{
//		printf("�ʽ�, ErrInfo = %s\n", ErrInfo);
//		pLog->PrintLog("�ʽ�, ErrInfo = %s\n", ErrInfo);
//		new_item["errInfo"] = G2U(ErrInfo);
//	}
//
//	//���ͻر�����
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("QryMoney callback, out=%s\n", out.c_str());
//	qryMoneyProcduct.SendData(out);
//	delete [] Result;
//}
//
////��ѯ�ֲ��б�
//int QryHold(){
//	Json::Value new_item;
//	std::string out;
//	Json::FastWriter writer;
//	int rt = 0;	
//	char *Result = new char[1024 * 1024];
//
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) != 0)
//	{		
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 1, Result, ErrInfo);     //0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 1, Result, ErrInfo); //0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}
//
//	new_item["ret"] = rt;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//
//	if (rt == 1)
//	{
//		printf("�ֲֹɷ��б�, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//		for (int i = 0; i < tr.FRowCount; i++)
//		{
//			if (eTradeID_WUKUANG == pInfo->_acountComId || 
//				eTradeID_GUANGFA == pInfo->_acountComId || 
//				eTradeID_CAITONG == pInfo->_acountComId)
//			{
//				//�ֲֽ���
//				CString code = tr.GetID(i, 140);         //֤ȯ����
//				CString name = tr.GetID(i, 141);         //֤ȯ����
//				CString num = tr.GetID(i, 200);          //֤ȯ����
//				CString costprice = tr.GetID(i, 202);    //�ɱ���
//				CString currentprice = tr.GetID(i, 949); //��ǰ��
//				CString balance = tr.GetID(i, 204);      //����ӯ��
//
//				printf("%s, %s, %s, %s, %s, %s\n", code, name, num, costprice, currentprice, balance);
//				//���ض�Ӧͨ��
//
//			}
//
//		}
//	}
//	else{
//		printf("�ֲ�, ErrInfo = %s\n", ErrInfo);
//		pLog->PrintLog("�ֲ�, ErrInfo = %s\n", ErrInfo);
//		new_item["errInfo"] = G2U(ErrInfo);
//	}
//
//	//���ͻر�����
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("QryHold callback, out=%s\n", out.c_str());
//
//	delete [] Result;
//	return rt;
//}
//
////���ͳɽ����ƻر�
//void SendDeal(CString con,CString assets, CString marketValue,CString balance)
//{
//	Json::Value new_item;
//	std::string out;
//	Json::FastWriter writer;
//
//	int rt = 0;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//	new_item["cno"] = (LPSTR)(LPCTSTR)con;
//	new_item["successOrderNumber"] = (LPSTR)(LPCTSTR)assets;
//	new_item["volume"] = (LPSTR)(LPCTSTR)marketValue;
//	new_item["price"] = (LPSTR)(LPCTSTR)balance;
//
//	//���ͻر�����
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
//	qrySuccessProcduct.SendData(out);	
//
//}
//
////���ͳ������ƻر�
//void SendCancel(CString con,CString assets, CString marketValue,CString balance)
//{
//	Json::Value new_item;
//	std::string out;
//	Json::FastWriter writer;
//
//	int rt = 0;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//	new_item["cno"] = (LPSTR)(LPCTSTR)con;
//	new_item["volume"] = (LPSTR)(LPCTSTR)marketValue;
//	new_item["rsp"]=1;
//	new_item["ret"]=1;
//
//	//���ͻر�����
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
//	deleteProcduct.SendData(out);
//}
//
////��ѯ���ճɽ�
//int QryDeal()
//{
//	static int myCount = 0;
//	static int errorNum = 0; 
//	int rt = 0;
//	char *Result = new char[1024 * 1024];
//
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) != 0)
//	{		
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 3, Result, ErrInfo); //0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 3, Result, ErrInfo);//0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}
//	if (rt == 1)
//	{
//		errorNum = 0;
//		if (myCount > 100000)
//		{
//			myCount=0;
//		}
//		iniFile->WriteIniInt_In("trade", "count", myCount++);
//		printf("���ճɽ��б�, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//		for (int i = 0; i < tr.FRowCount; i++)
//		{
//			if (eTradeID_WUKUANG == pInfo->_acountComId)
//			{//���
//				CString con = tr.GetID(i, 146);         //��ͬ���
//				CString assets = tr.GetID(i, 155);      //�ɽ����
//				CString marketValue = tr.GetID(i, 152); //�ɽ�����
//				CString balance = tr.GetID(i, 153);     //�ɽ��۸�
//				CString status = tr.GetID(i, 147);      //״̬
//				printf("%s, %s, %s, %s, %s\n", con, assets, marketValue, balance, status);
//
//				if (0 == check[(LPSTR)(LPCTSTR)assets])
//				{
//					if (status.Compare("��ͨ�ɽ�") == 0)
//					{
//						SendDeal(con, assets,  marketValue, balance);
//					}else if (status.Compare("�ڲ�����") == 0)
//					{
//						SendCancel(con, assets,  marketValue, balance);
//					}else if (status.Compare("�����ɽ�") == 0)
//					{
//						SendCancel(con, assets,  marketValue, balance);
//					}			
//
//					check[(LPSTR)(LPCTSTR)assets] = 1;
//				}
//			}else if (eTradeID_GUANGFA == pInfo->_acountComId || eTradeID_CAITONG == pInfo->_acountComId || 
//				eTradeID_PINGAN == pInfo->_acountComId || eTradeID_DONGBEI == pInfo->_acountComId || 
//				eTradeID_GUOYUAN == pInfo->_acountComId || eTradeID_HENGTAI == pInfo->_acountComId || 
//				eTradeID_CUOTAIJUNAN == pInfo->_acountComId)
//			{//�㷢֤ȯ,��֤ͨȯ
//				CString con = tr.GetID(i, 146);         //��ͬ���
//				CString assets = tr.GetID(i, 155);      //�ɽ����
//				CString marketValue = tr.GetID(i, 152); //�ɽ�����
//				CString balance = tr.GetID(i, 153);     //�ɽ��۸�
//				//CString status = tr.GetID(i, 147);      //״̬
//				printf("deal, con=%s, assets=%s, marketValue=%s, balance=%s\n", con, assets, marketValue, balance);
//
//				if (0 == check[(LPSTR)(LPCTSTR)assets])
//				{
//					SendDeal(con, assets,  marketValue, balance);
//					check[(LPSTR)(LPCTSTR)assets] = 1;
//				}
//			}			
//		}
//	}
//	else{		
//		printf("ErrInfo = %s, errorNum=%d\n", ErrInfo, errorNum++);
//		pLog->PrintLog("ErrInfo = %s, errorNum=%d\n", ErrInfo, errorNum);
//		if (errorNum > 20)
//		{
//			pLog->PrintLog("--------stop app, num=%d, error=%s--------\n", errorNum, ErrInfo);
//			exit(-2);
//		}
//	}	
//
//	delete [] Result;
//	return rt;
//}
//
////��ѯ����ί���б�
//int QryEntrust(){
//	int rt = 0;
//	TdxRowset tr;
//	char *Result = new char[1024 * 1024];
//
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) != 0)
//	{		
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 2, Result, ErrInfo); //0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 2, Result, ErrInfo); //0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	}
//
//	if (rt == 1)
//	{
//		printf("����ί���б�, Result=%s\n", Result);		
//		tr.Update(Result);
//		for (int i = 0; i < tr.FRowCount; i++)
//		{
//			if (eTradeID_GUANGFA == pInfo->_acountComId || eTradeID_CAITONG == pInfo->_acountComId 
//				|| eTradeID_DONGBEI == pInfo->_acountComId || eTradeID_HENGTAI == pInfo->_acountComId || 
//				eTradeID_CUOTAIJUNAN == pInfo->_acountComId)
//			{
//				CString con = tr.GetID(i, 146);         //��ͬ���				
//				CString num = tr.GetID(i, 144);         //ί������
//				CString price = tr.GetID(i, 145);       //�۸�
//				CString status = tr.GetID(i, 147);      //״̬
//				printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
//				if (status.Compare("�ϵ�") == 0 || status.Compare("�ѳ�") == 0)
//				{
//					if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
//					{
//						if (status.Compare("�ϵ�") == 0)
//						{
//							SendCancel(con, "",  num, price);
//						}else if (status.Compare("�ѳ�") == 0)
//						{
//							SendCancel(con, "",  num, price);
//						}
//						checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
//					}
//				}
//			}else if (eTradeID_GUOYUAN == pInfo->_acountComId)
//			{
//				CString con = tr.GetID(i, 146);         //��ͬ���				
//				CString num = tr.GetID(i, 144);         //ί������
//				CString price = tr.GetID(i, 145);       //�۸�
//				CString status = tr.GetID(i, 163);      //״̬
//				printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
//				if (status.Compare("�ϵ�") == 0 || status.Compare("�ѳ�") == 0)
//				{
//					if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
//					{
//						if (status.Compare("�ϵ�") == 0)
//						{
//							SendCancel(con, "",  num, price);
//						}else if (status.Compare("�ѳ�") == 0)
//						{
//							SendCancel(con, "",  num, price);
//						}
//						checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
//					}
//				}
//			}
//
//		}
//	}
//	else{
//		printf("ErrInfo = %s\n", ErrInfo);
//	}
//
//	delete [] Result;
//	return rt;
//}
//
//void checkDay()
//{
//	static int day = -1;
//	SYSTEMTIME sys; 
//	GetLocalTime(&sys); 
//	//printf( "%4d/%02d/%02d %02d:%02d:%02d.%03d ����%1d\n", sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,sys.wDayOfWeek);
//	//����, 5���, ���
//	if (day != sys.wDay && sys.wHour == 5 && sys.wMinute == 30)
//	{
//		check.clear();        //�ɽ��������
//		checkEntrust.clear(); //ί���������
//		pLog->PrintLog("��ճɽ���ί�м�¼, wDay=%d, wHour=%d, wMinute=%d\n", sys.wDay, sys.wHour, sys.wMinute);
//		day = sys.wDay;
//	}
//}
//
////����������
//void DealTotalReq(string& text, int funid)
//{
//	sys->tradeParse(text, funid);
//	//old deal
//	//Json::Reader reader;  
//	//Json::Value root;  
//	//if (reader.parse(text.c_str(), root))  // reader��Json�ַ���������root��root������Json��������Ԫ��   
//	//{
//	//	//��ȡ�µ�ָ�����
//	//	int  functionID = root["func"].asInt(); 
//	//	if (funid != 0)
//	//	{
//	//		functionID = funid;
//	//	}
//	//	printf("functionID = %d\n", functionID);
//	//	pLog->PrintLog("%s, functionID=%d\n", __FUNCTION__, functionID);	 
//	//	switch(functionID)
//	//	{
//	//	case eFunctionID_OrderInsert://ί���µ� 
//	//		EnterCriticalSection(&cs);
//	//		InsertOrder(root);
//	//		LeaveCriticalSection(&cs);
//	//		break;	
//	//	case eFunctionID_OrderDelete://����
//	//		EnterCriticalSection(&cs);
//	//		OrderDelete(root);
//	//		LeaveCriticalSection(&cs);
//	//		break;
//	//	case eFunctionID_MoneyQry://�˻��ʽ��ѯ
//	//		EnterCriticalSection(&cs);
//	//		QryMoney( root);
//	//		LeaveCriticalSection(&cs);
//	//		break;
//	//	case eFunctionID_HoldQry://�ֲֲ�ѯ
//	//		EnterCriticalSection(&cs);
//	//		QryHold();
//	//		LeaveCriticalSection(&cs);
//	//		break;
//	//	default:
//	//		break;
//	//	}		
//	//}
//	//else
//	//{
//	//	printf("json����ʧ��, text=%s\n", text.c_str());
//	//	pLog->PrintLog("json����ʧ��, text=%s\n", text.c_str());
//	//}	
//}
//
//
//
//
////ģ�ⷢ�ͽ���
//const char* SendTradeErrorFormat(std::string& out)
//{
//	char szBuf[10] = { 0 };
//	Json::Value new_item;
//	new_item["functionID"] = eFunctionID_OrderInsert;
//	new_item["orderNo"] = 1;
//	new_item["orderVol"] = 100;
//	new_item["orderPrice"]= 5.03;
//	new_item["orderType"] = 0;
//	new_item["orderMode"] = 0;
//	new_item["orderCode"] = "002087";
//
//	// ����޸�ʽjson�ַ���   
//	Json::FastWriter writer;
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	//pLog->PrintLog("out=%s", out2.c_str());
//	return out.c_str();
//}
//
//const char* SendTradeRspFormat(std::string& out)
//{
//	//char szBuf[10] = { 0 };
//	Json::Value new_item;
//	new_item["functionID"] = eFunctionID_OrderInsert;
//	new_item["orderNo"] = 1;
//	new_item["orderVol"] = 100;
//	new_item["orderPrice"]= 5.01;
//	new_item["orderType"] = 0;
//	new_item["orderMode"] = 0;
//	new_item["orderCode"] = "002087";
//	new_item["code"] = "����";
//
//	// ����޸�ʽjson�ַ���   
//	Json::FastWriter writer;
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	//pLog->PrintLog("out=%s", out2.c_str());
//	return out.c_str();
//}


//int _tmain1(int argc, _TCHAR* argv[])
//{
//	char szPath[1024] = {0};
//	char szIni[1024] = {0};
//	
//	//������־����
//	pLog = CreateLogformObj();
//	if (!pLog)
//	{
//		printf("create log error\n");
//		return -1;
//	}	
//	GetAppPath(szPath);
//	pLog->SetDir(szPath);
//	pLog->SetLogFileName("trade");
//	pLog->PrintLog("-----------program trade begin------------\n");
//
//	iniFile = new CIniFile();
//	sprintf(szIni, "%s//count.ini", szPath);
//	iniFile->SetFileName(szIni);
//
//	//��ʼ���ٽ���  
//	InitializeCriticalSection(&cs);
//	activemq::library::ActiveMQCPP::initializeLibrary();
//
//	std::cout << "=====================================================\n";
//	std::cout << "program trade begin:" << std::endl;
//	std::cout << "-----------------------------------------------------\n";
//
//	std::string brokerURI =
//		"failover:(tcp://114.55.103.143:61616"    //localhost,http://114.55.103.143
//		//        "?wireFormat=openwire"
//		//        "&transport.useInactivityMonitor=false"
//		//        "&connection.alwaysSyncSend=true"
//		//        "&connection.useAsyncSend=true"
//		//        "?transport.commandTracingEnabled=true"
//		//        "&transport.tcpTracingEnabled=true"
//		//        "&wireFormat.tightEncodingEnabled=true"
//		")";
//
//	hDll = LoadLibrary("TdxApi.Dll");
//	printf("program hDll = %p\n", hDll);
//
//	if (hDll==NULL)
//	{
//		printf("���׽ӿڼ���ʧ��\n");
//		pLog->PrintLog("���׽ӿڼ���ʧ��\n");
//		getchar();
//		return -1;
//	}
//
//	TdxInit=(FTdxInit)GetProcAddress(hDll, "TdxInit");
//	TdxExit=(FTdxExit)GetProcAddress(hDll, "TdxExit");
//	TdxLogin=(FTdxLogin)GetProcAddress(hDll, "TdxLogin");
//	TdxLogout=(FTdxLogout)GetProcAddress(hDll, "TdxLogout");
//	TdxQueryData=(FTdxQueryData)GetProcAddress(hDll, "TdxQueryData");
//	TdxSendOrder=(FTdxSendOrder)GetProcAddress(hDll, "TdxSendOrder");
//	TdxCancelOrder=(FTdxCancelOrder)GetProcAddress(hDll, "TdxCancelOrder");
//	TdxGetQuote=(FTdxGetQuote)GetProcAddress(hDll, "TdxGetQuote");
//	TdxQueryHistory=(FTdxQueryHistory)GetProcAddress(hDll, "TdxQueryHistory");
//	TdxSendOrderNew=(FTdxSendOrderNew)GetProcAddress(hDll, "TdxSendOrderNew");
//	TdxCancelOrderNew=(FTdxCancelOrderNew)GetProcAddress(hDll, "TdxCancelOrderNew");
//	GetSubmitUName=(FGetSubmitUName)GetProcAddress(hDll, "GetSubmitUName");
//	GetSubmitUID=(FGetSubmitUID)GetProcAddress(hDll, "GetSubmitUID");
//	GetDestName=(FGetDestName)GetProcAddress(hDll, "GetDestName");
//	GetBranchName=(FGetBranchName)GetProcAddress(hDll, "GetBranchName");
//	GetBranchID=(FGetBranchID)GetProcAddress(hDll, "GetBranchID");
//	GetClientConfig=(FGetClientConfig)GetProcAddress(hDll, "GetClientConfig");
//	GetOemQstag=(FGetOemQstag)GetProcAddress(hDll, "GetOemQstag");
//	GetServDesc=(FGetServDesc)GetProcAddress(hDll, "GetServDesc");
//
//	char Server[100] = "59.40.79.53";
//	int port = 7708;
//	int atype = 8;
//	int id = 10;
//	char Ver[100] = "6.00";
//	
//	char TradePass[100] = "258258";
//	char CommPass[100] = "";
//	int rt = -1;
//	char *Result = new char[1024 * 1024];
//	TdxRowset CheDanList;
//
//	//�õ�exeִ��·��.  
//	char tcExePath[MAX_PATH] = { 0 };
//	::GetModuleFileNameA(NULL, tcExePath, MAX_PATH);
//	char *pFind = strrchr(tcExePath, '\\');
//	if (pFind == NULL)
//	{
//		return -1;
//	}
//	*pFind = '\0';
//
//	string szIniPath = tcExePath;
//	szIniPath += "\\";
//	szIniPath += "service.ini";
//	cout << szIniPath << endl;
//
//	const char* pPath = szIniPath.c_str();
//	pInfo = new GobalInfo();
//	pInfo->ReadIni(pPath);  //��ȡ�����ļ�
//	strcpy(Server, pInfo->_ip.c_str());
//    port = pInfo->_port;
//    id = atoi(pInfo->_department.c_str());
//	strcpy(Ver, pInfo->_ver.c_str());
//	strcpy(Account, pInfo->_account.c_str());
//	strcpy(TradePass, pInfo->_tradePass.c_str());
//	strcpy(CommPass, pInfo->_commPass.c_str());
//	brokerURI = pInfo->_strBrokerURI;
//
//	SetConsoleTitle(pInfo->_title.c_str());
//
//
//	rt = TdxInit();
//	if (-1 == rt){
//		printf("init failed\n");
//	}else{
//		printf("init ok, ret = %d\n", rt);
//	}
//    
//	//login
//	ClientID = TdxLogin(Server, port, Ver, atype, id, Account, TradePass, CommPass, ErrInfo);
//	if (ClientID == -1)
//	{ 
//        printf("��¼ʧ��, ErrInfo=%s\n", ErrInfo);
//		pLog->PrintLog("��¼ʧ��, server=%s, port=%d, account=%s,ErrInfo=%s\n", Server, port, Account, ErrInfo);
//		//getchar();
//		return -1;
//	}else{
//		pLog->PrintLog("%s:%d,%s, ����ɹ�\n", Server, port, Account);
//		printf("����ɹ�\n");
//	}
//
//	Sleep(200);
//	FSubmitUID = GetSubmitUID(ClientID);
//
//	//��ѯ�ɶ���Ϣ
//	rt = TdxQueryData(ClientID, Account, 5, Result, ErrInfo);
//	if (rt == 1)
//	{
//		printf("Result = %s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);	
//		CString Ac = Account;
//  //      m_Account.GetWindowTextA(Ac);
//		for (int i = 0; i < tr.FRowCount; i++)
//		{
//			CString s = tr.GetID(i, 121); //�ʽ��˺�
//			CString s1 = tr.GetID(i, 281);//���ʱ�־
//			CString s2 = tr.GetID(i, 125);//�г����  0����A 1�Ϻ�A 2=����B 3=�Ϻ�B 12=��ת
//			CString s3 = tr.GetID(i, 123);//�ɶ�����
//
//			if (s1 == "1")
//			{
//				if (RAccount == "")
//				{
//					RAccount = s;
//					if (s == "0")
//						RAccount =Ac;
//				}
//				if (s2 == "0")
//					RSZAID = s3;
//				if (s2 == "1")
//					RSHAID = s3;
//				if (s != "0")
//					continue;
//			}
//			if (GAccount == "")
//			{
//				GAccount = s;
//				if ((s == "0") || (s == ""))
//					GAccount = Ac;
//			}
//			if ((s2 == "0") && (GSZAID == ""))
//				GSZAID = s3;
//			if ((s2 == "1") && (GSHAID == ""))
//				GSHAID = s3;
//		}
//
//		if (RAccount == "")
//			RAccount = GAccount;
//		if (RSHAID == "")
//			RSHAID = GSHAID;
//		if (RSZAID == "")
//			RSZAID = GSZAID;
//	}
//	else
//	{
//		printf("ȡ�ɶ���Ϣʧ��,ErrInfo = %s\n", ErrInfo);
//	}
//	memset(Result,0, 1024*1024);
//
//	std::string strDest3 = "query-order-success";
//	qrySuccessProcduct.CreateProduct(brokerURI, strDest3);
//
//	////----------------��ѯ�ʽ��б�-----------------
//	////0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//	//rt = TdxQueryData(ClientID, Account, 0, Result, ErrInfo);
//	//if (rt == 1)
//	//{
//	//	printf("�ֹ��б�, Result=%s\n", Result);
//	//	pLog->PrintLog("------------zhijin------------\n");
//	//	pLog->PrintLog("%s\n", Result);
//	//	TdxRowset tr;
//	//	tr.Update(Result);
//	//	for (int i = 0; i < tr.FRowCount; i++)
//	//	{
//	//		//��ѯ�ʽ�
//	//		CString s1 = tr.GetID(i,113);
//	//		CString assets = tr.GetID(i, 310);      //���ʲ�
//	//		CString marketValue = tr.GetID(i, 205); //������ֵ
//	//		CString balance = tr.GetID(i, 301);     //�����ʽ�
//	//		//tr.GetID(0,113);
//	//		printf("%s, %s, %s\n", assets, marketValue, balance);
//
//	//		////�ֲֽ���
//	//		//CString code = tr.GetID(i, 140);         //֤ȯ����
//	//		//CString name = tr.GetID(i, 141);         //֤ȯ����
//	//		//CString num = tr.GetID(i, 200);          //֤ȯ����
//	//		//CString costprice = tr.GetID(i, 202);    //�ɱ���
//	//		//CString balance = tr.GetID(i, 204);      //����ӯ��
//
//	//		//printf("%s, %s, %s, %s, %s\n", code, name, num, costprice, balance);
//
//	//		//�ɽ�����
//	//		//CString con = tr.GetID(i, 146);      //��ͬ���
//	//		//CString assets = tr.GetID(i, 155);      //�ɽ����
//	//		//CString marketValue = tr.GetID(i, 152); //�ɽ�����
//	//		//CString balance = tr.GetID(i, 153);     //�ɽ��۸�
//	//		//printf("%s, %s, %s, %s\n", con, assets, marketValue, balance);
//
//	//		//SendDeal( con, assets,  marketValue, balance);
//	//	}
//
//	////	//CString assets = tr.GetID(0, 310);      //���ʲ�
//	////	//CString marketValue = tr.GetID(0, 205); //������ֵ
//	////	//CString balance = tr.GetID(0, 301);     //�����ʽ�
//	////	//printf("%s, %s, %s\n", assets, marketValue, balance);
//
//	//}
//	//else{
//	//	printf("ErrInfo = %s\n", ErrInfo);
//	//}
//	//memset(Result,0, 1024*1024);
//	//getchar();
//	// delete [] Resul
//
// //   std::string out;
//	//std::string strDest2 = "abc";
//	//tradeProcductSend.CreateProduct(brokerURI, strDest2);
//	//SendTradeRspFormat(out);
//
//	////tradeProcductSend.SendData(out);
//	//getchar();
//	//------------��Ϣ����-----------
//	char szBuf[512]={0};
//	sprintf(szBuf, "cmd-%d-%s", pInfo->_acountComId, pInfo->_account.c_str()); 
//	printf("receive cmd: %s\n", szBuf);
//    std::string strDest = szBuf;//pInfo->_trade2jna;//"OrderToTrade"
//	//��������
//	MqConsumer testConsumer(brokerURI, strDest);
//	Thread consumerThread1(&testConsumer);
//	consumerThread1.start();
//	//MqConsumer deleteConsumer(brokerURI, "DeleteToTrade");
//	//Thread consumerThread2(&deleteConsumer);
//	//consumerThread2.start();
//
//    //�������׷���
//	std::string strTradeToOrderDest = "query-entrust-cno"; //pInfo->_jna2trade; //"TradeToOrder";
//	tradeProcduct.CreateProduct(brokerURI, strTradeToOrderDest);
//	strTradeToOrderDest = "query-order-cancel";
//	deleteProcduct.CreateProduct(brokerURI, strTradeToOrderDest);
//
//	strTradeToOrderDest = "query-amount";
//	qryMoneyProcduct.CreateProduct(brokerURI, strTradeToOrderDest);
//
//	Sleep(1);
//
//	DWORD ThreadID =0;  
//	HANDLE hThread[5] = {0};  
//	for (int i = 0;i < 1; i++)  
//	{  
//		//�����̣߳�������Threadд�ļ�  
//		hThread[i] = CreateThread(NULL, 0, ThreadFunc, (LPVOID)(i + 1), 0, &ThreadID);  
//		printf("Thread #%d has been created successfully.\n", i + 1);  
//	}  
//
//    getchar();
//	getchar();
//
//	
//
//	printf("program trade over\n");
//	getchar();
//	return 0;
//}

//----------------��ѯ�����б�-----------------
////0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
//rt = TdxQueryData(ClientID, Account, 4, Result, ErrInfo);
//if (rt == 1)
//{
//	printf("�����б�, Result=%s\n", Result);
//	CheDanList.Update(Result);
//	//CheDanList.RowToListCtrl(m_che);
//}
//else{
//	printf("ErrInfo = %s\n", ErrInfo);
//}
//memset(Result,0, 1024*1024);
// // delete [] Result;
//Sleep(1000);
//-------------����-------------------
////����
//char code[20] = "002087";
//if (strlen(code)!=6)
//	return -2;
////char price1[20];
////m_price.GetWindowTextA(price1,20);
//double price=4.82f;
////char num1[20];
////m_num.GetWindowTextA(num1,20);
//   int num = 100;
//CString gdcode=GSZAID;
//if (code[0]=='6')
//	gdcode=GSHAID;
//rt = TdxSendOrder(ClientID, 0, 0, (LPSTR)(LPCTSTR)gdcode, code, price, num, Result, ErrInfo);
//if (rt == 1)
//{
//	printf("trade ok, Result=%s\n", Result);
//	TdxRowset tr;
//	tr.Update(Result);
//	CString s = tr.GetID(0, 146);
//	printf("s = %s\n", (LPSTR)(LPCTSTR)s);
//}
//else{
//	printf("trade failed, ErrInfo=%s\n", ErrInfo);
//}
//memset(Result,0, 1024*1024);	

//delete [] Result;

//   //����
//char code[20] = "002087";
//if (strlen(code)!=6)
//	return -2;
////char price1[20];
////m_price.GetWindowTextA(price1,20);
//double price=4.98f;
////char num1[20];
////m_num.GetWindowTextA(num1,20);
//int num = 100;
//CString gdcode=GSZAID;
//if (code[0]=='6')
//	gdcode=GSHAID;
//rt = TdxSendOrder(ClientID, 1, 0, (LPSTR)(LPCTSTR)gdcode, code, price, num, Result, ErrInfo);
//if (rt == 1)
//{
//	printf("trade ok, Result=%s\n", Result);
//	TdxRowset tr;
//	tr.Update(Result);
//	//tr.RowToListBox(outlist);
//}
//else{
//	printf("trade failed\n");
//	//outlist.AddString(ErrInfo);
//}
//memset(Result,0, 1024*1024);	

//delete [] Result;

//---------------����--------------------
//int i = 0;
//   CString orderid = CheDanList.GetID(i, 146);
//CString stockcode = CheDanList.GetID(i, 140);
//CString stkname = CheDanList.GetID(i, 141);
//CString buyorsell = CheDanList.GetID(i, 131);
//CString mks = CheDanList.GetID(i, 100);
//rt = TdxCancelOrder(ClientID, (LPSTR)(LPCTSTR)stockcode, (LPSTR)(LPCTSTR)orderid, (LPSTR)(LPCTSTR)mks, Result, ErrInfo);
//if (rt == 1)
//{
//	printf("����, Result=%s\n", Result);
//	TdxRowset tr;
//	tr.Update(Result);
//	//tr.RowToListBox(outlist);
//}
//else
//	printf("ErrInfo = %s\n", ErrInfo);



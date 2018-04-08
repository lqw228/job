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

MqProducer tradeProcduct;        //交易生产者
MqProducer deleteProcduct;       //撤单生产者
MqProducer qryMoneyProcduct;     //查询资金账号生产者
MqProducer qrySuccessProcduct;   //查询成交记录生产者
MqProducer qryHoldProcduct;      //查询持仓
MqProducer qryHoldTopicProcduct; //查询持仓发送广播
MqProducer qryMoneyTopicProcduct; //查询资金内容发送广播


GobalInfo* pInfo = NULL;

CRITICAL_SECTION cs;           //定义临界区全局变量
map<string, int> check;        //成交区别
map<string, int> checkEntrust; //委托区别

ILogEvent* pLog = NULL;
CIniFile* iniFile = NULL;
TradeSystem* sys = NULL;//交易系统

//处理总请求
void DealTotalReq(string& text, int funid)
{
	sys->tradeParse(text, funid);
}

//线程函数：轮询成交和委托 
DWORD WINAPI ThreadFunc(LPVOID lpParam)  
{  
    int n = (int)lpParam;
	int nCount = 0;

	while (1)
	{
		Sleep(5000);
		//进入临界区  
		EnterCriticalSection(&cs);
		sys->checkDay(); //隔天清空记录内容
		sys->QryDeal();
		sys->QryEntrust();
		sys->QryHold(false);
		sys->QryMoney(false);
		//离开临界区  
		LeaveCriticalSection(&cs); 
		
	}
	printf("Thread #%d returned successfully\n", n);  
	return 0;  
}

int _tmain(int argc, _TCHAR* argv[])
{
	activemq::library::ActiveMQCPP::initializeLibrary();	 
	InitializeCriticalSection(&cs);//初始化临界区 
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
	//-----------------------创建mq交易发送----------------
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

	//--------------创建mq接收-----------
	char szBuf[512]={0};
	sprintf(szBuf, "cmd-%d-%s", pInfo->_acountComId, pInfo->_account.c_str()); 
	printf("receive cmd: %s\n", szBuf);
	std::string strDest = szBuf;	
	MqConsumer testConsumer(brokerURI, strDest);
	Thread consumerThread1(&testConsumer);
	consumerThread1.start();

	DWORD ThreadID =0;  
	HANDLE hThread = NULL;  
	//创建线程，轮询成交 
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
//	//直接对jsoncpp对象以数字索引作为下标进行赋值，则自动作为数组
//	iNum[1] = 1;
//	iNum[2] = 2;
//	iNum[3] = 3;
//	iNum[4] = 4;
//	iNum[5] = 5;
//	iNum[6] = 6;
//
//	//增加对象数组
//	root["array"]    = arrayObj;
//	//增加字符串
//	root["name"]    = "json";
//	//增加数字
//	root["number"]    = 666;
//	//增加布尔变量
//	root["value"]    = true;
//	//增加数字数组
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

//FTdxInit TdxInit=NULL; //函数指针
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
//int ClientID = -1;     //连接ID，-1为未连接
//char ErrInfo[256] = {0};

//char Account[100] = "3000087958";

//CString MAccount ;//当前登录主账号
//CString FSubmitUID;
//
//CString GAccount ;//股票账号
//CString GSZAID ;//深A 0
//CString GSHAID ;//沪A 1
//CString GSZBID ;//  B 2
//CString GSHBID ;//  B 3
//CString FGZID ;//  B 12
//
//CString RAccount ;//融资账号
//CString RSZAID ;//深A 0 融资账号
//CString RSHAID ;//沪A 1
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
//	////提取下单指令参数
//	std::string orderNo = root["orderNumber"].asString();
//	int num = root["volume"].asInt();
//	double price = root["price"].asDouble();
//	int TradeType = root["behavior"].asInt(); //0:买入1:卖出
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
//	//买入
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
//	// 输出无格式json字符串   
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
//		printf("撤单, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//	}
//	else
//	{
//		printf("撤单,ErrInfo = %s\n", ErrInfo);
//		pLog->PrintLog("撤单,ErrInfo = %s\n", ErrInfo);
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
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 0, Result, ErrInfo);    //0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 0, Result, ErrInfo);  //0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}	
//
//	new_item["ret"] = rt;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//	if (rt == 1)
//	{
//		printf("资金列表, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//		CString assets = tr.GetID(0, 310);      //总资产
//		CString marketValue = tr.GetID(0, 205); //最新市值
//		CString balance = tr.GetID(0, 301);     //可用资金
//		printf("%s, %s, %s\n", assets, marketValue, balance);
//		new_item["assets"] = (LPSTR)(LPCTSTR)assets;
//		new_item["marketValue"] = (LPSTR)(LPCTSTR)marketValue;
//		new_item["balance"] = (LPSTR)(LPCTSTR)balance;
//	}
//	else{
//		printf("资金, ErrInfo = %s\n", ErrInfo);
//		pLog->PrintLog("资金, ErrInfo = %s\n", ErrInfo);
//		new_item["errInfo"] = G2U(ErrInfo);
//	}
//
//	//发送回报内容
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("QryMoney callback, out=%s\n", out.c_str());
//	qryMoneyProcduct.SendData(out);
//	delete [] Result;
//}
//
////查询持仓列表
//int QryHold(){
//	Json::Value new_item;
//	std::string out;
//	Json::FastWriter writer;
//	int rt = 0;	
//	char *Result = new char[1024 * 1024];
//
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) != 0)
//	{		
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 1, Result, ErrInfo);     //0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 1, Result, ErrInfo); //0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}
//
//	new_item["ret"] = rt;
//	new_item["acountComId"] = pInfo->_acountComId; 
//	new_item["cashAcount"] = pInfo->_account.c_str();
//
//	if (rt == 1)
//	{
//		printf("持仓股份列表, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//		for (int i = 0; i < tr.FRowCount; i++)
//		{
//			if (eTradeID_WUKUANG == pInfo->_acountComId || 
//				eTradeID_GUANGFA == pInfo->_acountComId || 
//				eTradeID_CAITONG == pInfo->_acountComId)
//			{
//				//持仓解析
//				CString code = tr.GetID(i, 140);         //证券代码
//				CString name = tr.GetID(i, 141);         //证券名称
//				CString num = tr.GetID(i, 200);          //证券数量
//				CString costprice = tr.GetID(i, 202);    //成本价
//				CString currentprice = tr.GetID(i, 949); //当前价
//				CString balance = tr.GetID(i, 204);      //浮动盈亏
//
//				printf("%s, %s, %s, %s, %s, %s\n", code, name, num, costprice, currentprice, balance);
//				//返回对应通道
//
//			}
//
//		}
//	}
//	else{
//		printf("持仓, ErrInfo = %s\n", ErrInfo);
//		pLog->PrintLog("持仓, ErrInfo = %s\n", ErrInfo);
//		new_item["errInfo"] = G2U(ErrInfo);
//	}
//
//	//发送回报内容
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("QryHold callback, out=%s\n", out.c_str());
//
//	delete [] Result;
//	return rt;
//}
//
////发送成交主推回报
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
//	//发送回报内容
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
//	qrySuccessProcduct.SendData(out);	
//
//}
//
////发送撤单主推回报
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
//	//发送回报内容
//	out = writer.write(new_item);
//	printf("out=%s\n", out.c_str());
//	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
//	deleteProcduct.SendData(out);
//}
//
////查询当日成交
//int QryDeal()
//{
//	static int myCount = 0;
//	static int errorNum = 0; 
//	int rt = 0;
//	char *Result = new char[1024 * 1024];
//
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) != 0)
//	{		
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 3, Result, ErrInfo); //0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 3, Result, ErrInfo);//0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}
//	if (rt == 1)
//	{
//		errorNum = 0;
//		if (myCount > 100000)
//		{
//			myCount=0;
//		}
//		iniFile->WriteIniInt_In("trade", "count", myCount++);
//		printf("当日成交列表, Result=%s\n", Result);
//		TdxRowset tr;
//		tr.Update(Result);
//		for (int i = 0; i < tr.FRowCount; i++)
//		{
//			if (eTradeID_WUKUANG == pInfo->_acountComId)
//			{//五矿
//				CString con = tr.GetID(i, 146);         //合同编号
//				CString assets = tr.GetID(i, 155);      //成交编号
//				CString marketValue = tr.GetID(i, 152); //成交数量
//				CString balance = tr.GetID(i, 153);     //成交价格
//				CString status = tr.GetID(i, 147);      //状态
//				printf("%s, %s, %s, %s, %s\n", con, assets, marketValue, balance, status);
//
//				if (0 == check[(LPSTR)(LPCTSTR)assets])
//				{
//					if (status.Compare("普通成交") == 0)
//					{
//						SendDeal(con, assets,  marketValue, balance);
//					}else if (status.Compare("内部撤单") == 0)
//					{
//						SendCancel(con, assets,  marketValue, balance);
//					}else if (status.Compare("撤单成交") == 0)
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
//			{//广发证券,财通证券
//				CString con = tr.GetID(i, 146);         //合同编号
//				CString assets = tr.GetID(i, 155);      //成交编号
//				CString marketValue = tr.GetID(i, 152); //成交数量
//				CString balance = tr.GetID(i, 153);     //成交价格
//				//CString status = tr.GetID(i, 147);      //状态
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
////查询当日委托列表
//int QryEntrust(){
//	int rt = 0;
//	TdxRowset tr;
//	char *Result = new char[1024 * 1024];
//
//	if (FSubmitUID.Compare(pInfo->_account.c_str()) != 0)
//	{		
//		rt = TdxQueryData(ClientID, (LPSTR)(LPCTSTR)FSubmitUID, 2, Result, ErrInfo); //0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}else
//	{		
//		rt = TdxQueryData(ClientID, (char*)pInfo->_account.c_str(), 2, Result, ErrInfo); //0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	}
//
//	if (rt == 1)
//	{
//		printf("当日委托列表, Result=%s\n", Result);		
//		tr.Update(Result);
//		for (int i = 0; i < tr.FRowCount; i++)
//		{
//			if (eTradeID_GUANGFA == pInfo->_acountComId || eTradeID_CAITONG == pInfo->_acountComId 
//				|| eTradeID_DONGBEI == pInfo->_acountComId || eTradeID_HENGTAI == pInfo->_acountComId || 
//				eTradeID_CUOTAIJUNAN == pInfo->_acountComId)
//			{
//				CString con = tr.GetID(i, 146);         //合同编号				
//				CString num = tr.GetID(i, 144);         //委托数量
//				CString price = tr.GetID(i, 145);       //价格
//				CString status = tr.GetID(i, 147);      //状态
//				printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
//				if (status.Compare("废单") == 0 || status.Compare("已撤") == 0)
//				{
//					if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
//					{
//						if (status.Compare("废单") == 0)
//						{
//							SendCancel(con, "",  num, price);
//						}else if (status.Compare("已撤") == 0)
//						{
//							SendCancel(con, "",  num, price);
//						}
//						checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
//					}
//				}
//			}else if (eTradeID_GUOYUAN == pInfo->_acountComId)
//			{
//				CString con = tr.GetID(i, 146);         //合同编号				
//				CString num = tr.GetID(i, 144);         //委托数量
//				CString price = tr.GetID(i, 145);       //价格
//				CString status = tr.GetID(i, 163);      //状态
//				printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
//				if (status.Compare("废单") == 0 || status.Compare("已撤") == 0)
//				{
//					if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
//					{
//						if (status.Compare("废单") == 0)
//						{
//							SendCancel(con, "",  num, price);
//						}else if (status.Compare("已撤") == 0)
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
//	//printf( "%4d/%02d/%02d %02d:%02d:%02d.%03d 星期%1d\n", sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,sys.wDayOfWeek);
//	//隔天, 5点半, 清空
//	if (day != sys.wDay && sys.wHour == 5 && sys.wMinute == 30)
//	{
//		check.clear();        //成交区别清空
//		checkEntrust.clear(); //委托区别清空
//		pLog->PrintLog("清空成交和委托记录, wDay=%d, wHour=%d, wMinute=%d\n", sys.wDay, sys.wHour, sys.wMinute);
//		day = sys.wDay;
//	}
//}
//
////处理总请求
//void DealTotalReq(string& text, int funid)
//{
//	sys->tradeParse(text, funid);
//	//old deal
//	//Json::Reader reader;  
//	//Json::Value root;  
//	//if (reader.parse(text.c_str(), root))  // reader将Json字符串解析到root，root将包含Json里所有子元素   
//	//{
//	//	//提取下单指令参数
//	//	int  functionID = root["func"].asInt(); 
//	//	if (funid != 0)
//	//	{
//	//		functionID = funid;
//	//	}
//	//	printf("functionID = %d\n", functionID);
//	//	pLog->PrintLog("%s, functionID=%d\n", __FUNCTION__, functionID);	 
//	//	switch(functionID)
//	//	{
//	//	case eFunctionID_OrderInsert://委托下单 
//	//		EnterCriticalSection(&cs);
//	//		InsertOrder(root);
//	//		LeaveCriticalSection(&cs);
//	//		break;	
//	//	case eFunctionID_OrderDelete://撤单
//	//		EnterCriticalSection(&cs);
//	//		OrderDelete(root);
//	//		LeaveCriticalSection(&cs);
//	//		break;
//	//	case eFunctionID_MoneyQry://账户资金查询
//	//		EnterCriticalSection(&cs);
//	//		QryMoney( root);
//	//		LeaveCriticalSection(&cs);
//	//		break;
//	//	case eFunctionID_HoldQry://持仓查询
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
//	//	printf("json解析失败, text=%s\n", text.c_str());
//	//	pLog->PrintLog("json解析失败, text=%s\n", text.c_str());
//	//}	
//}
//
//
//
//
////模拟发送交易
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
//	// 输出无格式json字符串   
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
//	new_item["code"] = "中文";
//
//	// 输出无格式json字符串   
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
//	//创建日志对象
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
//	//初始化临界区  
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
//		printf("交易接口加载失败\n");
//		pLog->PrintLog("交易接口加载失败\n");
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
//	//得到exe执行路径.  
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
//	pInfo->ReadIni(pPath);  //读取配置文件
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
//        printf("登录失败, ErrInfo=%s\n", ErrInfo);
//		pLog->PrintLog("登录失败, server=%s, port=%d, account=%s,ErrInfo=%s\n", Server, port, Account, ErrInfo);
//		//getchar();
//		return -1;
//	}else{
//		pLog->PrintLog("%s:%d,%s, 登入成功\n", Server, port, Account);
//		printf("登入成功\n");
//	}
//
//	Sleep(200);
//	FSubmitUID = GetSubmitUID(ClientID);
//
//	//查询股东信息
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
//			CString s = tr.GetID(i, 121); //资金账号
//			CString s1 = tr.GetID(i, 281);//融资标志
//			CString s2 = tr.GetID(i, 125);//市场类别  0深圳A 1上海A 2=深圳B 3=上海B 12=股转
//			CString s3 = tr.GetID(i, 123);//股东代码
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
//		printf("取股东信息失败,ErrInfo = %s\n", ErrInfo);
//	}
//	memset(Result,0, 1024*1024);
//
//	std::string strDest3 = "query-order-success";
//	qrySuccessProcduct.CreateProduct(brokerURI, strDest3);
//
//	////----------------查询资金列表-----------------
//	////0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//	//rt = TdxQueryData(ClientID, Account, 0, Result, ErrInfo);
//	//if (rt == 1)
//	//{
//	//	printf("持股列表, Result=%s\n", Result);
//	//	pLog->PrintLog("------------zhijin------------\n");
//	//	pLog->PrintLog("%s\n", Result);
//	//	TdxRowset tr;
//	//	tr.Update(Result);
//	//	for (int i = 0; i < tr.FRowCount; i++)
//	//	{
//	//		//查询资金
//	//		CString s1 = tr.GetID(i,113);
//	//		CString assets = tr.GetID(i, 310);      //总资产
//	//		CString marketValue = tr.GetID(i, 205); //最新市值
//	//		CString balance = tr.GetID(i, 301);     //可用资金
//	//		//tr.GetID(0,113);
//	//		printf("%s, %s, %s\n", assets, marketValue, balance);
//
//	//		////持仓解析
//	//		//CString code = tr.GetID(i, 140);         //证券代码
//	//		//CString name = tr.GetID(i, 141);         //证券名称
//	//		//CString num = tr.GetID(i, 200);          //证券数量
//	//		//CString costprice = tr.GetID(i, 202);    //成本价
//	//		//CString balance = tr.GetID(i, 204);      //浮动盈亏
//
//	//		//printf("%s, %s, %s, %s, %s\n", code, name, num, costprice, balance);
//
//	//		//成交解析
//	//		//CString con = tr.GetID(i, 146);      //合同编号
//	//		//CString assets = tr.GetID(i, 155);      //成交编号
//	//		//CString marketValue = tr.GetID(i, 152); //成交数量
//	//		//CString balance = tr.GetID(i, 153);     //成交价格
//	//		//printf("%s, %s, %s, %s\n", con, assets, marketValue, balance);
//
//	//		//SendDeal( con, assets,  marketValue, balance);
//	//	}
//
//	////	//CString assets = tr.GetID(0, 310);      //总资产
//	////	//CString marketValue = tr.GetID(0, 205); //最新市值
//	////	//CString balance = tr.GetID(0, 301);     //可用资金
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
//	//------------消息队列-----------
//	char szBuf[512]={0};
//	sprintf(szBuf, "cmd-%d-%s", pInfo->_acountComId, pInfo->_account.c_str()); 
//	printf("receive cmd: %s\n", szBuf);
//    std::string strDest = szBuf;//pInfo->_trade2jna;//"OrderToTrade"
//	//创建接收
//	MqConsumer testConsumer(brokerURI, strDest);
//	Thread consumerThread1(&testConsumer);
//	consumerThread1.start();
//	//MqConsumer deleteConsumer(brokerURI, "DeleteToTrade");
//	//Thread consumerThread2(&deleteConsumer);
//	//consumerThread2.start();
//
//    //创建交易发送
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
//		//创建线程，并调用Thread写文件  
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

//----------------查询撤单列表-----------------
////0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
//rt = TdxQueryData(ClientID, Account, 4, Result, ErrInfo);
//if (rt == 1)
//{
//	printf("撤单列表, Result=%s\n", Result);
//	CheDanList.Update(Result);
//	//CheDanList.RowToListCtrl(m_che);
//}
//else{
//	printf("ErrInfo = %s\n", ErrInfo);
//}
//memset(Result,0, 1024*1024);
// // delete [] Result;
//Sleep(1000);
//-------------交易-------------------
////买入
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

//   //卖出
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

//---------------撤单--------------------
//int i = 0;
//   CString orderid = CheDanList.GetID(i, 146);
//CString stockcode = CheDanList.GetID(i, 140);
//CString stkname = CheDanList.GetID(i, 141);
//CString buyorsell = CheDanList.GetID(i, 131);
//CString mks = CheDanList.GetID(i, 100);
//rt = TdxCancelOrder(ClientID, (LPSTR)(LPCTSTR)stockcode, (LPSTR)(LPCTSTR)orderid, (LPSTR)(LPCTSTR)mks, Result, ErrInfo);
//if (rt == 1)
//{
//	printf("撤单, Result=%s\n", Result);
//	TdxRowset tr;
//	tr.Update(Result);
//	//tr.RowToListBox(outlist);
//}
//else
//	printf("ErrInfo = %s\n", ErrInfo);



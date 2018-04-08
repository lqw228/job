#include "StdAfx.h"
#include <iostream>

#include "System.h"
#include "trade.h"
#include "ILogEvent.h"
#include "Dir.h"
#include "IniFile.h"
#include "gobal.h"
#include "MqProducer.h"
#include "TdxRowset.h"
#include "Func.h"

using namespace std;

extern ILogEvent* pLog;
extern CIniFile* iniFile;
extern CRITICAL_SECTION cs;//定义临界区全局变量

extern MqProducer tradeProcduct;      //交易生产者
extern MqProducer deleteProcduct;     //撤单生产者
extern MqProducer qryMoneyProcduct;   //查询资金账号生产者
extern MqProducer qrySuccessProcduct; //查询成交记录
extern MqProducer qryHoldProcduct;    //查询持仓
extern MqProducer qryHoldTopicProcduct;  //查询持仓内容
extern MqProducer qryMoneyTopicProcduct; //查询资金内容

extern map<string, int> check;        //成交区别
extern map<string, int> checkEntrust; //委托区别

TradeSystem::TradeSystem(void)
{
	m_pTrade = NULL;
	m_bInit = false;
	m_bStart = false;
}

TradeSystem::~TradeSystem(void)
{
}


// 初始化
bool TradeSystem::init(void)
{
	char szPath[1024] = {0};
	char szIni[1024] = {0};	

	//创建日志对象
	pLog = CreateLogformObj();
	if (!pLog)
	{
		printf("create log error\n");
		return false;
	}	
	GetAppPath(szPath);
	pLog->SetDir(szPath);
	pLog->SetLogFileName("trade");
	pLog->PrintLog("-----------program trade begin------------\n");

	iniFile = new CIniFile();
	sprintf(szIni, "%s//count.ini", szPath);
	iniFile->SetFileName(szIni);

	//得到exe执行路径.  
	char tcExePath[MAX_PATH] = { 0 };
	::GetModuleFileNameA(NULL, tcExePath, MAX_PATH);
	char *pFind = strrchr(tcExePath, '\\');
	if (pFind == NULL)
	{
		return false;
	}
	*pFind = '\0';

	string szIniPath = tcExePath;
	szIniPath += "\\";
	szIniPath += "service.ini";
	cout << szIniPath << endl;

	const char* pPath = szIniPath.c_str();
	pInfo = new GobalInfo();
	pInfo->ReadIni(pPath);  //读取配置文件
	SetConsoleTitle(pInfo->_title.c_str());

	m_pTrade = new Trade();
	if (-1 == m_pTrade->Init())
	{
		m_bInit = false;
	}
	else
	{
		m_bInit = true;
	}

	return m_bInit;
}

bool TradeSystem::Uninit(void)
{
	return false;
}

int TradeSystem::start(void)
{
	int ret = -1;
	ret = m_pTrade->Login((char*)pInfo->_ip.c_str(), pInfo->_port,
		(char*)pInfo->_ver.c_str(), 8, atoi(pInfo->_department.c_str()), 
		(char*)pInfo->_account.c_str(), (char*)pInfo->_tradePass.c_str(), 
		(char*)pInfo->_commPass.c_str());
	return ret;
}

int TradeSystem::stop(void)
{
	return 0;
}

std::string writeJson() 
{
	using namespace std;

	Json::Value root;
	Json::Value arrayObj;
	Json::Value item;
	Json::Value iNum;


	item["string"]    = "this is a string";
	item["number"]    = 999;
	item["aaaaaa"]    = "bbbbbb";
	arrayObj.append(item);

	//直接对jsoncpp对象以数字索引作为下标进行赋值，则自动作为数组
	iNum[1] = 1;
	iNum[2] = 2;
	iNum[3] = 3;
	iNum[4] = 4;
	iNum[5] = 5;
	iNum[6] = 6;

	//增加对象数组
	root["array"]    = arrayObj;
	//增加字符串
	root["name"]    = "json";
	//增加数字
	root["number"]    = 666;
	//增加布尔变量
	root["value"]    = true;
	//增加数字数组
	root["arrnum"]    = iNum;

	root.toStyledString();
	string out = root.toStyledString();

	return out;
}

//查询持仓列表
void TradeSystem::QryHold(bool bQueue)
{
	Json::Value root;
	Json::Value arrayObj;
	Json::Value item;
	std::string out;
	Json::FastWriter writer;
	char* Result = NULL;
	char* ErrInfo = NULL;
	int rt = m_pTrade->QueryData(eQryType_HoldPosition);
	root["ret"] = rt;
	root["acountComId"] = pInfo->_acountComId; 
	root["cashAcount"] = pInfo->_account.c_str();
	Result = m_pTrade->GetResult();

	if (rt == 1)
	{
		printf("持仓股份列表, Result=%s\n", Result);
		TdxRowset tr;
		tr.Update(Result);
		for (int i = 0; i < tr.FRowCount; i++)
		{
			if (eTradeHold_COMMON == SwitchHoldType(pInfo->_acountComId))
			{
				//持仓解析
				CString code = tr.GetID(i, 140);         //证券代码
				CString name = tr.GetID(i, 141);         //证券名称
				CString num = tr.GetID(i, 200);          //证券数量
				CString costprice = tr.GetID(i, 202);    //成本价
				CString currentprice = tr.GetID(i, 949); //当前价
				CString balance = tr.GetID(i, 204);      //浮动盈亏
				CString sellnum = tr.GetID(i, 201);      //可卖数量

				if (eTradeID_WUKUANG == pInfo->_acountComId || eTradeID_CHANGCHENG == pInfo->_acountComId
					|| eTradeID_MINSHENG == pInfo->_acountComId)
				{//五矿特殊
					num = tr.GetID(i, 621);
				}

				printf("code=%s, name=%s, num=%s, costprice=%s, currentprice=%s, balance=%s\n", 
					code, name, num, costprice, currentprice, balance);				
				item["code"] = (LPSTR)(LPCTSTR)code;
				item["num"] = (LPSTR)(LPCTSTR)num;
				item["sellnum"] = (LPSTR)(LPCTSTR)sellnum;
				item["costprice"] = (LPSTR)(LPCTSTR)costprice;
				item["currentprice"] = (LPSTR)(LPCTSTR)currentprice;
				item["balance"] = (LPSTR)(LPCTSTR)balance;
				arrayObj.append(item);
			}
		}
		root["array"] = arrayObj;
	}
	else
	{
		printf("持仓, ErrInfo = %s\n", ErrInfo);
		pLog->PrintLog("持仓, ErrInfo = %s\n", ErrInfo);
		root["errInfo"] = G2U(ErrInfo);
	}

	//发送回报内容
	out = writer.write(root);
	//printf("out=%s\n", out.c_str());
	pLog->PrintLog("QryHold callback, out=%s\n", out.c_str());
	if (bQueue)
	{
		qryHoldProcduct.SendData(out);
	}
	else
	{
		qryHoldTopicProcduct.SendData(out);
	}	

	return;
}

//查询资金
void TradeSystem::QryMoney(bool bQueue)
{
	Json::Value new_item;
	std::string out;
	Json::FastWriter writer;
	char* Result = NULL;
	char* ErrInfo = NULL;
	int rt = m_pTrade->QueryData(eQryType_Fund);
	new_item["ret"] = rt;
	new_item["acountComId"] = pInfo->_acountComId; 
	new_item["cashAcount"] = pInfo->_account.c_str();
	Result = m_pTrade->GetResult();
	if (1 == rt)
	{
		printf("资金列表, Result=%s\n", Result);
		TdxRowset tr;
		tr.Update(Result);
		
		CString assets = tr.GetID(0, 310);      //总资产
		CString marketValue = tr.GetID(0, 205); //最新市值
		CString balance = tr.GetID(0, 301);     //可用资金
		if (eTradeID_PINGAN == pInfo->_acountComId)
		{
			ParsePingAnAsset(Result, assets, marketValue, balance);
		}
		printf("%s, %s, %s\n", assets, marketValue, balance);
		new_item["assets"] = (LPSTR)(LPCTSTR)assets;
		new_item["marketValue"] = (LPSTR)(LPCTSTR)marketValue;
		new_item["balance"] = (LPSTR)(LPCTSTR)balance;
	}else{
		ErrInfo = m_pTrade->GetError();
		printf("资金, ErrInfo = %s\n", ErrInfo);
		pLog->PrintLog("资金, ErrInfo = %s\n", ErrInfo);
		new_item["errInfo"] = G2U(ErrInfo);
	}
	//发送回报内容
	out = writer.write(new_item);
	printf("out=%s", out.c_str());
	pLog->PrintLog("QryMoney callback, out=%s\n", out.c_str());
	if (bQueue)
	{
		qryMoneyProcduct.SendData(out);
	}else
	{
		qryMoneyTopicProcduct.SendData(out);

	}
	

}

void TradeSystem::InsertOrder(Json::Value& root)
{
	//提取下单指令参数
	std::string orderNo = root["orderNumber"].asString();
	int num = root["volume"].asInt();
	double price = root["price"].asDouble();
	int TradeType = root["behavior"].asInt(); //0:买入1:卖出
	int TradeMode = 0;//root["orderMode"].asInt();
	std::string strCode = root["stockcode"].asString();
	int rt = -1;
	char *Result = NULL;
	std::string out;

	Json::Value new_item;
	new_item["acountComId"] = pInfo->_acountComId; 
	new_item["cashAcount"] = pInfo->_account.c_str();
	new_item["orderNumber"] = orderNo;

	rt = m_pTrade->SendOrder(TradeType, (char*)strCode.c_str(), price, num);
	new_item["ret"] = rt;
	if (rt == 1)
	{
		Result = m_pTrade->GetResult();
		printf("trade ok, Result=%s\n", Result);
		TdxRowset tr;
		tr.Update(Result);
		CString s = tr.GetID(0, 146);
		printf("s = %s\n", (LPSTR)(LPCTSTR)s);
		new_item["cno"] = (LPSTR)(LPCTSTR)s;
	}
	else{
		printf("trade failed, ErrInfo=%s\n", m_pTrade->GetError());
		pLog->PrintLog("trade failed, ErrInfo=%s\n",  m_pTrade->GetError());
		new_item["errInfo"] = G2U(m_pTrade->GetError());
	}

	// 输出无格式json字符串   
	Json::FastWriter writer;
	out = writer.write(new_item);
	printf("out=%s\n", out.c_str());
	pLog->PrintLog("InsertOrder callback, out=%s\n", out.c_str());
	tradeProcduct.SendData(out);
}

int TradeSystem::OrderDelete(Json::Value& root)
{
	Json::Value new_item;
	std::string out;
	Json::FastWriter writer;
	std::string stockcode = root["stockcode"].asString();
	std::string orderid = root["cno"].asString();
	std::string mks = "";
	char* Result = NULL;
	char* ErrInfo = NULL;
	int rt = 0;
	new_item["acountComId"] = pInfo->_acountComId; 
	new_item["cashAcount"] = pInfo->_account.c_str();
	new_item["cno"] = orderid;
	new_item["rsp"] = 0;

	pLog->PrintLog("OrderDelete, orderid=%s\n", orderid.c_str());
	rt = m_pTrade->CancelOrder((char*)stockcode.c_str(), (char*)orderid.c_str());
	new_item["ret"] = rt;
	if (rt == 1)
	{
		Result = m_pTrade->GetResult();
		printf("撤单, Result=%s\n", Result);
		TdxRowset tr;
		tr.Update(Result);
	}
	else
	{
		ErrInfo = m_pTrade->GetError();
		printf("撤单,ErrInfo = %s\n", ErrInfo);
		pLog->PrintLog("撤单,ErrInfo = %s\n", ErrInfo);
		new_item["errInfo"] = G2U(ErrInfo);
	}
	out = writer.write(new_item);
	printf("out=%s\n", out.c_str());
	pLog->PrintLog("OrderDelete callback, out=%s\n", out.c_str());
	deleteProcduct.SendData(out);

	return rt;
}

//交易接收解析
void TradeSystem::tradeParse(string& text, int funid)
{
	Json::Reader reader;  
	Json::Value root;  
	if (reader.parse(text.c_str(), root))  // reader将Json字符串解析到root，root将包含Json里所有子元素   
	{
		//提取下单指令参数
		int  functionID = root["func"].asInt(); 
		if (funid != 0)
		{
			functionID = funid;
		}
		printf("functionID = %d\n", functionID);
		pLog->PrintLog("%s, functionID=%d\n", __FUNCTION__, functionID);	 
		switch(functionID)
		{
		case eFunctionID_OrderInsert://委托下单 
			EnterCriticalSection(&cs);
			InsertOrder(root);
			LeaveCriticalSection(&cs);
			break;	
		case eFunctionID_OrderDelete://撤单
			EnterCriticalSection(&cs);
			OrderDelete(root);
			LeaveCriticalSection(&cs);
			break;
		case eFunctionID_MoneyQry://账户资金查询
			EnterCriticalSection(&cs);
			QryMoney(); //root
			LeaveCriticalSection(&cs);
			break;
		case eFunctionID_HoldQry://持仓查询
			EnterCriticalSection(&cs);
			QryHold();
			LeaveCriticalSection(&cs);
			break;
		default:
			break;
		}		
	}
	else
	{
		printf("json解析失败, text=%s\n", text.c_str());
		pLog->PrintLog("json解析失败, text=%s\n", text.c_str());
	}	
}

//发送成交主推回报
void TradeSystem::SendDeal(CString con, CString assets, CString marketValue, CString balance)
{
	Json::Value new_item;
	std::string out;
	Json::FastWriter writer;

	int rt = 0;
	new_item["acountComId"] = pInfo->_acountComId; 
	new_item["cashAcount"] = pInfo->_account.c_str();
	new_item["cno"] = (LPSTR)(LPCTSTR)con;
	new_item["successOrderNumber"] = (LPSTR)(LPCTSTR)assets;
	new_item["volume"] = (LPSTR)(LPCTSTR)marketValue;
	new_item["price"] = (LPSTR)(LPCTSTR)balance;

	//发送回报内容
	out = writer.write(new_item);
	printf("out=%s\n", out.c_str());
	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
	qrySuccessProcduct.SendData(out);	

}

//发送撤单主推回报
void TradeSystem::SendCancel(CString con, CString assets, CString marketValue,CString balance)
{
	Json::Value new_item;
	std::string out;
	Json::FastWriter writer;

	int rt = 0;
	new_item["acountComId"] = pInfo->_acountComId; 
	new_item["cashAcount"] = pInfo->_account.c_str();
	new_item["cno"] = (LPSTR)(LPCTSTR)con;
	new_item["volume"] = (LPSTR)(LPCTSTR)marketValue;
	new_item["rsp"]=1;
	new_item["ret"]=1;

	//发送回报内容
	out = writer.write(new_item);
	printf("out=%s\n", out.c_str());
	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
	deleteProcduct.SendData(out);
}

//CString con = tr.GetID(i, 146);         //合同编号
//CString assets = tr.GetID(i, 155);      //成交编号
//CString marketValue = tr.GetID(i, 152); //成交数量
//CString balance = tr.GetID(i, 153);     //成交价格
void TradeSystem::DoDeal(CString& con, CString& deal, CString& num, CString& price)
{
	printf("deal, con=%s, deal=%s, num=%s, price=%s\n", con, deal,  num,  price);
	if (0 == check[(LPSTR)(LPCTSTR)deal])
	{
		SendDeal(con, deal,  num,  price);
		check[(LPSTR)(LPCTSTR)deal] = 1;
	}
}

//CString con = tr.GetID(i, 146);         //合同编号				
//CString num = tr.GetID(i, 144);         //委托数量
//CString price = tr.GetID(i, 145);       //价格
//CString status = tr.GetID(i, 147);      //状态
//numDeal:委托成交数量
void TradeSystem::DoEntrust(CString& con, CString& num, CString& price, CString& status, CString& numDeal)
{
	char szBuf[256] = {0};
	printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
	if (status.Compare("废单") == 0 || status.Compare("已撤") == 0 || status.Compare("部撤") == 0)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			if (status.Compare("废单") == 0)
			{
				SendCancel(con, "",  num, price);
			}else if (status.Compare("已撤") == 0)
			{
				SendCancel(con, "",  num, price);
			}else if (status.Compare("部撤") == 0)
			{
				int numCancel = atoi(num) - atoi(numDeal);				
				sprintf(szBuf, "%d", numCancel);
				SendCancel(con, "",  szBuf, price);
			}
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}
}

void TradeSystem::DoEntrustGuangDa(CString& con, CString& num, CString& price, CString& status, CString& numDeal)
{
	char szBuf[256] = {0};
	printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
	if (status.Find("废单") != -1 || status.Find("已撤") != -1 || status.Find("部撤") != -1 || status.Find("撤单") != -1)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			if (status.Find("废单") != -1 || status.Find("已撤") != -1 || status.Find("撤单") != -1)
			{
				SendCancel(con, "",  num, price);
			}else if (status.Find("部撤") != -1)
			{//部分撤单
				int numCancel = atoi(num) - atoi(numDeal);				
				sprintf(szBuf, "%d", numCancel);
				SendCancel(con, "",  szBuf, price);
			}
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}

}

void TradeSystem::DoEntrustDongWu(CString& con, CString& num, CString& price, CString& status, CString& numDeal)
{
	printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
	if (status.Find("全部撤单") != -1 || status.Find("非法委托") != -1)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			SendCancel(con, "",  num, price);
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}else if (status.Find("部分撤单") != -1)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			char szBuf[256] = {0};
			int numCancel = atoi(num) - atoi(numDeal);				
			sprintf(szBuf, "%d", numCancel);
			SendCancel(con, "",  num, price);
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}
}

void TradeSystem::DoEntrustZhaoShang(CString& con, CString& num, CString& price, CString& status, CString& numDeal)
{
	printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
	if (status.Find("部分撤单") != -1)
	{//部分撤单
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			char szBuf[256] = {0};
			int numCancel = atoi(num) - atoi(numDeal);				
			sprintf(szBuf, "%d", numCancel);
			SendCancel(con, "",  num, price);
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}else if (status.Find("全部撤单") != -1)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			SendCancel(con, "",  num, price);
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}
}


//解析成交结果
void TradeSystem::parseDeal(char *Result)
{
	printf("当日成交列表, Result=%s\n", Result);
	TdxRowset tr;
	tr.Update(Result);
	for (int i = 0; i < tr.FRowCount; i++)
	{
		if (eTradeDeal_COMMON == SwitchDealType(pInfo->_acountComId))
		{//广发证券,财通证券
			CString con = tr.GetID(i, 146);         //合同编号
			CString deal = tr.GetID(i, 155);        //成交编号
			CString num = tr.GetID(i, 152);         //成交数量
			CString price = tr.GetID(i, 153);       //成交价格
			//CString status = tr.GetID(i, 147);    //状态
			DoDeal(con, deal, num, price);

		}else if (eTradeDeal_ONE == SwitchDealType(pInfo->_acountComId))
		{
			CString con = tr.GetID(i, 146);         //合同编号
			CString deal = tr.GetID(i, 155);        //成交编号
			CString num = tr.GetID(i, 152);         //成交数量
			CString price = tr.GetID(i, 153);       //成交价格
			CString buy = tr.GetID(i, 167);         //撤单标志 1：撤单, 0:买卖
			if (buy.Compare("0") == 0)
			{
				DoDeal(con, deal, num, price);
			}
		}else if (eTradeID_ZHAOSHANG == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //合同编号
			CString deal = tr.GetID(i, 155);        //成交编号
			CString num = tr.GetID(i, 152);         //成交数量
			CString price = tr.GetID(i, 153);       //成交价格
			CString buy = tr.GetID(i, 131);         //买卖标志
			if (buy.Compare("买入") == 0 || buy.Compare("卖出") == 0)
			{
				DoDeal(con, deal, num, price);
			}
		}else if (eTradeID_WUKUANG == pInfo->_acountComId)
		{//五矿
			CString con = tr.GetID(i, 146);         //合同编号
			CString assets = tr.GetID(i, 155);      //成交编号
			CString marketValue = tr.GetID(i, 152); //成交数量
			CString balance = tr.GetID(i, 153);     //成交价格
			CString status = tr.GetID(i, 147);      //状态
			printf("%s, %s, %s, %s, %s\n", con, assets, marketValue, balance, status);
			if (strcmp(assets, "") == 0)
			{//成交编号为空
			}
			else
			{//成交编号非空
				if (0 == check[(LPSTR)(LPCTSTR)assets])
				{
					if (status.Compare("普通成交") == 0)
					{
						SendDeal(con, assets,  marketValue, balance);
					}else if (status.Compare("内部撤单") == 0)
					{
						SendCancel(con, assets,  marketValue, balance);
					}else if (status.Compare("撤单成交") == 0)
					{
						SendCancel(con, assets,  marketValue, balance);
					}
					check[(LPSTR)(LPCTSTR)assets] = 1;
				}
			}
		}	
	}
}

//解析委托结果
void TradeSystem::parseEntrust(char *Result)
{
	TdxRowset tr;
	printf("当日委托列表, Result=%s\n", Result);			
	tr.Update(Result);

	for (int i = 0; i < tr.FRowCount; i++)
	{
		if (eTradeEntrust_COMMON == SwitchEntrustType(pInfo->_acountComId))
		{						
			CString num = tr.GetID(i, 144);         //委托数量
			CString price = tr.GetID(i, 145);       //价格
			CString con = tr.GetID(i, 146);         //合同编号	
			CString status = tr.GetID(i, 147);      //状态
			CString numDeal = tr.GetID(i, 152);     //成交数量
			DoEntrust(con,  num,  price, status, numDeal);
		
		}else if (eTradeID_GUOYUAN == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //合同编号				
			CString num = tr.GetID(i, 144);         //委托数量
			CString price = tr.GetID(i, 145);       //价格
			CString numDeal = tr.GetID(i, 152);     //成交数量
			CString status = tr.GetID(i, 163);      //状态
			DoEntrust(con,  num,  price, status, numDeal);
			
		}else if (eTradeID_GUANGDA == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //合同编号				
			CString num = tr.GetID(i, 144);         //委托数量
			CString price = tr.GetID(i, 145);       //价格
			CString numDeal = tr.GetID(i, 152);     //成交数量
			CString status = tr.GetID(i, 5552);      //状态
			DoEntrustGuangDa(con,  num,  price, status, numDeal);
		}
		else if (eTradeID_ZHONGXIN == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //合同编号				
			CString num = tr.GetID(i, 144);         //委托数量
			CString price = tr.GetID(i, 145);       //价格
			CString numDeal = tr.GetID(i, 152);     //成交数量
			CString status = tr.GetID(i, 5597);      //状态
			DoEntrust(con,  num,  price, status, numDeal);
		}else if (eTradeID_DONGWU == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //合同编号				
			CString num = tr.GetID(i, 144);         //委托数量
			CString price = tr.GetID(i, 145);       //价格
			CString numDeal = tr.GetID(i, 152);     //成交数量
			CString status = tr.GetID(i, 147);      //状态
			DoEntrustDongWu(con,  num,  price, status, numDeal);
		}else if (eTradeID_ZHAOSHANG == pInfo->_acountComId || eTradeID_ZHONGTIAN == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //合同编号				
			CString num = tr.GetID(i, 144);         //委托数量
			CString price = tr.GetID(i, 145);       //价格
			CString status = tr.GetID(i, 147);      //状态
			CString numDeal = tr.GetID(i, 152);     //成交数量
			DoEntrustZhaoShang(con,  num,  price, status, numDeal);
		}
	}
}

//查询当日成交
int TradeSystem::QryDeal()
{
	static int myCount = 0;
	static int errorNum = 0; 
	int rt = 0;
	char *Result = NULL;
	char *ErrInfo = NULL;

	rt = m_pTrade->QueryData(eQryType_Deal);
	if (rt == 1)
	{
		Result = m_pTrade->GetResult();
		errorNum = 0;
		if (myCount > 100000)
		{
			myCount=0;
		}
		iniFile->WriteIniInt_In("trade", "count", myCount++);
		parseDeal(Result);		
	}
	else
	{
		ErrInfo = m_pTrade->GetError();
		printf("ErrInfo = %s, errorNum=%d\n", ErrInfo, errorNum++);
		pLog->PrintLog("ErrInfo = %s, errorNum=%d\n", ErrInfo, errorNum);
		if (errorNum > 10)
		{
			pLog->PrintLog("--------stop app, num=%d, error=%s--------\n", errorNum, ErrInfo);
			exit(-2);
		}
	}

	return rt;
}

//查询当日委托列表
int TradeSystem::QryEntrust(){
	int rt = 0;

	rt = m_pTrade->QueryData(eQryType_Entrust);
	if (rt == 1)
	{
		parseEntrust(m_pTrade->GetResult());
	}
	else
	{
		printf("ErrInfo = %s\n", m_pTrade->GetError());
	}

	return rt;
}

void TradeSystem::checkDay()
{
	static int day = -1;
	SYSTEMTIME sys; 
	GetLocalTime(&sys); 
	//printf( "%4d/%02d/%02d %02d:%02d:%02d.%03d 星期%1d\n", sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,sys.wDayOfWeek);
	//隔天, 5点半, 清空
	if (day != sys.wDay && sys.wHour == 5 && sys.wMinute == 30)
	{
		check.clear();        //成交区别清空
		checkEntrust.clear(); //委托区别清空
		pLog->PrintLog("清空成交和委托记录, wDay=%d, wHour=%d, wMinute=%d\n", sys.wDay, sys.wHour, sys.wMinute);
		day = sys.wDay;
	}
}

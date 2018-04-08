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
extern CRITICAL_SECTION cs;//�����ٽ���ȫ�ֱ���

extern MqProducer tradeProcduct;      //����������
extern MqProducer deleteProcduct;     //����������
extern MqProducer qryMoneyProcduct;   //��ѯ�ʽ��˺�������
extern MqProducer qrySuccessProcduct; //��ѯ�ɽ���¼
extern MqProducer qryHoldProcduct;    //��ѯ�ֲ�
extern MqProducer qryHoldTopicProcduct;  //��ѯ�ֲ�����
extern MqProducer qryMoneyTopicProcduct; //��ѯ�ʽ�����

extern map<string, int> check;        //�ɽ�����
extern map<string, int> checkEntrust; //ί������

TradeSystem::TradeSystem(void)
{
	m_pTrade = NULL;
	m_bInit = false;
	m_bStart = false;
}

TradeSystem::~TradeSystem(void)
{
}


// ��ʼ��
bool TradeSystem::init(void)
{
	char szPath[1024] = {0};
	char szIni[1024] = {0};	

	//������־����
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

	//�õ�exeִ��·��.  
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
	pInfo->ReadIni(pPath);  //��ȡ�����ļ�
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

	//ֱ�Ӷ�jsoncpp����������������Ϊ�±���и�ֵ�����Զ���Ϊ����
	iNum[1] = 1;
	iNum[2] = 2;
	iNum[3] = 3;
	iNum[4] = 4;
	iNum[5] = 5;
	iNum[6] = 6;

	//���Ӷ�������
	root["array"]    = arrayObj;
	//�����ַ���
	root["name"]    = "json";
	//��������
	root["number"]    = 666;
	//���Ӳ�������
	root["value"]    = true;
	//������������
	root["arrnum"]    = iNum;

	root.toStyledString();
	string out = root.toStyledString();

	return out;
}

//��ѯ�ֲ��б�
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
		printf("�ֲֹɷ��б�, Result=%s\n", Result);
		TdxRowset tr;
		tr.Update(Result);
		for (int i = 0; i < tr.FRowCount; i++)
		{
			if (eTradeHold_COMMON == SwitchHoldType(pInfo->_acountComId))
			{
				//�ֲֽ���
				CString code = tr.GetID(i, 140);         //֤ȯ����
				CString name = tr.GetID(i, 141);         //֤ȯ����
				CString num = tr.GetID(i, 200);          //֤ȯ����
				CString costprice = tr.GetID(i, 202);    //�ɱ���
				CString currentprice = tr.GetID(i, 949); //��ǰ��
				CString balance = tr.GetID(i, 204);      //����ӯ��
				CString sellnum = tr.GetID(i, 201);      //��������

				if (eTradeID_WUKUANG == pInfo->_acountComId || eTradeID_CHANGCHENG == pInfo->_acountComId
					|| eTradeID_MINSHENG == pInfo->_acountComId)
				{//�������
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
		printf("�ֲ�, ErrInfo = %s\n", ErrInfo);
		pLog->PrintLog("�ֲ�, ErrInfo = %s\n", ErrInfo);
		root["errInfo"] = G2U(ErrInfo);
	}

	//���ͻر�����
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

//��ѯ�ʽ�
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
		printf("�ʽ��б�, Result=%s\n", Result);
		TdxRowset tr;
		tr.Update(Result);
		
		CString assets = tr.GetID(0, 310);      //���ʲ�
		CString marketValue = tr.GetID(0, 205); //������ֵ
		CString balance = tr.GetID(0, 301);     //�����ʽ�
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
		printf("�ʽ�, ErrInfo = %s\n", ErrInfo);
		pLog->PrintLog("�ʽ�, ErrInfo = %s\n", ErrInfo);
		new_item["errInfo"] = G2U(ErrInfo);
	}
	//���ͻر�����
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
	//��ȡ�µ�ָ�����
	std::string orderNo = root["orderNumber"].asString();
	int num = root["volume"].asInt();
	double price = root["price"].asDouble();
	int TradeType = root["behavior"].asInt(); //0:����1:����
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

	// ����޸�ʽjson�ַ���   
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
		printf("����, Result=%s\n", Result);
		TdxRowset tr;
		tr.Update(Result);
	}
	else
	{
		ErrInfo = m_pTrade->GetError();
		printf("����,ErrInfo = %s\n", ErrInfo);
		pLog->PrintLog("����,ErrInfo = %s\n", ErrInfo);
		new_item["errInfo"] = G2U(ErrInfo);
	}
	out = writer.write(new_item);
	printf("out=%s\n", out.c_str());
	pLog->PrintLog("OrderDelete callback, out=%s\n", out.c_str());
	deleteProcduct.SendData(out);

	return rt;
}

//���׽��ս���
void TradeSystem::tradeParse(string& text, int funid)
{
	Json::Reader reader;  
	Json::Value root;  
	if (reader.parse(text.c_str(), root))  // reader��Json�ַ���������root��root������Json��������Ԫ��   
	{
		//��ȡ�µ�ָ�����
		int  functionID = root["func"].asInt(); 
		if (funid != 0)
		{
			functionID = funid;
		}
		printf("functionID = %d\n", functionID);
		pLog->PrintLog("%s, functionID=%d\n", __FUNCTION__, functionID);	 
		switch(functionID)
		{
		case eFunctionID_OrderInsert://ί���µ� 
			EnterCriticalSection(&cs);
			InsertOrder(root);
			LeaveCriticalSection(&cs);
			break;	
		case eFunctionID_OrderDelete://����
			EnterCriticalSection(&cs);
			OrderDelete(root);
			LeaveCriticalSection(&cs);
			break;
		case eFunctionID_MoneyQry://�˻��ʽ��ѯ
			EnterCriticalSection(&cs);
			QryMoney(); //root
			LeaveCriticalSection(&cs);
			break;
		case eFunctionID_HoldQry://�ֲֲ�ѯ
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
		printf("json����ʧ��, text=%s\n", text.c_str());
		pLog->PrintLog("json����ʧ��, text=%s\n", text.c_str());
	}	
}

//���ͳɽ����ƻر�
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

	//���ͻر�����
	out = writer.write(new_item);
	printf("out=%s\n", out.c_str());
	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
	qrySuccessProcduct.SendData(out);	

}

//���ͳ������ƻر�
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

	//���ͻر�����
	out = writer.write(new_item);
	printf("out=%s\n", out.c_str());
	pLog->PrintLog("%s, out=%s", __FUNCTION__, out.c_str());
	deleteProcduct.SendData(out);
}

//CString con = tr.GetID(i, 146);         //��ͬ���
//CString assets = tr.GetID(i, 155);      //�ɽ����
//CString marketValue = tr.GetID(i, 152); //�ɽ�����
//CString balance = tr.GetID(i, 153);     //�ɽ��۸�
void TradeSystem::DoDeal(CString& con, CString& deal, CString& num, CString& price)
{
	printf("deal, con=%s, deal=%s, num=%s, price=%s\n", con, deal,  num,  price);
	if (0 == check[(LPSTR)(LPCTSTR)deal])
	{
		SendDeal(con, deal,  num,  price);
		check[(LPSTR)(LPCTSTR)deal] = 1;
	}
}

//CString con = tr.GetID(i, 146);         //��ͬ���				
//CString num = tr.GetID(i, 144);         //ί������
//CString price = tr.GetID(i, 145);       //�۸�
//CString status = tr.GetID(i, 147);      //״̬
//numDeal:ί�гɽ�����
void TradeSystem::DoEntrust(CString& con, CString& num, CString& price, CString& status, CString& numDeal)
{
	char szBuf[256] = {0};
	printf("con=%s, num=%s, price=%s, status=%s\n", con, num, price, status);
	if (status.Compare("�ϵ�") == 0 || status.Compare("�ѳ�") == 0 || status.Compare("����") == 0)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			if (status.Compare("�ϵ�") == 0)
			{
				SendCancel(con, "",  num, price);
			}else if (status.Compare("�ѳ�") == 0)
			{
				SendCancel(con, "",  num, price);
			}else if (status.Compare("����") == 0)
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
	if (status.Find("�ϵ�") != -1 || status.Find("�ѳ�") != -1 || status.Find("����") != -1 || status.Find("����") != -1)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			if (status.Find("�ϵ�") != -1 || status.Find("�ѳ�") != -1 || status.Find("����") != -1)
			{
				SendCancel(con, "",  num, price);
			}else if (status.Find("����") != -1)
			{//���ֳ���
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
	if (status.Find("ȫ������") != -1 || status.Find("�Ƿ�ί��") != -1)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			SendCancel(con, "",  num, price);
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}else if (status.Find("���ֳ���") != -1)
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
	if (status.Find("���ֳ���") != -1)
	{//���ֳ���
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			char szBuf[256] = {0};
			int numCancel = atoi(num) - atoi(numDeal);				
			sprintf(szBuf, "%d", numCancel);
			SendCancel(con, "",  num, price);
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}else if (status.Find("ȫ������") != -1)
	{
		if (0 == checkEntrust[(LPSTR)(LPCTSTR)con])
		{
			SendCancel(con, "",  num, price);
			checkEntrust[(LPSTR)(LPCTSTR)con] = 1;
		}
	}
}


//�����ɽ����
void TradeSystem::parseDeal(char *Result)
{
	printf("���ճɽ��б�, Result=%s\n", Result);
	TdxRowset tr;
	tr.Update(Result);
	for (int i = 0; i < tr.FRowCount; i++)
	{
		if (eTradeDeal_COMMON == SwitchDealType(pInfo->_acountComId))
		{//�㷢֤ȯ,��֤ͨȯ
			CString con = tr.GetID(i, 146);         //��ͬ���
			CString deal = tr.GetID(i, 155);        //�ɽ����
			CString num = tr.GetID(i, 152);         //�ɽ�����
			CString price = tr.GetID(i, 153);       //�ɽ��۸�
			//CString status = tr.GetID(i, 147);    //״̬
			DoDeal(con, deal, num, price);

		}else if (eTradeDeal_ONE == SwitchDealType(pInfo->_acountComId))
		{
			CString con = tr.GetID(i, 146);         //��ͬ���
			CString deal = tr.GetID(i, 155);        //�ɽ����
			CString num = tr.GetID(i, 152);         //�ɽ�����
			CString price = tr.GetID(i, 153);       //�ɽ��۸�
			CString buy = tr.GetID(i, 167);         //������־ 1������, 0:����
			if (buy.Compare("0") == 0)
			{
				DoDeal(con, deal, num, price);
			}
		}else if (eTradeID_ZHAOSHANG == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //��ͬ���
			CString deal = tr.GetID(i, 155);        //�ɽ����
			CString num = tr.GetID(i, 152);         //�ɽ�����
			CString price = tr.GetID(i, 153);       //�ɽ��۸�
			CString buy = tr.GetID(i, 131);         //������־
			if (buy.Compare("����") == 0 || buy.Compare("����") == 0)
			{
				DoDeal(con, deal, num, price);
			}
		}else if (eTradeID_WUKUANG == pInfo->_acountComId)
		{//���
			CString con = tr.GetID(i, 146);         //��ͬ���
			CString assets = tr.GetID(i, 155);      //�ɽ����
			CString marketValue = tr.GetID(i, 152); //�ɽ�����
			CString balance = tr.GetID(i, 153);     //�ɽ��۸�
			CString status = tr.GetID(i, 147);      //״̬
			printf("%s, %s, %s, %s, %s\n", con, assets, marketValue, balance, status);
			if (strcmp(assets, "") == 0)
			{//�ɽ����Ϊ��
			}
			else
			{//�ɽ���ŷǿ�
				if (0 == check[(LPSTR)(LPCTSTR)assets])
				{
					if (status.Compare("��ͨ�ɽ�") == 0)
					{
						SendDeal(con, assets,  marketValue, balance);
					}else if (status.Compare("�ڲ�����") == 0)
					{
						SendCancel(con, assets,  marketValue, balance);
					}else if (status.Compare("�����ɽ�") == 0)
					{
						SendCancel(con, assets,  marketValue, balance);
					}
					check[(LPSTR)(LPCTSTR)assets] = 1;
				}
			}
		}	
	}
}

//����ί�н��
void TradeSystem::parseEntrust(char *Result)
{
	TdxRowset tr;
	printf("����ί���б�, Result=%s\n", Result);			
	tr.Update(Result);

	for (int i = 0; i < tr.FRowCount; i++)
	{
		if (eTradeEntrust_COMMON == SwitchEntrustType(pInfo->_acountComId))
		{						
			CString num = tr.GetID(i, 144);         //ί������
			CString price = tr.GetID(i, 145);       //�۸�
			CString con = tr.GetID(i, 146);         //��ͬ���	
			CString status = tr.GetID(i, 147);      //״̬
			CString numDeal = tr.GetID(i, 152);     //�ɽ�����
			DoEntrust(con,  num,  price, status, numDeal);
		
		}else if (eTradeID_GUOYUAN == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //��ͬ���				
			CString num = tr.GetID(i, 144);         //ί������
			CString price = tr.GetID(i, 145);       //�۸�
			CString numDeal = tr.GetID(i, 152);     //�ɽ�����
			CString status = tr.GetID(i, 163);      //״̬
			DoEntrust(con,  num,  price, status, numDeal);
			
		}else if (eTradeID_GUANGDA == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //��ͬ���				
			CString num = tr.GetID(i, 144);         //ί������
			CString price = tr.GetID(i, 145);       //�۸�
			CString numDeal = tr.GetID(i, 152);     //�ɽ�����
			CString status = tr.GetID(i, 5552);      //״̬
			DoEntrustGuangDa(con,  num,  price, status, numDeal);
		}
		else if (eTradeID_ZHONGXIN == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //��ͬ���				
			CString num = tr.GetID(i, 144);         //ί������
			CString price = tr.GetID(i, 145);       //�۸�
			CString numDeal = tr.GetID(i, 152);     //�ɽ�����
			CString status = tr.GetID(i, 5597);      //״̬
			DoEntrust(con,  num,  price, status, numDeal);
		}else if (eTradeID_DONGWU == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //��ͬ���				
			CString num = tr.GetID(i, 144);         //ί������
			CString price = tr.GetID(i, 145);       //�۸�
			CString numDeal = tr.GetID(i, 152);     //�ɽ�����
			CString status = tr.GetID(i, 147);      //״̬
			DoEntrustDongWu(con,  num,  price, status, numDeal);
		}else if (eTradeID_ZHAOSHANG == pInfo->_acountComId || eTradeID_ZHONGTIAN == pInfo->_acountComId)
		{
			CString con = tr.GetID(i, 146);         //��ͬ���				
			CString num = tr.GetID(i, 144);         //ί������
			CString price = tr.GetID(i, 145);       //�۸�
			CString status = tr.GetID(i, 147);      //״̬
			CString numDeal = tr.GetID(i, 152);     //�ɽ�����
			DoEntrustZhaoShang(con,  num,  price, status, numDeal);
		}
	}
}

//��ѯ���ճɽ�
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

//��ѯ����ί���б�
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
	//printf( "%4d/%02d/%02d %02d:%02d:%02d.%03d ����%1d\n", sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds,sys.wDayOfWeek);
	//����, 5���, ���
	if (day != sys.wDay && sys.wHour == 5 && sys.wMinute == 30)
	{
		check.clear();        //�ɽ��������
		checkEntrust.clear(); //ί���������
		pLog->PrintLog("��ճɽ���ί�м�¼, wDay=%d, wHour=%d, wMinute=%d\n", sys.wDay, sys.wHour, sys.wMinute);
		day = sys.wDay;
	}
}

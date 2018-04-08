#include "StdAfx.h"
#include "trade.h"

#include "ILogEvent.h"
#include "Dir.h"
#include "IniFile.h"
#include "TdxRowset.h"

extern ILogEvent* pLog;

Trade::Trade(void)
{
	TdxInit=NULL; //����ָ��
	TdxExit=NULL;
	TdxLogin=NULL;
	TdxLogout=NULL;
	TdxQueryData=NULL;
	TdxSendOrder=NULL;
	TdxCancelOrder=NULL;
	TdxGetQuote=NULL;
	TdxQueryHistory=NULL;
	TdxSendOrderNew=NULL;
	TdxCancelOrderNew=NULL;
	GetSubmitUName=NULL;
	GetSubmitUID=NULL;
	GetDestName=NULL;
	GetBranchName=NULL;
	GetBranchID=NULL;
	GetClientConfig=NULL;
	GetOemQstag=NULL;
	GetServDesc=NULL;
	m_nClientID = -1;
	hDll = NULL;
	Result = NULL;
	m_bInit = false;
	memset(ErrInfo, 0, sizeof(ErrInfo));
	memset(m_szAccount, 0, sizeof(m_szAccount));
	Result = new char[1024 * 1024];
}

Trade::~Trade(void)
{
	if (Result)
	{
		delete[] Result;
		Result = NULL;
	}
}

//��ȡ������Ϣ
char* Trade::GetError()
{
	return ErrInfo;
}

//��ȡ���
char* Trade::GetResult()
{
	return Result;
}

//��ʼ��
int Trade::Init()
{
	hDll = LoadLibrary("TdxApi.Dll");
	printf("program hDll = %p\n", hDll);
	if (hDll == NULL)
	{
		printf("���׽ӿڼ���ʧ��\n");
		pLog->PrintLog("���׽ӿڼ���ʧ��\n");
		m_bInit = false;
		return -1;
	}

	TdxInit=(FTdxInit)GetProcAddress(hDll, "TdxInit");
	TdxExit=(FTdxExit)GetProcAddress(hDll, "TdxExit");
	TdxLogin=(FTdxLogin)GetProcAddress(hDll, "TdxLogin");
	TdxLogout=(FTdxLogout)GetProcAddress(hDll, "TdxLogout");
	TdxQueryData=(FTdxQueryData)GetProcAddress(hDll, "TdxQueryData");
	TdxSendOrder=(FTdxSendOrder)GetProcAddress(hDll, "TdxSendOrder");
	TdxCancelOrder=(FTdxCancelOrder)GetProcAddress(hDll, "TdxCancelOrder");
	TdxGetQuote=(FTdxGetQuote)GetProcAddress(hDll, "TdxGetQuote");
	TdxQueryHistory=(FTdxQueryHistory)GetProcAddress(hDll, "TdxQueryHistory");
	TdxSendOrderNew=(FTdxSendOrderNew)GetProcAddress(hDll, "TdxSendOrderNew");
	TdxCancelOrderNew=(FTdxCancelOrderNew)GetProcAddress(hDll, "TdxCancelOrderNew");
	GetSubmitUName=(FGetSubmitUName)GetProcAddress(hDll, "GetSubmitUName");
	GetSubmitUID=(FGetSubmitUID)GetProcAddress(hDll, "GetSubmitUID");
	GetDestName=(FGetDestName)GetProcAddress(hDll, "GetDestName");
	GetBranchName=(FGetBranchName)GetProcAddress(hDll, "GetBranchName");
	GetBranchID=(FGetBranchID)GetProcAddress(hDll, "GetBranchID");
	GetClientConfig=(FGetClientConfig)GetProcAddress(hDll, "GetClientConfig");
	GetOemQstag=(FGetOemQstag)GetProcAddress(hDll, "GetOemQstag");
	GetServDesc=(FGetServDesc)GetProcAddress(hDll, "GetServDesc");
	int rt = TdxInit();
	if (-1 == rt){
		m_bInit = false;
		printf("init failed\n");
	}else{
		printf("init ok, ret = %d\n", rt);
		m_bInit = true;
	}
	return rt;
}

//����
int Trade::Login(char *server,int port,char *Version,int AccountType,int ID,char *Account,char *JyPass,char *TxPass)
{
	if (!m_bInit)
	{
		return -1;
	}
	printf("login server=%s, port=%d, Version=%s, AccountType=%d, ID=%d, Account=%s, JyPass=%s, TxPass=%s\n", 
		server, port, Version, AccountType, ID ,Account, JyPass, TxPass);
	pLog->PrintLog("login server=%s, port=%d, Version=%s, AccountType=%d, ID=%d, Account=%s, JyPass=%s, TxPass=%s\n", 
		server, port, Version, AccountType, ID ,Account, JyPass, TxPass);
	m_nClientID = TdxLogin(server, port, Version, AccountType, ID ,Account, JyPass, TxPass, ErrInfo);
	if (m_nClientID == -1)
	{ 
		printf("��¼ʧ��, ErrInfo=%s\n", ErrInfo);
		pLog->PrintLog("��¼ʧ��, server=%s, port=%d, account=%s,ErrInfo=%s\n", server, port, Account, ErrInfo);
		return -1;
	}else{
		strcpy(m_szAccount, GetSubmitUID(m_nClientID));
		if (1 == QueryShareHolderInfo())
		{
			pLog->PrintLog("%s:%d,%s, ����ɹ�\n", server, port, Account);
			printf("����ɹ�\n");
		}
		else
		{
			printf("����ʧ��\n");
			return -1;
		}

	}
	return 0;
}

//��ѯ�ɶ���Ϣ
int Trade::QueryShareHolderInfo()
{
	//��ѯ�ɶ���Ϣ
	int rt = TdxQueryData(m_nClientID, m_szAccount, 5, Result, ErrInfo);
	if (rt == 1)
	{
		printf("Result = %s\n", Result);
		TdxRowset tr;
		tr.Update(Result);	
		CString Ac = m_szAccount;
		for (int i = 0; i < tr.FRowCount; i++)
		{
			CString s = tr.GetID(i, 121); //�ʽ��˺�
			CString s1 = tr.GetID(i, 281);//���ʱ�־
			CString s2 = tr.GetID(i, 125);//�г����  0����A 1�Ϻ�A 2=����B 3=�Ϻ�B 12=��ת
			CString s3 = tr.GetID(i, 123);//�ɶ�����

			if (s1 == "1")
			{
				if (RAccount == "")
				{
					RAccount = s;
					if (s == "0")
						RAccount =Ac;
				}
				if (s2 == "0")
					RSZAID = s3;
				if (s2 == "1")
					RSHAID = s3;
				if (s != "0")
					continue;
			}
			if (GAccount == "")
			{
				GAccount = s;
				if ((s == "0") || (s == ""))
					GAccount = Ac;
			}
			if ((s2 == "0") && (GSZAID == ""))
				GSZAID = s3;
			if ((s2 == "1") && (GSHAID == ""))
				GSHAID = s3;
		}

		if (RAccount == "")
			RAccount = GAccount;
		if (RSHAID == "")
			RSHAID = GSHAID;
		if (RSZAID == "")
			RSZAID = GSZAID;
	}
	else
	{
		printf("ȡ�ɶ���Ϣʧ��,ErrInfo = %s\n", ErrInfo);
		pLog->PrintLog("ȡ�ɶ���Ϣʧ��,ErrInfo = %s\n", ErrInfo);
	}
	return rt;

}

//�ǳ�
int Trade::Logout()
{
	return 0;

}

//�˳�
int Trade::Exit()
{
	return 0;

}

//��ѯ
int Trade::QueryData(int type)
{
	int rt = 0;
	if (-1 == m_nClientID)
	{
		return -1;
	}
	rt = TdxQueryData(m_nClientID, m_szAccount, type, Result, ErrInfo); //��ѯ
	return rt;
}

//�µ�
int Trade::SendOrder(int TradeType,char *StkCode,double Price,int volume)
{
	int rt = 0;

	if (strlen(StkCode) != 6){
		pLog->PrintLog("��Ʊ�������, ������λ, StkCode=%s\n ", StkCode);
		return -1;
	}

	CString gdcode=GSZAID;
	if (StkCode[0]=='6')
		gdcode=GSHAID;
	pLog->PrintLog("Trade::SendOrder,ClientID=%d, Account=%s, TradeType=%d, gdcode=%s, StkCode=%s, price=%f, volume=%d\n", m_nClientID, m_szAccount, TradeType, (LPSTR)(LPCTSTR)gdcode, StkCode, Price, volume);
	rt = TdxSendOrderNew(m_nClientID, m_szAccount, TradeType, 0, (LPSTR)(LPCTSTR)gdcode, StkCode, Price, volume, Result, ErrInfo);
	return rt;
}

//����
int Trade::CancelOrder(char* stockcode,char* orderid)
{
	return TdxCancelOrderNew(m_nClientID, m_szAccount, stockcode, orderid, "", Result, ErrInfo);
}
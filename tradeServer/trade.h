#pragma once

#include <afx.h>
#include "ITrade.h"
#include "tradeFun.h"

class Trade :public ITrade
{
public:
	Trade(void);
	~Trade(void);
public:
	//��ʼ��
	virtual int Init();
	//����
	virtual int Login(char *server,int port,char *Version,int AccountType,int ID,char *Account,char *JyPass,char *TxPass);
	//�ǳ�
	virtual int Logout();
	//�˳�
	virtual int Exit();
	//��ѯ
	virtual int QueryData(int type);
	//�µ�
	virtual int SendOrder(int TradeType,char *StkCode,double Price,int volume);
	//����
	virtual int CancelOrder(char* stockcode,char* orderid);
	//��ȡ������Ϣ
	virtual char* GetError();
	//��ȡ���
	virtual char* GetResult();

	//SendOrder(int TradeType,char *StkCode,double Price,int volume)

protected:
	//��ѯ�ɶ���Ϣ
	int QueryShareHolderInfo();
private:
	bool m_bInit;
	int m_nClientID; //�ͻ��˾��
    //����ָ��
	FTdxInit TdxInit; 
	FTdxExit TdxExit;
	FTdxLogin TdxLogin;
	FTdxLogout   TdxLogout;
	FTdxQueryData TdxQueryData;
	FTdxSendOrder TdxSendOrder;
	FTdxCancelOrder	TdxCancelOrder;
	FTdxGetQuote TdxGetQuote;
	FTdxQueryHistory TdxQueryHistory;
	FTdxSendOrderNew TdxSendOrderNew;
	FTdxCancelOrderNew	TdxCancelOrderNew;
	FGetSubmitUName GetSubmitUName;
	FGetSubmitUID GetSubmitUID;
	FGetDestName GetDestName;
	FGetBranchName GetBranchName;
	FGetBranchID GetBranchID;
	FGetClientConfig GetClientConfig;
	FGetOemQstag GetOemQstag;
	FGetServDesc GetServDesc;
	HINSTANCE hDll;
	char ErrInfo[256];
	char *Result;
	char m_szAccount[100];


	CString GAccount ;//��Ʊ�˺�
	CString GSZAID ;//��A 0
	CString GSHAID ;//��A 1
	CString GSZBID ;//  B 2
	CString GSHBID ;//  B 3
	CString FGZID ;//  B 12

	CString RAccount ;//�����˺�
	CString RSZAID ;//��A 0 �����˺�
	CString RSHAID ;//��A 1
	CString RSZBID ;//  B 2
	CString RSHBID ;//  B 3
};

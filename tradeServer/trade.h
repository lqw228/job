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
	//初始化
	virtual int Init();
	//登入
	virtual int Login(char *server,int port,char *Version,int AccountType,int ID,char *Account,char *JyPass,char *TxPass);
	//登出
	virtual int Logout();
	//退出
	virtual int Exit();
	//查询
	virtual int QueryData(int type);
	//下单
	virtual int SendOrder(int TradeType,char *StkCode,double Price,int volume);
	//撤单
	virtual int CancelOrder(char* stockcode,char* orderid);
	//获取错误信息
	virtual char* GetError();
	//获取结果
	virtual char* GetResult();

	//SendOrder(int TradeType,char *StkCode,double Price,int volume)

protected:
	//查询股东信息
	int QueryShareHolderInfo();
private:
	bool m_bInit;
	int m_nClientID; //客户端句柄
    //函数指针
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


	CString GAccount ;//股票账号
	CString GSZAID ;//深A 0
	CString GSHAID ;//沪A 1
	CString GSZBID ;//  B 2
	CString GSHBID ;//  B 3
	CString FGZID ;//  B 12

	CString RAccount ;//融资账号
	CString RSZAID ;//深A 0 融资账号
	CString RSHAID ;//沪A 1
	CString RSZBID ;//  B 2
	CString RSHBID ;//  B 3
};

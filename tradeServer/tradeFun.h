#ifndef __TRADEFUN_H__  
#define __TRADEFUN_H__  

typedef int Tc_Engine;
typedef int Tc_Client;
typedef int Tc_ParSet;
typedef int Tc_RowSet;
typedef int Tc_Job;

typedef int   (__cdecl *FTdxInit)();
typedef int   (__cdecl *FTdxExit)();
typedef int   (__cdecl *FTdxLogin)(char *server,int port,char *Version,int AccountType,int ID,char *Account,char *JyPass,char *TxPass,char *ErrInfo);
typedef int   (__cdecl *FTdxLogout)(int ClientID);
typedef int   (__cdecl *FTdxQueryData)(int ClientID,char *Account,int QueryMode,char *Result,char *ErrInfo);
typedef int   (__cdecl *FTdxSendOrder)(int ClientID,int TradeType,int TradeMode,char *GdCode,char *StkCode,double Price,int volume,char *Result,char *ErrInfo);
typedef int   (__cdecl *FTdxCancelOrder)(int ClientID,char *StockCode,char *TradeID,char *Mark,char *Result,char *ErrInfo);
typedef int   (__cdecl *FTdxGetQuote)(int ClientID,char* StockCode, char* Result, char* ErrInfo);
typedef int   (__cdecl *FTdxQueryHistory)(int ClientID,char *Account,int Category, char* StartDate, char* EndDate, char* Result, char* ErrInfo);
typedef int   (__cdecl *FTdxSendOrderNew)(int ClientID,char* Acount,int TradeType,int TradeMode,char *GdCode,char *StkCode,double Price,int volume,char *Result,char *ErrInfo);
typedef int   (__cdecl *FTdxCancelOrderNew)(int ClientID,char* Acount,char *StockCode,char *TradeID,char *Mark,char *Result,char *ErrInfo);

typedef char* (__cdecl *FGetSubmitUName)(Tc_Client Client);
typedef char* (__cdecl *FGetSubmitUID)(Tc_Client Client);
typedef char* (__cdecl *FGetDestName)(Tc_Client Client);
typedef char* (__cdecl *FGetBranchName)(Tc_Client Client);
typedef int   (__cdecl *FGetBranchID)(Tc_Client Client);
typedef char* (__cdecl *FGetClientConfig)(Tc_Client Client);
typedef char* (__cdecl *FGetOemQstag)(Tc_Client Client);
typedef char* (__cdecl *FGetServDesc)(Tc_Client Client);
  
#endif  
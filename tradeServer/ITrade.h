#pragma once

class ITrade
{
public:
	ITrade(void){};
	~ITrade(void){};

	//初始化
	virtual int Init() = 0;
	//登入
	virtual int Login(char *server,int port,char *Version,int AccountType,int ID,char *Account,char *JyPass,char *TxPass) = 0;
	//查询
	virtual int QueryData(int type) = 0;
	//下单
	virtual int SendOrder(int TradeType,char *StkCode,double Price,int volume) = 0;
	//撤单
	virtual int CancelOrder(char* stockcode,char* orderid) =0;
	//登出
	virtual int Logout() = 0;
	//退出
	virtual int Exit() = 0;
    //获取错误信息
	virtual char* GetError() = 0;
	//获取结果
	virtual char* GetResult() = 0;
};

#pragma once

#include "ITrade.h"
#include <afx.h>

#include <json/json.h>
#include <algorithm>
#include <sstream>

class TradeSystem
{
public:
	TradeSystem(void);
	~TradeSystem(void);

	//初始化
	bool init(void);
    //退出
	bool Uninit(void);
    //启动
	int start(void);
    //停止
	int stop(void);

	//交易接收解析
	void tradeParse(string& text, int funid);
	//查询当日成交
	int QryDeal();
	//查询当日委托
	int QryEntrust();
	//查询持仓列表
	void QryHold(bool bQueue = true);
    //隔日清除保存记录
	void checkDay();
    //解析成交结果
	void parseDeal(char *Result);
	//解析委托结果
	void parseEntrust(char *Result);
	//查询资金
	void QryMoney(bool bQueue = true);
	
protected:
	//广大证券委托处理
	void DoEntrustGuangDa(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
    //处理通用委托
	void DoEntrust(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
    //处理通用成交
	void DoDeal(CString& con, CString& deal, CString& num, CString& price);

	//下单
	void InsertOrder(Json::Value& root);
	//撤单
	int OrderDelete(Json::Value& root);
	//发送成交主推回报
	void SendDeal(CString con,CString assets, CString marketValue,CString balance);
	//发送撤单主推回报
	void SendCancel(CString con,CString assets, CString marketValue,CString balance);

	//东吴委托撤单方式
	void DoEntrustDongWu(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
	//招商撤单
	void DoEntrustZhaoShang(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
private:
	ITrade* m_pTrade;
	bool m_bInit;
	bool m_bStart;
};

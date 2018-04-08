#pragma once

// 请求
enum eFunctionID
{

	eFunctionID_OrderInsert = 0,          // 报单请求
	eFunctionID_OrderDelete = 1,          // 报单撤销
	eFunctionID_MoneyQry    = 2,          // 账户资金查询
	eFunctionID_HoldQry     = 3,          //持仓查询
	eFunctionID_OrderQry,             // 报单查询
	eFunctionID_OrderStateAutoPush,   // 报单状态主推
	eFunctionID_OrderInfoAutoPush,    // 报单信息主推
	eFunctionID_MatchQry,             // 成交查询
	eFunctionID_MatchStateAutoPush,   // 成交状态主推
	eFunctionID_MatchInfoAutoPush,    // 成交信息主推
	
	eFunctionID_MoneyChgAutoPush,     // 账户资金变化主推
	eFunctionID_OrderModity,          // 报单改单
	//eFunctionID_Begin,
	eFunctionID_Login,            // 登录
	eFunctionID_MoneyQry_End,         // 账户资金查询结束
	eFunctionID_OrderQry_End,	

	eFunctionID_End = 99
};

//查询类型, 0资金",1"持仓股份",2"当日委托",3"当日成交",4"撤单列表",5"股东代码"
enum 
{
	eQryType_Fund = 0,
	eQryType_HoldPosition = 1,	
	eQryType_Entrust = 2,
	eQryType_Deal = 3,
	eQryType_Cancel = 4,
	eQryType_Stockholder = 5,
};

//券商机构ID
enum
{
	eTradeID_WUKUANG = 1, //五矿证券
	eTradeID_GUANGFA = 4, //广发证券
	eTradeID_CAITONG = 5, //财通证券
	eTradeID_DONGBEI = 8, //东北证券
	eTradeID_FANGZHENG = 9, //方正证券
	eTradeID_PINGAN  = 106, //平安证券 (资金余额解析特殊处理)	
	eTradeID_GUOYUAN = 108, //国元证券
	eTradeID_HENGTAI = 109, //恒泰证券
	eTradeID_CHANGCHENG = 110, //长城证券
	eTradeID_CUOTAIJUNAN = 111, //国泰君安
	eTradeID_DONGWU = 112, //东吴证券
	eTradeID_ZHAOSHANG = 113, //招商证券
	eTradeID_DIYICHUANGYE = 114, //第一创业证券
	eTradeID_GUANGDA = 115, //光大证券
	eTradeID_DONGWAN = 116, //东莞证券
	eTradeID_ZHONGYUAN = 117, //中原证券
	eTradeID_XIANGCAI = 118, //湘财证券
	eTradeID_ZHONGXIN = 119, //中信证券
	eTradeID_XINGYE = 120, //兴业证券
	eTradeID_HUAFU = 121, //华福证券
	eTradeID_MINSHENG = 122, //民生证券
	eTradeID_ZHONGTIAN = 123, //中天证券
	eTradeID_XINDA = 124,     //信达证券 (资金余额无数据)
	
};

enum
{
	eTradeDeal_OTHER = -1,
	eTradeDeal_COMMON = 0,//成交通用处理
	eTradeDeal_ONE = 1,   //成交记录中包含撤单记录
};

enum
{
	eTradeEntrust_OTHER = -1,
	eTradeEntrust_COMMON = 0,//委托通用处理
};

enum
{
	eTradeHold_OTHER = -1,
	eTradeHold_COMMON = 0,//持仓通用处理
};


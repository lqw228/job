#include "StdAfx.h"
#include "Func.h"
#include <afx.h>
#include "gobal.h"

//GB2312到UTF-8的转换
char* G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return str;
}

int String2Array(const CString& s, CStringArray &sa, char chSplitter)
{
	int nLen=s.GetLength(), nLastPos, nPos;
	bool bContinue;

	sa.RemoveAll();
	nLastPos=0;
	do
	{
		bContinue=false;	
		nPos = s.Find(chSplitter, nLastPos);
		if (-1!=nPos)
		{
			sa.Add(s.Mid(nLastPos, nPos-nLastPos));
			nLastPos=nPos+1;
			if (nLastPos != nLen) bContinue=true;
		}
	} while (bContinue);

	if (nLastPos != nLen)
		sa.Add(s.Mid(nLastPos, nLen-nLastPos));

	return (int)sa.GetSize();
}

void ParsePingAnAsset(char* Result, CString& str310, CString& str205, CString& str301)
{
	CStringArray sa;
	CStringArray sa1;
	String2Array(Result, sa, '\n');
	CString str1 = sa.GetAt(2);
	int cs=String2Array(str1, sa1,'\t');
	//CString str2 = sa1.GetAt(0);
	str310 = sa1.GetAt(8);//310_总资产
	str205 = sa1.GetAt(7);//205_最新市值
	str301 = sa1.GetAt(3);//301_可用资金

	printf("str205=%s, str301=%s, str310=%s\n", str205, str301, str310);
}
//eTradeID_DIYICHUANGYE == pInfo->_acountComId || eTradeID_XIANGCAI == pInfo->_acountComId
//|| eTradeID_MINSHENG == pInfo->_acountComId
//判断成交结果
int SwitchDealType(int type)
{
	int ret = eTradeDeal_OTHER;
	switch (type)
	{
	case eTradeID_GUANGFA:
	case eTradeID_CAITONG:
	case eTradeID_PINGAN:
	case eTradeID_DONGBEI:
	case eTradeID_GUOYUAN:
	case eTradeID_HENGTAI:
	case eTradeID_CUOTAIJUNAN:
	case eTradeID_DONGWU:
	case eTradeID_GUANGDA:
	case eTradeID_DONGWAN:
	case eTradeID_ZHONGYUAN:	
	case eTradeID_ZHONGXIN:
	case eTradeID_XINGYE:
	case eTradeID_HUAFU:	
	case eTradeID_ZHONGTIAN:
	case eTradeID_FANGZHENG:
		 ret = eTradeDeal_COMMON;
		break;
	case eTradeID_CHANGCHENG:
	case eTradeID_DIYICHUANGYE:
	case eTradeID_XIANGCAI:
	case eTradeID_MINSHENG:
		ret = eTradeDeal_ONE;
		break;
	}
	return ret;
}

//判断委托结果
int SwitchEntrustType(int type)
{
	int ret = eTradeEntrust_OTHER;
	switch (type)
	{
	case eTradeID_GUANGFA:
	case eTradeID_CAITONG:
	case eTradeID_DONGBEI:
	case eTradeID_HENGTAI:
	case eTradeID_CUOTAIJUNAN:
	case eTradeID_WUKUANG:
	case eTradeID_DIYICHUANGYE:
	case eTradeID_DONGWAN:
	case eTradeID_ZHONGYUAN:
	case eTradeID_XIANGCAI:
	case eTradeID_XINGYE:
	case eTradeID_HUAFU:
	case eTradeID_MINSHENG:
	case eTradeID_CHANGCHENG:
	case eTradeID_FANGZHENG:
		ret = eTradeEntrust_COMMON;
		break;
	}
	return ret;
}

//判断持仓结果
int SwitchHoldType(int type)
{
	int ret = eTradeHold_OTHER;
	//switch (type)
	//{
	//case eTradeID_GUANGFA:
	//case eTradeID_CAITONG:
	//case eTradeID_FANGZHENG:
	//case eTradeID_WUKUANG:
	//case eTradeID_CHANGCHENG:
	//	ret = eTradeHold_COMMON;
	//	break;
	//}
	ret = eTradeHold_COMMON;
	return ret;
}
#pragma once
#include <afx.h>
//GB2312到UTF-8的转换
extern char* G2U(const char* gb2312);
extern int SwitchDealType(int type);
extern int SwitchEntrustType(int type);
extern int SwitchHoldType(int type); //判断持仓结果
//extern void ParsePingAnAsset(char* Result);
extern int String2Array(const CString& s, CStringArray& sa, char chSplitter);
extern void ParsePingAnAsset(char* Result, CString& str310, CString& str205, CString& str301);

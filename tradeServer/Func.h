#pragma once
#include <afx.h>
//GB2312��UTF-8��ת��
extern char* G2U(const char* gb2312);
extern int SwitchDealType(int type);
extern int SwitchEntrustType(int type);
extern int SwitchHoldType(int type); //�жϳֲֽ��
//extern void ParsePingAnAsset(char* Result);
extern int String2Array(const CString& s, CStringArray& sa, char chSplitter);
extern void ParsePingAnAsset(char* Result, CString& str310, CString& str205, CString& str301);

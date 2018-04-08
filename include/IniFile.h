// IniFile.h: interface for the CIniFile class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include <atlstr.h>

#ifndef DLL_INI_API
#define DLL_INI_API _declspec(dllimport)
#endif
class DLL_INI_API CIniFile  
{
public:
	CIniFile();
	virtual ~CIniFile();

	//设置配置文件名
	void SetFileName(const char* pName);
    //写配置文件Int数据
	void WriteIniInt_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,int nValue);	 
	//写配置文件String数据
	void WriteIniString_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,LPCTSTR lpcszValue);	 
    //读配置文件int数据
	int ReadIniInt_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,int nDefValue); 
	//读配置文件string数据
	void ReadIniString_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,LPCTSTR lpcszDefValue,TCHAR* lpValue,int nMax); 
private:
	char m_szFileName[1024];
};


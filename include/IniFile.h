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

	//���������ļ���
	void SetFileName(const char* pName);
    //д�����ļ�Int����
	void WriteIniInt_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,int nValue);	 
	//д�����ļ�String����
	void WriteIniString_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,LPCTSTR lpcszValue);	 
    //�������ļ�int����
	int ReadIniInt_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,int nDefValue); 
	//�������ļ�string����
	void ReadIniString_In(LPCTSTR lpcszSec,LPCTSTR lpcszKey,LPCTSTR lpcszDefValue,TCHAR* lpValue,int nMax); 
private:
	char m_szFileName[1024];
};


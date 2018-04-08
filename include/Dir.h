#pragma once
/**************************************************************************
* @file Dir.h
* @Copyright (C) uhope Corporation
* @All rights reserved.
* @brief Ŀ¼ģ��
* @author liuqingwu
* @date 2012-8-1 9:39:55
* @version 1.0.0.3
**************************************************************************/

#ifndef DLL_DIR_API
#define DLL_DIR_API extern "C" _declspec(dllimport)
#endif
/************************************************************************** 
* @brief CreateSingleDir 
* 
* ���������ļ���.
* @param[in] pDir �ļ���·��
**************************************************************************/
DLL_DIR_API bool CreateSingleDir(const char* pDir);

/************************************************************************** 
* @brief CreateMultipleDirectory 
* 
* �������ļ���.
* @param[in] szPath 
* @return DLL_DIR_API bool  
**************************************************************************/
DLL_DIR_API bool CreateMultipleDirectory(const char* szPath);

/************************************************************************** 
* @brief GetAppPath 
* 
* ��ȡӦ�ó���·��.
* @param[out] strAppPath 
**************************************************************************/
DLL_DIR_API void GetAppPath(char* pAppPath);

/************************************************************************** 
* @brief FileExist 
* 
* �ж��ļ��Ƿ����.
* @param[in] pFilePath 
* @return bool  true:�ļ����ڣ� false: �ļ�������
**************************************************************************/
DLL_DIR_API bool FileExist(const char* pFilePath);

/************************************************************************** 
* @brief Log 
* 
* ��־������.
* @param[in] format 
* @param[in] ... 
* @return int  0: �ɹ��� ������ʧ��
**************************************************************************/
DLL_DIR_API int Log(char * format, ...);
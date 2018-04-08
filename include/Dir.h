#pragma once
/**************************************************************************
* @file Dir.h
* @Copyright (C) uhope Corporation
* @All rights reserved.
* @brief 目录模块
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
* 创建单层文件夹.
* @param[in] pDir 文件夹路径
**************************************************************************/
DLL_DIR_API bool CreateSingleDir(const char* pDir);

/************************************************************************** 
* @brief CreateMultipleDirectory 
* 
* 创建多文件夹.
* @param[in] szPath 
* @return DLL_DIR_API bool  
**************************************************************************/
DLL_DIR_API bool CreateMultipleDirectory(const char* szPath);

/************************************************************************** 
* @brief GetAppPath 
* 
* 获取应用程序路径.
* @param[out] strAppPath 
**************************************************************************/
DLL_DIR_API void GetAppPath(char* pAppPath);

/************************************************************************** 
* @brief FileExist 
* 
* 判断文件是否存在.
* @param[in] pFilePath 
* @return bool  true:文件存在， false: 文件不存在
**************************************************************************/
DLL_DIR_API bool FileExist(const char* pFilePath);

/************************************************************************** 
* @brief Log 
* 
* 日志处理函数.
* @param[in] format 
* @param[in] ... 
* @return int  0: 成功， 其他：失败
**************************************************************************/
DLL_DIR_API int Log(char * format, ...);
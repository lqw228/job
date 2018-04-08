#pragma once
#ifndef DLL_XLOGEVENT_API
#define DLL_XLOGEVENT_API _declspec(dllimport)
#endif
typedef enum tagLogLevel
{
	LOG_NORMAL = 0,
	LOG_ERROR  = 1,
	LOG_WARN   = 2,
	LOG_DEBUG  = 3,

}LOG_LEVEL;

class DLL_XLOGEVENT_API ILogEvent
{
public:
	ILogEvent(void){};
	virtual ~ILogEvent(void){};
	virtual int SetLogFileName(const char* pFileName) = 0;  //设置日志文件名
	virtual int GetLogFileName(char* pOutFileName) = 0;     //获取日志文件名
	virtual int SetDir(const char* pDir) = 0;               //设置日志目录
	virtual int GetDir(char* pOutDir) = 0;                  //获取日志目录
	virtual int SetLevel(LOG_LEVEL level) = 0;              //设置日志级别
	virtual LOG_LEVEL GetLevel() = 0;                       //获取日志级别
	virtual int PrintLog(char* szFmt, ...) = 0;                  //打印日志
	virtual int PrintLog(LOG_LEVEL level, char* szFmt, ...) = 0; //打印日志
	virtual int GetLastError() = 0;                              //获取日志错误信息
};

DLL_XLOGEVENT_API ILogEvent*  CreateLogformObj();
DLL_XLOGEVENT_API int    ReleaseLogformObj(ILogEvent* pThis);

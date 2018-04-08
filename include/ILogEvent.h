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
	virtual int SetLogFileName(const char* pFileName) = 0;  //������־�ļ���
	virtual int GetLogFileName(char* pOutFileName) = 0;     //��ȡ��־�ļ���
	virtual int SetDir(const char* pDir) = 0;               //������־Ŀ¼
	virtual int GetDir(char* pOutDir) = 0;                  //��ȡ��־Ŀ¼
	virtual int SetLevel(LOG_LEVEL level) = 0;              //������־����
	virtual LOG_LEVEL GetLevel() = 0;                       //��ȡ��־����
	virtual int PrintLog(char* szFmt, ...) = 0;                  //��ӡ��־
	virtual int PrintLog(LOG_LEVEL level, char* szFmt, ...) = 0; //��ӡ��־
	virtual int GetLastError() = 0;                              //��ȡ��־������Ϣ
};

DLL_XLOGEVENT_API ILogEvent*  CreateLogformObj();
DLL_XLOGEVENT_API int    ReleaseLogformObj(ILogEvent* pThis);

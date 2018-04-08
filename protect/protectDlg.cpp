// protectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "protect.h"
#include "protectDlg.h"
#include "Tlhelp32.h"
#include "Dir.h"
#include "ILogEvent.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ILogEvent* pLog = NULL;
CIniFile* iniFile = NULL;
CIniFile* iniKeep = NULL;

char szPath[1024] = { 0 };
char szApp[512]   = {0};
char szIni[1024]  = {0};

//股市有效时间 9点到16点
bool checkIsWork()
{
	SYSTEMTIME sys; 
	GetLocalTime( &sys ); 
	if (sys.wDayOfWeek >= 1 && sys.wDayOfWeek <= 5 && sys.wHour >= 9 && sys.wHour <= 16)
	{
		return true;
	}

	return false;
}

//-------------关闭进程---------------------
void CloseProcess(const char* pProcessName)
{
	//char c[] = {"ServerDia.exe"}; //定义字符串并初始化,c为8长度,最后结尾有'/0',定义一个字符为'x',

	HANDLE handle         = NULL;        //定义CreateToolhelp32Snapshot系统快照句柄
	HANDLE handle1        = NULL;        //定义要结束进程句柄
	PROCESSENTRY32 *info = NULL;         //定义PROCESSENTRY32结构字指

	if (NULL == pProcessName)
	{
		return;
	}

	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获得系统快照句柄	

	//PROCESSENTRY32  结构的 dwSize 成员设置成 sizeof(PROCESSENTRY32)
	info         = new PROCESSENTRY32;
	info->dwSize = sizeof(PROCESSENTRY32);
	//调用一次 Process32First 函数，从快照中获取进程列表
	Process32First(handle,info);
	//重复调用 Process32Next，直到函数返回 FALSE 为止

	while(Process32Next(handle,info)!=FALSE)
	{
		info->szExeFile;     //指向进程名字
		//strcmp字符串比较函数同要结束相同
		if(strcmp(pProcessName, info->szExeFile) == 0)
		{
			//PROCESS_TERMINATE表示为结束操作打开,FALSE=可继承,info->th32ProcessID=进程ID
			handle1=OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
			//-------结束进程---------
			TerminateProcess(handle1, 0);
			printf("kill processid=%d,szExeFile=%s\n", info->th32ProcessID, info->szExeFile);
		} 
		//printf("path: szExeFile = %s\n", info->szExeFile);
	}
	CloseHandle(handle);

	if (NULL == info)
	{
		delete info;
		info = NULL;
	}
}

bool HasProcess(const char* pProcessName)
{
	bool bRet = false;

	HANDLE handle         = NULL;        //定义CreateToolhelp32Snapshot系统快照句柄
	HANDLE handle1        = NULL;        //定义要结束进程句柄
	PROCESSENTRY32 *info = NULL;         //定义PROCESSENTRY32结构字指

	if (NULL == pProcessName)
	{
		return bRet;
	}

	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获得系统快照句柄	

	//PROCESSENTRY32  结构的 dwSize 成员设置成 sizeof(PROCESSENTRY32)
	info         = new PROCESSENTRY32;
	info->dwSize = sizeof(PROCESSENTRY32);
	//调用一次 Process32First 函数，从快照中获取进程列表
	Process32First(handle,info);
	//重复调用 Process32Next，直到函数返回 FALSE 为止

	while(Process32Next(handle,info)!=FALSE)
	{
		info->szExeFile;     //指向进程名字
		//strcmp字符串比较函数同要结束相同
		if(strcmp(pProcessName, info->szExeFile) == 0)
		{
			////PROCESS_TERMINATE表示为结束操作打开,FALSE=可继承,info->th32ProcessID=进程ID
			//handle1=OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
			////-------结束进程---------
			//TerminateProcess(handle1, 0);
			//printf("qq---------\n");
			bRet = true;
		} 
		printf("path: szExeFile = %s, th32ProcessID=%d\n", info->szExeFile, info->th32ProcessID);
	}
	CloseHandle(handle);

	if (NULL == info)
	{
		delete info;
		info = NULL;
	}
	return bRet;
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CprotectDlg 对话框




CprotectDlg::CprotectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CprotectDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CprotectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CprotectDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CprotectDlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, &CprotectDlg::OnBnClickedButton2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CprotectDlg 消息处理程序

BOOL CprotectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//pLog =  CreateLogformObj();

	//创建日志对象
	pLog = CreateLogformObj();
	if (!pLog)
	{
		printf("create log error\n");
		return -1;
	}	
	GetAppPath(szPath);
	pLog->SetDir(szPath);
	pLog->SetLogFileName("protect");
	iniFile = new CIniFile();
	sprintf(szIni, "%s//service.ini", szPath);
	iniFile->SetFileName(szIni);//"service.ini"
	iniFile->ReadIniString_In("windows", "app", "tradeServer.exe", szApp, sizeof(szApp));

	iniKeep = new CIniFile();
	memset(szIni, 0, sizeof(szIni));
	sprintf(szIni, "%s\\count.ini", szPath);
	iniKeep->SetFileName(szIni);
	//iniKeep->ReadIniString_In("windows", "app", "tradeServer.exe", szApp, sizeof(szApp));

	
 
	char szBuf[512] = {0};
	sprintf(szBuf, "protect %s", szApp);
	SetWindowText(szBuf);

	SetTimer(1, 2000, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CprotectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CprotectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CprotectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CprotectDlg::OnBnClickedButton1()
{
	//WinExec(".\\mapTest.exe", SW_SHOW);//SW_SHOWMAXIMIZED
	WinExec("tradeServer.exe", SW_SHOW);
}

void CprotectDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		static int myCount = -1; //统计不在线时间
		static int myCount1 = 0;
		static int myCount2 = 0;
		//不存在重启程序
		if (!HasProcess(szApp))
		{//不存在			
			if (-1 == myCount)
			{//第一次发现不存在，立即启动
				pLog->PrintLog("----1-----restart, %s---------\n", szApp);
				WinExec(szApp, SW_MINIMIZE);
				myCount=0;
			}else if (checkIsWork())
			{//在工作日立即启动程序
				pLog->PrintLog("----2-----restart, %s---------\n", szApp);
				WinExec(szApp, SW_MINIMIZE);
				myCount = 0;
			}
			else
			{//不在工作日 2分钟重启一次
				if (60 == myCount)
				{
					pLog->PrintLog("---3------restart, %s---------\n", szApp);
					WinExec(szApp, SW_MINIMIZE);
					myCount = 0;
				}
			}
			myCount++;
			
		}else
		{
			myCount = 0;			
			int myCount3 = iniKeep->ReadIniInt_In("trade", "count", 0);
			//pLog->PrintLog("---------read trade count=%d---------\n", myCount3);
			if (myCount3 != myCount1)
			{
				myCount1 = myCount3;
				myCount2 = 0;
			}else
			{
				myCount2++;
			}

			if (myCount2 > 6)
			{//关闭进程
				CloseProcess(szApp);
				pLog->PrintLog("---------stop, %s, num=%d,---------\n", szApp, myCount2);
				myCount2=0;
			}

		}

	}

	CDialog::OnTimer(nIDEvent);
}

void CprotectDlg::OnBnClickedButton2()
{
	//CloseProcess("mapTest.exe");
	CloseProcess("tradeServer.exe");
}

void CprotectDlg::OnClose()
{
	CloseProcess(szApp); //关闭服务

	CDialog::OnClose();
}

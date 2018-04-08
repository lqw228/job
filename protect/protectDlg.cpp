// protectDlg.cpp : ʵ���ļ�
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

//������Чʱ�� 9�㵽16��
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

//-------------�رս���---------------------
void CloseProcess(const char* pProcessName)
{
	//char c[] = {"ServerDia.exe"}; //�����ַ�������ʼ��,cΪ8����,����β��'/0',����һ���ַ�Ϊ'x',

	HANDLE handle         = NULL;        //����CreateToolhelp32Snapshotϵͳ���վ��
	HANDLE handle1        = NULL;        //����Ҫ�������̾��
	PROCESSENTRY32 *info = NULL;         //����PROCESSENTRY32�ṹ��ָ

	if (NULL == pProcessName)
	{
		return;
	}

	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//���ϵͳ���վ��	

	//PROCESSENTRY32  �ṹ�� dwSize ��Ա���ó� sizeof(PROCESSENTRY32)
	info         = new PROCESSENTRY32;
	info->dwSize = sizeof(PROCESSENTRY32);
	//����һ�� Process32First �������ӿ����л�ȡ�����б�
	Process32First(handle,info);
	//�ظ����� Process32Next��ֱ���������� FALSE Ϊֹ

	while(Process32Next(handle,info)!=FALSE)
	{
		info->szExeFile;     //ָ���������
		//strcmp�ַ����ȽϺ���ͬҪ������ͬ
		if(strcmp(pProcessName, info->szExeFile) == 0)
		{
			//PROCESS_TERMINATE��ʾΪ����������,FALSE=�ɼ̳�,info->th32ProcessID=����ID
			handle1=OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
			//-------��������---------
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

	HANDLE handle         = NULL;        //����CreateToolhelp32Snapshotϵͳ���վ��
	HANDLE handle1        = NULL;        //����Ҫ�������̾��
	PROCESSENTRY32 *info = NULL;         //����PROCESSENTRY32�ṹ��ָ

	if (NULL == pProcessName)
	{
		return bRet;
	}

	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//���ϵͳ���վ��	

	//PROCESSENTRY32  �ṹ�� dwSize ��Ա���ó� sizeof(PROCESSENTRY32)
	info         = new PROCESSENTRY32;
	info->dwSize = sizeof(PROCESSENTRY32);
	//����һ�� Process32First �������ӿ����л�ȡ�����б�
	Process32First(handle,info);
	//�ظ����� Process32Next��ֱ���������� FALSE Ϊֹ

	while(Process32Next(handle,info)!=FALSE)
	{
		info->szExeFile;     //ָ���������
		//strcmp�ַ����ȽϺ���ͬҪ������ͬ
		if(strcmp(pProcessName, info->szExeFile) == 0)
		{
			////PROCESS_TERMINATE��ʾΪ����������,FALSE=�ɼ̳�,info->th32ProcessID=����ID
			//handle1=OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
			////-------��������---------
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CprotectDlg �Ի���




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


// CprotectDlg ��Ϣ�������

BOOL CprotectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//pLog =  CreateLogformObj();

	//������־����
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

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CprotectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
		static int myCount = -1; //ͳ�Ʋ�����ʱ��
		static int myCount1 = 0;
		static int myCount2 = 0;
		//��������������
		if (!HasProcess(szApp))
		{//������			
			if (-1 == myCount)
			{//��һ�η��ֲ����ڣ���������
				pLog->PrintLog("----1-----restart, %s---------\n", szApp);
				WinExec(szApp, SW_MINIMIZE);
				myCount=0;
			}else if (checkIsWork())
			{//�ڹ�����������������
				pLog->PrintLog("----2-----restart, %s---------\n", szApp);
				WinExec(szApp, SW_MINIMIZE);
				myCount = 0;
			}
			else
			{//���ڹ����� 2��������һ��
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
			{//�رս���
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
	CloseProcess(szApp); //�رշ���

	CDialog::OnClose();
}

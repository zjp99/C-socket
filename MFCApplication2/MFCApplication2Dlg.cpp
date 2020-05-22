
// MFCApplication2Dlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MFCApplication2.h"
#include "MFCApplication2Dlg.h"
#include "afxdialogex.h"
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace std;

SOCKET g_SockSrv;
SOCKET g_SockConn[5];
static unsigned char SocketCnt;
BYTE g_Thread1Exit = 0;
BYTE g_Thread2Exit = 0;




DWORD WINAPI AcceptClient_Proc(LPVOID param);
DWORD WINAPI AcceptData_Proc(LPVOID param);
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication2Dlg 对话框



CMFCApplication2Dlg::CMFCApplication2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION2_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_vServerIp = _T("");
	m_vSendEdit = _T("");
	m_vReceiveEdit = _T("");
	m_vPort = 8888;
}

void CMFCApplication2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT3, m_vSendEdit);
	DDX_Text(pDX, IDC_EDIT1, m_vReceiveEdit);
	DDX_Text(pDX, IDC_EDIT2, m_vPort);
	DDX_Control(pDX, IDC_IPADDRESS1, m_vIP);
}

BEGIN_MESSAGE_MAP(CMFCApplication2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication2Dlg::OnBnClickedButton1)

	ON_MESSAGE(MSG_UPDATEDATA, &CMFCApplication2Dlg::OnUpdateMyData)

	
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CMFCApplication2Dlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCApplication2Dlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CMFCApplication2Dlg 消息处理程序

BOOL CMFCApplication2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	WORD wVersionRequested;//保存WinSock库版本号。不是C++标准的类型，是微软SDK中的类型，WORD的意思为字，是2byte（16位）的无符号整数，表示范围0~65535.
	WSADATA wsaData;//存放windows socket初始化信息
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);//加载套接字库，成功返回0
	if (err != 0)
		return FALSE;
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)//判断wsaData.wVersion低字节和高字节是否都为1，若版本不对应，调用WSACleanup函数终止对Socket的使用
	{
		WSACleanup();
		return FALSE;
	}

	char strIP[30] = "127.0.0.1";
	DWORD dwIP = ntohl(inet_addr(strIP));
	m_vIP.SetAddress(dwIP);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication2Dlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCApplication2Dlg::OnBnClickedButton1()
{
	

	//-------创建用于监听的套接字-----------
	g_SockSrv = socket(AF_INET,SOCK_STREAM,0);//参数：AF_INET地址族（这里只能是AF_INET或PF_INET，两者在windows没区别）,SOCK_STREAM基于TCP/IP需要创建的是流式套接字（所以这里只能是SOCK_STREAM），
                                                   //0（根据地址格式和套接字类别，自动选择一个合适的协议）
    //--------绑定套接字------------
	SOCKADDR_IN addrSrv;
	//struct sockaddr_in
	//{ 
	// short sin_family;//地址族AF_INET
	// unsigned short sin_port;//将要分配给套接字的端口
	// struct in_addr sin_addr;//套接字的主机IP地址(32位的IPv4地址) 
	// char sin_zero[8];//填充数，使sockaddr_in结构和sockaddr结构的长度一样,成功返回0
	//};
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//htonl(dwIP);//S_addr需要u_long类型。htonl函数可以将INADDR_ANY值转换为网络字节顺序。htonl函数将一个u_long类型的值从主机字节顺序转换为TCP/IP的网络字节顺序
	                                         //将IP地址指定为INADDR_ANY，允许套接字向任何分配给本地机器的IP地址发送或接收数据。当有多个网卡多个IP，使用INADDR_ANY可以简化编程	
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(8888);//要使用1024以上端口。htons函数将一个u_short类型的值从主机字节顺序转换为TCP/IP网络字节顺序。
	bind(g_SockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));//创建套接字后，要将该套接字绑定到本地的某个地址和端口上。参数（要绑定的套接字，该套接字的本地地址信息，该地址结构的长度）
	
	//-------将套接字设为监听模式，准备接收客户端请求-------
	listen(g_SockSrv,5);//参数（套接字描述符，backlog这里是5（等待连接队列的最大长度））
	

	//开启一个接收客户请求线程
	HANDLE ThreadHandle;

	g_Thread1Exit = 1;
	ThreadHandle = CreateThread(NULL,0,AcceptClient_Proc,this,0,NULL);
	CloseHandle(ThreadHandle);

	// TODO: 在此添加控件通知处理程序代码
}
DWORD WINAPI AcceptClient_Proc(LPVOID param)
{
	SOCKADDR_IN addrClient;//用来接收客户端的地址信息

	CMFCApplication2Dlg *MainDlg = (CMFCApplication2Dlg *)param;

	::SetWindowText(::GetDlgItem(MainDlg->m_hWnd, IDC_EDIT1), _T("等待连接...\r\n"));


	/******************获取类成员变量值****************************************
		MainDlg->SendMessage(MSG_UPDATEDATA);//发送消息
		Sleep(50);
		int val = MainDlg->m_vPort;
		CString str;
		str.Format(_T("prot = %d\r\n"),val);
		::SetWindowText(::GetDlgItem(MainDlg->m_hWnd,IDC_EDIT_receive),str);
	*************************************************************************/

	while (g_Thread1Exit)
	{

		int len = sizeof(SOCKADDR);//对accept的第三个函数来说，在调用之前必须为它赋予一个初始值，即SOCKADDR_IN（？还是SOCKADDR？）结构体长度。

		//等待客户请求的到来
		g_SockConn[SocketCnt] = accept(g_SockSrv, (SOCKADDR*)&addrClient, &len); //参数（已经通过listen函数设置为监听状态的套接字描述符，当客户端向服务器发起连接时用来保存发起连接的IP地址和端口信息，返回保护地址信息的长度）

		if (INVALID_SOCKET != g_SockConn[SocketCnt])
		{
			::SetWindowText(::GetDlgItem(MainDlg->m_hWnd, IDC_EDIT1), _T("已经连接\r\n"));
			SocketCnt++;
			if (SocketCnt >= 5)
			{
				SocketCnt = 0;
				break;
			}

			//开启服务器数据接收线程
			g_Thread2Exit = 1;
			HANDLE ThreadHandle = CreateThread(NULL, 0, AcceptData_Proc, param, 0, NULL);
			CloseHandle(ThreadHandle);

			::MessageBox(MainDlg->m_hWnd, _T("socket连接成功!"), _T("连接成功"), MB_OK);

		}
		/*
		  //判断客户端链接是否断开
		  if(g_Thread2Exit == 1)
		  {
			   int optval = 0 , optlen = sizeof(int);

			   getsockopt(g_SockConn[SocketCnt], SOL_SOCKET, SO_ERROR,(char*) &optval, &optlen);
			   switch(optval)
			   {
				  case 0:    //“处于连接状态“
					  break;
				  case ECONNREFUSED:
					  SocketCnt--;
				  //case EHOSTNUMREACH:

			   }
		  }
		  */
		Sleep(50);
	}

	return 1;
}


LRESULT  CMFCApplication2Dlg::OnUpdateMyData(WPARAM wParam, LPARAM lParam)
{

	//m_vReceiveEdit = "";
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(_T(""));

	UpdateData(TRUE);

	return 1;
}





DWORD WINAPI AcceptData_Proc(LPVOID param)
{
	static char recvBuf[1024];
	UINT length = 0;
	CMFCApplication2Dlg *MainDlg = (CMFCApplication2Dlg *)param;

	//接收服务器数据
	while (g_Thread2Exit)
	{
		Sleep(50);

		memset(recvBuf, 0, 1024);
		length = recv(g_SockConn[SocketCnt - 1], recvBuf, 1024, 0);
		if (length > 0)
		{
			MainDlg->SendMessage(MSG_UPDATEDATA);//发送消息

			CString str(recvBuf);

			::SetWindowText(::GetDlgItem(MainDlg->m_hWnd, IDC_EDIT1), str);
		}

	}

	return 1;
}



void CMFCApplication2Dlg::OnBnClickedButtonClose()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);

	SocketCnt = 0;
	//关线程
	g_Thread2Exit = 0;
	g_Thread1Exit = 0;

	Sleep(100);

	for (char i = 0; i < 5; i++)
		closesocket(g_SockConn[i]);

}


void CMFCApplication2Dlg::OnBnClickedButton3()
{
	if (INVALID_SOCKET == g_SockConn[0])
	{
		MessageBox(_T("没有客户端连接!"));
		return;
	}
	else {

		CString msg;

		GetDlgItem(IDC_EDIT3)->GetWindowTextW(msg);

		//int len = msg.GetLength();
		USES_CONVERSION;

		char SendBuf[1024];

		int len = WideCharToMultiByte(CP_ACP, 0, msg, msg.GetLength(), NULL, 0, NULL, NULL);

		WideCharToMultiByte(CP_ACP, 0, msg, msg.GetLength(), SendBuf, len, NULL, NULL);

		SendBuf[len] = '\0';

		//-------------发送数据--------------
		len = send(g_SockConn[0], SendBuf, len, 0);

		if (len <= 0)
		{
			MessageBox(_T("发送失败!"));
		}
	}
	// TODO: 在此添加控件通知处理程序代码
}

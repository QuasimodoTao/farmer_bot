#include "main.h"
#include <string>
#include "net.h"
#include <string.h>
#include "pack.h"
#include <fstream>
#include "bot.h"
#include <iphlpapi.h>
#include <WS2tcpip.h>

#pragma comment(lib,"Iphlpapi.lib")

Main botApp;

afx_msg void StartStopButton::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if (nChar != TEXT('\n')) CButton::OnChar(nChar, nRepCnt, nFlags);
	else {
		if (isStart) {
			botApp.stopBot();
			isStart = false;
		}
		else {
			botApp.startBot();
			isStart = true;
		}
	}
}
afx_msg void StartStopButton::OnLButtonDown(UINT nFlags, CPoint point) {
	if (isStart) {
		botApp.stopBot();
		isStart = false;
	}
	else {
		botApp.startBot();
		isStart = true;
	}
}
afx_msg void ExplorerButton::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	CFileDialog* fileDialog;
	const wchar_t* path;

	if (nChar != TEXT('\n')) CButton::OnChar(nChar, nRepCnt, nFlags);
	else {
		fileDialog = new CFileDialog(true, NULL, NULL, 6, NULL, this->GetParentFrame());
		fileDialog->DoModal();
		path = fileDialog->GetPathName();
		botApp.configPathEdit->SetWindowTextW(path);
		botApp.loadHead();
	}
}
afx_msg void ExplorerButton::OnLButtonDown(UINT nFlags, CPoint point) {
	CFileDialog* fileDialog;
	CString path;

	fileDialog = new CFileDialog(true, NULL, NULL, 6, NULL, this->GetParentFrame());
	fileDialog->DoModal();
	path = fileDialog->GetPathName();
	botApp.configPathEdit->SetWindowTextW(path);
	botApp.loadHead();
}
bool Main::loadHead() {
	fstream file;
	struct Bot::CONFIG_FILE_HEAD head;
	wchar_t* _configName;
	int len;
	std::wstring config_name = L"config.bin";

	if (len = this->configPathEdit->GetWindowTextLengthW()) {
		len += 8;
		_configName = new wchar_t[len];
		this->configPathEdit->GetWindowTextW(_configName, len);
		config_name = _configName;
		delete _configName;
	}

	file.open(config_name, ios::in | ios::binary);
	if (!file.is_open()) {
		file.open(config_name, ios::out | ios::binary);
		memset(&head, 0, sizeof(head));
		head.magic = Bot::configMagic;
		file.write((char*)&head, sizeof(head));
	}
	else file.read((char*)&head, sizeof(head));
	
	file.close();
	if (head.magic != Bot::configMagic) return false;
	if (head.userVaild) this->userEdit->SetWindowTextW(head.user);
	if (head.passwordVaild) this->passwordEdit->SetWindowTextW(head.password);
	if (head.payNameVaild) this->payEdit->SetWindowTextW(head.payName);
	if (head.annonceNameVaild) this->annonceEdit->SetWindowTextW(head.annonceName);
	return true;
}
bool Main::saveHead() {
	fstream file;
	struct Bot::CONFIG_FILE_HEAD head;
	wchar_t* _configName;
	int len;
	std::wstring config_name = L"config.bin";

	if (len = this->configPathEdit->GetWindowTextLengthW()) {
		len += 8;
		_configName = new wchar_t[len];
		this->configPathEdit->GetWindowTextW(_configName, len);
		config_name = _configName;
		delete _configName;
	}

	file.open(config_name, ios::out | ios::in | ios::binary);
	memset(&head, 0, sizeof(head));
	head.magic = Bot::configMagic;
	if (!file.is_open()) {
		file.open(config_name, ios::out | ios::binary);
		file.write((char*)&head, sizeof(head));
		file.close();
		file.open(config_name, ios::in | ios::out | ios::binary);
	}
	else file.read((char*)&head, sizeof(head));
	if (head.magic != Bot::configMagic) {
		file.close();
		return false; 
	}
	len = this->userEdit->GetWindowTextLengthW();
	if (len <= 0 || len > 32) {
		::MessageBoxW(nullptr, L"用户名和密码不能为空且不能长于32个字符", L"错误", MB_OK);
		file.close();
		return false;
	}
	this->userEdit->GetWindowTextW(head.user, 34);
	head.userVaild = true;
	len = this->passwordEdit->GetWindowTextLengthW();
	if (len <= 0 || len > 32) {
		::MessageBoxW(nullptr, L"用户名和密码不能为空且不能长于32个字符", L"错误", MB_OK);
		file.close();
		return false;
	}
	this->passwordEdit->GetWindowTextW(head.password, 34);
	head.passwordVaild = true;

	len = this->payEdit->GetWindowTextLengthW();
	if (len) {
		if (len > 32) {
			::MessageBoxW(nullptr, L"支付名和通知名不能长于32个字符", L"错误", MB_OK);
			file.close();
			return false;
		}
		this->payEdit->GetWindowTextW(head.payName, 34);
		head.payNameVaild = true;
	}
	else head.payNameVaild = false;
	
	len = this->annonceEdit->GetWindowTextLengthW();
	if (len) {
		if (len > 32) {
			::MessageBoxW(nullptr, L"支付名和通知名不能长于32个字符", L"错误", MB_OK);
			file.close();
			return false;
		}
		this->annonceEdit->GetWindowTextW(head.annonceName, 34);
		head.annonceNameVaild = true;
	}
	else head.annonceNameVaild = false;
	file.seekp(0);
	file.write((char*)&head, sizeof(head));
	file.close();
	return true;
}

BEGIN_MESSAGE_MAP(StartStopButton, CButton)
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()
BEGIN_MESSAGE_MAP(ExplorerButton, CButton)
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()
BEGIN_MESSAGE_MAP(MainWindow,CFrameWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()
BEGIN_MESSAGE_MAP(ChatEdit,CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

afx_msg void ChatEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	int len;
	wchar_t* wstr;
	if (nChar != 13) CEdit::OnChar(nChar, nRepCnt, nFlags);
	else {
		len = this->GetWindowTextLengthW();
		wstr = new wchar_t[len + 8];
		this->GetWindowTextW(wstr, len + 8);
		this->SetWindowTextW(L"");
		botApp.sendChat(wstr);
		delete[] wstr;
	}
}

BOOL Main::InitInstance() {
	RECT rect;
	RECT win_rect;
	struct WSAData d;

	if (WSAStartup(MAKEWORD(2, 0), &d)) {
		return false;
	}

	msgSave = new std::wstring * [msg_count];
	for (int i = 0; i < msg_count; i++) msgSave[i] = new std::wstring(L"");

	this->m_pMainWnd = new MainWindow();
	this->startStopButton = new StartStopButton();
	this->outEdit = new CEdit();
	this->chatEdit = new ChatEdit();
	this->configPathEdit = new CEdit();
	this->explorerButton = new ExplorerButton();

	this->userLab = new CEdit();
	this->passwordLab = new CEdit();
	this->payLab = new CEdit();
	this->annonceLab = new CEdit();

	this->userEdit = new CEdit();
	this->passwordEdit = new CEdit();
	this->payEdit = new CEdit();
	this->annonceEdit = new CEdit();

	this->m_pMainWnd->GetClientRect(&win_rect);

	rect.top = win_rect.top + BORDER_WIDTH;
	rect.bottom = rect.top + CONTROL_HIGH;
	rect.right = win_rect.right - BORDER_WIDTH;
	rect.left = rect.right - CONTROL_WIDTH;
	this->explorerButton->Create(L"浏览", WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::EXPLORER_BUTTON_ID);

	//rect.top = win_rect.top + BORDER_WIDTH;
	//rect.bottom = rect.top + CONTROL_HIGH;
	rect.right = rect.left - BORDER_WIDTH;
	rect.left = win_rect.left + BORDER_WIDTH;
	this->configPathEdit->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::CONFIG_PATH_EDIT_ID);
	this->configPathEdit->SetCueBanner(L"这里输入配置文件的路径");

	rect.top = win_rect.top + BORDER_WIDTH * 2 + CONTROL_HIGH * 1;
	rect.bottom = rect.top + CONTROL_HIGH;

	rect.left = win_rect.left + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	this->userLab->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::USER_LAB_ID);
	this->userLab->SetWindowTextW(L"用户");
	this->userLab->SetReadOnly();

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	this->userEdit->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::USER_EDIT_ID);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	this->passwordLab->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::PASSWORD_LAB_ID);
	this->passwordLab->SetWindowTextW(L"密码");
	this->passwordLab->SetReadOnly();

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	this->passwordEdit->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::PASSWORD_EDIT_ID);
	this->passwordEdit->SetPasswordChar(L'*');

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	this->payLab->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::PAY_LAB_ID);
	this->payLab->SetWindowTextW(L"支付");
	this->payLab->SetReadOnly();

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	this->payEdit->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::PAY_EDIT_ID);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	this->annonceLab->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::ANNONCE_LAB_ID);
	this->annonceLab->SetWindowTextW(L"通知");
	this->annonceLab->SetReadOnly();

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	this->annonceEdit->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::ANNONCE_EDIT_ID);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	this->startStopButton->Create(L"开始", WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::START_STOP_BUTION_ID);

	rect.bottom = win_rect.bottom - BORDER_WIDTH;
	rect.top = rect.bottom - CONTROL_HIGH;
	rect.right = win_rect.right - BORDER_WIDTH;
	rect.left = win_rect.left + BORDER_WIDTH;
	this->chatEdit->Create(WS_VISIBLE | WS_CHILD, rect, this->m_pMainWnd, Main::CHAT_EDIT_ID);
	this->chatEdit->SetReadOnly();

	rect.top = win_rect.top + BORDER_WIDTH * 3 + CONTROL_HIGH * 2;
	rect.bottom = win_rect.bottom - BORDER_WIDTH * 2 - CONTROL_HIGH;
	rect.right = win_rect.right - BORDER_WIDTH;
	rect.left = win_rect.left + BORDER_WIDTH;
	this->outEdit->Create(WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE, rect, this->m_pMainWnd, Main::OUT_EDIT_ID);
	this->outEdit->SetReadOnly();

	this->m_pMainWnd->ShowWindow(this->m_nCmdShow);
	this->m_pMainWnd->UpdateWindow();

	return CWinApp::InitApplication();
}
afx_msg void MainWindow::OnPaint() {
	CPaintDC dc(this);
	CBrush brush(0xdddddd);
	CRect rect;

	this->GetClientRect(&rect);
	dc.FillRect(&rect, &brush);
}
afx_msg void MainWindow::OnSize(UINT nType, int cx, int cy) {
	RECT rect;
	RECT win_rect;

	this->GetClientRect(&win_rect);

	rect.top = win_rect.top + BORDER_WIDTH;
	rect.bottom = rect.top + CONTROL_HIGH;
	rect.right = win_rect.right - BORDER_WIDTH;
	rect.left = rect.right - CONTROL_WIDTH;
	botApp.explorerButton->MoveWindow(&rect);


	//rect.top = win_rect.top + BORDER_WIDTH;
	//rect.bottom = rect.top + CONTROL_HIGH;
	rect.right = rect.left - BORDER_WIDTH;
	rect.left = win_rect.left + BORDER_WIDTH;
	botApp.configPathEdit->MoveWindow(&rect);

	rect.top = win_rect.top + BORDER_WIDTH * 2 + CONTROL_HIGH * 1;
	rect.bottom = rect.top + CONTROL_HIGH;

	rect.left = win_rect.left + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	botApp.userLab->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	botApp.userEdit->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	botApp.passwordLab->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	botApp.passwordEdit->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	botApp.payLab->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	botApp.payEdit->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH / 2;
	botApp.annonceLab->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	botApp.annonceEdit->MoveWindow(&rect);

	rect.left = rect.right + BORDER_WIDTH;
	rect.right = rect.left + CONTROL_WIDTH;
	botApp.startStopButton->MoveWindow(&rect);

	rect.bottom = win_rect.bottom - BORDER_WIDTH;
	rect.top = rect.bottom - CONTROL_HIGH;
	rect.right = win_rect.right - BORDER_WIDTH;
	rect.left = win_rect.left + BORDER_WIDTH;
	botApp.chatEdit->MoveWindow(&rect);

	rect.top = win_rect.top + BORDER_WIDTH * 3 + CONTROL_HIGH * 2;
	rect.bottom = win_rect.bottom - BORDER_WIDTH * 2 - CONTROL_HIGH;
	rect.right = win_rect.right - BORDER_WIDTH;
	rect.left = win_rect.left + BORDER_WIDTH;
	botApp.outEdit->MoveWindow(&rect);
}
afx_msg int MainWindow::OnCreate(LPCREATESTRUCT lpcs) {
	CPaintDC dc(this);
	CBrush brush(0xdddddd);
	CRect rect;

	this->GetClientRect(&rect);
	dc.FillRect(&rect, &brush);
	return 0;
}
MainWindow::MainWindow() {
	this->Create(nullptr, L"Bot");
}
void MainWindow::OnClose() {
	botApp.stopBot();
	__super::OnClose();
}
int Main::ExitInstance() {
	delete this->configPathEdit;
	delete this->explorerButton;
	delete this->startStopButton;
	delete this->outEdit;
	delete this->chatEdit;
	delete this->userLab;
	delete this->passwordLab;
	delete this->payLab;
	delete this->annonceLab;
	delete this->userEdit;
	delete this->passwordEdit;
	delete this->payEdit;
	delete this->annonceEdit;

	for (int i = 0; i < msg_count; i++) delete msgSave[i];
	delete msgSave;
	WSACleanup();
	return 0;
}




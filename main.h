#pragma once
#include <afxwin.h>
#include <afxdlgs.h>
#include <string>
#include "net.h"
#include <atomic>
#include "bot.h"

#define BORDER_WIDTH	3
#define CONTROL_WIDTH	100
#define CONTROL_HIGH	20

//#define MASK_ALL_CHAT	

class StartStopButton : public CButton {
private:
	DECLARE_MESSAGE_MAP();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
	bool isStart = false;
};
class ExplorerButton : public CButton {
	DECLARE_MESSAGE_MAP();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

class ChatEdit : public CEdit {
	DECLARE_MESSAGE_MAP();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class Main : public CWinApp{
private:
	std::wstring** msgSave = nullptr;
	int msgSaveHead = 0;
	//int msgSaveTail = 0;
	static const int msg_count = 32;


public:
	Bot bot;

	StartStopButton* startStopButton = nullptr;//
	ChatEdit* chatEdit = nullptr;//
	ExplorerButton* explorerButton = nullptr;//
	
	CEdit* userLab = nullptr;
	CEdit* passwordLab = nullptr;
	CEdit* payLab = nullptr;
	CEdit* annonceLab = nullptr;

	CEdit* userEdit = nullptr;
	CEdit* passwordEdit = nullptr;
	CEdit* payEdit = nullptr;
	CEdit* annonceEdit = nullptr;

	CEdit* outEdit = nullptr;
	CEdit* configPathEdit = nullptr;

	static const int CONFIG_PATH_EDIT_ID = 1;
	static const int EXPLORER_BUTTON_ID = 2;
	static const int START_STOP_BUTION_ID = 3;
	static const int PAUSE_BUTTON_ID = 4;
	static const int OUT_EDIT_ID = 5;
	static const int CHAT_EDIT_ID = 6;

	static const int USER_LAB_ID = 7;
	static const int PASSWORD_LAB_ID = 8;
	static const int PAY_LAB_ID = 9;
	static const int ANNONCE_LAB_ID = 10;

	static const int USER_EDIT_ID = 11;
	static const int PASSWORD_EDIT_ID = 12;
	static const int PAY_EDIT_ID = 13;
	static const int ANNONCE_EDIT_ID = 14;

	static const int FLUSH_BUTTON_ID = 15;
	static const int ADAPTER_COMBO_BOX_ID = 16;

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	bool loadHead();
	bool saveHead();
	void startBot() {
		wchar_t* configName;
		int len;
		configPathEdit->SetReadOnly(true);
		userEdit->SetReadOnly(true);
		passwordEdit->SetReadOnly(true);
		payEdit->SetReadOnly(true);
		annonceEdit->SetReadOnly(true);
		if (!saveHead()) {
			configPathEdit->SetReadOnly(false);
			userEdit->SetReadOnly(false);
			passwordEdit->SetReadOnly(false);
			payEdit->SetReadOnly(false);
			annonceEdit->SetReadOnly(false);
			return;
		}
		len = configPathEdit->GetWindowTextLengthW();
		if (len) {
			len += 8;
			configName = new wchar_t[len];
			configPathEdit->GetWindowTextW(configName, len);
			bot.set(configName);
			delete[] configName;
		}
		else bot.set((wchar_t* )NULL);
		bot.run();
		chatEdit->SetReadOnly(false);
		this->startStopButton->SetWindowTextW(L"Í£Ö¹");
	}
	void stopBot() {
		configPathEdit->SetReadOnly(true);
		userEdit->SetReadOnly(false);
		passwordEdit->SetReadOnly(false);
		payEdit->SetReadOnly(false);
		annonceEdit->SetReadOnly(false);
		chatEdit->SetReadOnly(true);
		bot.stop(0);
		this->startStopButton->SetWindowTextW(L"¿ªÊ¼");
	}

	void sendChat(const wchar_t* wstr) {
		bot.send_chat(wstr);
	}
	void showMessage(const wchar_t* msg) {
		std::wstring cur(msg);
		showMessage(&cur);
	}
	void showMessage(std::wstring* msg) {
		std::wstring cur(L"");
		*msgSave[msgSaveHead] = *msg;
		msgSaveHead++;
		msgSaveHead %= msg_count;
		if (msgSaveHead) {
			for (int i = msgSaveHead;i < msg_count;i++) {
				if(!msgSave[i]->empty()) cur += *msgSave[i];
			}
			for (int i = 0;i < msgSaveHead;i++) {
				if (!msgSave[i]->empty()) cur += *msgSave[i];
			}
		
		}
		else {
			for (int i = 0;i < msg_count;i++) {
				if (!msgSave[i]->empty()) cur += *msgSave[i];
			}
		}
		outEdit->SetWindowTextW(cur.c_str());
	}
};

class MainWindow : public CFrameWnd {
public:
	MainWindow();
protected:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP();
};

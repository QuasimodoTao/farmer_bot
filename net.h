#pragma once

#define WIN32_LEAN_AND_MEAN

#include "streamarray.h"
#include <String>
#include "pack.h"
#include "mc.h"
#include <winsock2.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

class Net {
private:
	SOCKET sock = 0;
	std::wstring host;
	std::wstring svr;
	const wchar_t * def_svr = L"25565";
	HANDLE mutex = NULL;
	bool compressed = false;
public:
	volatile bool connected = false;
	const static int unknow_host = 1;
	const static int host_unreachable = 2;
	const static int connect_interrupt = 3;

	Net() {}
	void Connect() throw(int);
 	void Disconnect() {
		if (!connected) return;
		connected = false;
		::shutdown(sock, SD_BOTH);
		::closesocket(sock);
		if (mutex) DeleteObject(mutex);
		mutex = nullptr;
	}
	Pack* Recive();
	bool Send(Pack* v);
	void Set(std::wstring* _host, std::wstring* _svr) {
		if (_host) host = *_host;
		if (_svr) svr = *_svr;
	}
	void SetCompress(bool p) {
		compressed = p;
	}
	void PackOutPing(int protocol, std::wstring* host, short port) throw(int);
	void PackOutRequest() throw(int);
	void PackOutHandshake(int protocol, std::wstring* host, short port)throw(int);
	void PackOutLogin(std::wstring* username)throw(int);

	void PackOutTeleCofm(int id)throw(int);
	void PackOutChat(std::wstring* msg)throw(int);
	void PackOutClientStatus(int id)throw(int);
	void PackOutClickWinButton(int win, int bid)throw(int);

	//0x0a

	void PackOutKeepAlive(long long id)throw(int);
	//0x10,lock diffcult
	void PackOutPlayLocation(double x, double y, double z, bool onground)throw(int);
	void PackOutPlayPosRotation(double x, double y, double z, float yaw, float pitch, bool onground)throw(int);
	void PackOutPlayMovement(bool onground)throw(int);
	void PackOutDig(int lev, int x, int y, int z, int face)throw(int);
	void PackOutPlaceBlock(int hand, int x, int y, int z, int face,
		float cx, float cy, float cz, bool inside_block)throw(int);
	void PackOutUseItem(int hand)throw(int);
	void PackOutClientSetting()throw(int);
};


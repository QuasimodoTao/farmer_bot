#pragma once

#include "main.h"
#include "net.h"
#include "pack.h"
#include <Ws2tcpip.h>
#include "zlib.h"

void Net::Connect() throw(int) {
	int ip_host = 0;
	short port = 0;
	addrinfoW info;
	addrinfoW* result;
	int rc;
	int err;
	SOCKADDR_IN self;

	if (connected) return;
	if (host.empty()) throw unknow_host;
	if (svr.empty()) svr = def_svr;
	memset(&info, 0, sizeof(info));
	info.ai_flags = AI_CANONNAME;
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;
	info.ai_protocol = IPPROTO_TCP;
	rc = ::GetAddrInfoW(host.c_str(), svr.c_str(), &info, &result);
	if (rc) throw host_unreachable;
	sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	rc = ::connect(sock, result->ai_addr, result->ai_addrlen);
	::FreeAddrInfoW(result);
	if (rc < 0) {
		err = WSAGetLastError();
		throw connect_interrupt;
	}
	mutex = CreateMutexW(nullptr, 0, nullptr);
	compressed = false;
	connected = true;
}
Pack* Net::Recive(){
	unsigned int len = 0;
	unsigned char byte = 0;
	unsigned int shift = 0;
	int rc;
	char data[512];
	StreamArray result;
	int compress_len;
	unsigned char* df;
	unsigned long long dlen;
	int err;
	int loop_count;

	if (!this->connected) throw connect_interrupt;

	loop_count = 0;
	while (1) {
		rc = ::recv(sock, (char*)&byte, 1, 0);
		if (rc < 0) {
			Disconnect();
			return nullptr;
		}
		if (!rc) {
			loop_count++;
			if (loop_count >= 150) {
				Disconnect();
				return nullptr;
			}
			::Sleep(loop_count);
			continue;
		}
		loop_count = 0;
		len |= ((unsigned int)(byte & 0x7f)) << (shift * 7);
		shift++;
		result.WriteByte(byte);
		if (!(byte & 0x80)) break;
	}
	len = result.ReadVarInt();
	loop_count = 0;
	while (len >= 512) {
		rc = ::recv(sock, data, 512, 0);
		if (rc < 0 ) {
			Disconnect();
			return nullptr;
		}
		if (!rc) {
			loop_count++;
			if (loop_count >= 150) {
				Disconnect();
				return nullptr;
			}
			::Sleep(loop_count);
			continue;
		}
		loop_count = 0;
		result.WriteArray(data, rc);
		len -= rc;
	}
	loop_count = 0;
	while (len) {
		rc = ::recv(sock, data, len, 0);
		if (rc < 0) {
			Disconnect();
			return nullptr;
		}
		if (!rc) {
			loop_count++;
			if (loop_count >= 150) {
				Disconnect();
				return nullptr;
			}
			::Sleep(loop_count);
			continue;
		}
		loop_count = 0;
		result.WriteArray(data, rc);
		len -= rc;
	}
	if (this->compressed) {
		compress_len = result.ReadVarInt();
		if (compress_len <= 0) return new Pack(&result);
		df = new unsigned char[compress_len];
		dlen = compress_len;
		rc = ::uncompress(df, (uLongf*)&dlen, (const Bytef*)result.RBuf(), result.RLen());
		StreamArray r2;
		r2.WriteArray(df, compress_len);
		delete[] df;
		return new Pack(&r2);
	}
	else return new Pack(&result);
}
bool Net::Send(Pack * v) throw(int){
	unsigned int len;
	unsigned int len_shift = 0;
	char varLen[5];
	unsigned char byte = 0;
	int rc;
	int err;

	if (!this->connected) return false;
	len = v->Len();
	if (this->compressed) len++;
	varLen[0] = 0;
	while (len) {
		byte = len & 0x7f;
		len >>= 7;
		if (!len) {
			varLen[len_shift] = byte;
			break;
		}
		varLen[len_shift] = byte | 0x80;
		len_shift++;
	}
	len_shift++;

	WaitForSingleObject(mutex, INFINITE);
	rc = send(sock, varLen, len_shift, 0);
	if (rc <= 0) {
		Disconnect();
		return false;
	}
	if (this->compressed) {
		varLen[0] = 0;
		rc = send(sock, varLen, 1, 0);
		if (rc <= 0) {
			Disconnect();
			return false;
		}
	}
	rc = send(sock, (char*)v->CBuf(), v->Len(), 0);
	if (rc <= 0) {
		Disconnect();
		return false;
	}
	ReleaseMutex(mutex);
	return true;
}
void Net::PackOutPing(int protocol, std::wstring* host, short port) throw (int){
	Pack pk(0);
	if (!port) port = 25565;
	pk.WriteVarInt(protocol);
	pk.WriteString(host);
	pk.WriteShort(port);
	pk.WriteVarInt(1);
	try {
		this->Send(&pk);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutRequest() throw(int) {
	Pack pk(0);
	try {
		this->Send(&pk);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutHandshake(int protocol, std::wstring* host, short port) throw(int) {
	Pack pk(0);
	pk.WriteVarInt(protocol);
	pk.WriteString(host);
	pk.WriteShort(port);
	pk.WriteVarInt(2);
	try {
		this->Send(&pk);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutLogin(std::wstring* username) throw(int) {
	Pack pk(0);
	pk.WriteString(username);
	try {
		this->Send(&pk);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutTeleCofm(int id) throw(int) {
	Pack pk(0);
	pk.WriteVarInt(id);
	try {
		this->Send(&pk);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutChat(std::wstring* msg) throw(int) {
	Pack pk(3);
	pk.WriteString(msg);
	try {
		this->Send(&pk);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutClientStatus(int id) throw(int) {
	Pack pk(4);
	pk.WriteVarInt(id);
	try {
		this->Send(&pk);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutClickWinButton(int win, int bid) throw(int) {
	Pack p(7);
	p.WriteByte(win);
	p.WriteByte(bid);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}

//0x0a

void Net::PackOutKeepAlive(long long id) throw(int) {
	Pack p(0x0f);
	p.WriteLong(id);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}
//0x10,lock diffcult
void Net::PackOutPlayLocation(double x, double y, double z, bool onground) throw(int) {
	Pack p(0x11);
	p.WriteDouble(x);
	p.WriteDouble(y);
	p.WriteDouble(z);
	p.WriteByte(onground ? 1 : 0);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutPlayPosRotation(double x, double y, double z, float yaw, float pitch, bool onground) throw(int) {
	Pack p(0x12);
	p.WriteDouble(x);
	p.WriteDouble(y);
	p.WriteDouble(z);
	p.WriteFloat(yaw);
	p.WriteFloat(pitch);
	p.WriteByte(onground ? 1 : 0);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutPlayMovement(bool onground)throw(int) {
	Pack p(0x14);
	p.WriteByte(onground ? 1 : 0);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutDig(int lev, int x, int y, int z, int face) throw(int) {
	Pack p(0x1a);
	p.WriteVarInt(lev);
	p.WritePos(x, y, z);
	p.WriteByte(face);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutPlaceBlock(int hand, int x, int y, int z, int face,
	float cx, float cy, float cz, bool inside_block) throw(int) {
	Pack p(0x2e);
	p.WriteVarInt(hand);
	p.WritePos(x, y, z);
	p.WriteVarInt(face);
	p.WriteFloat(cx);
	p.WriteFloat(cy);
	p.WriteFloat(cz);
	p.WriteByte(inside_block);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutUseItem(int hand) throw(int) {
	Pack p(0x2f);
	p.WriteVarInt(hand);
	try {
		this->Send(&p);
	}
	catch (int e) {
		throw e;
	}
}
void Net::PackOutClientSetting() throw(int) {
	std::wstring l(L"zh_ch");
	Pack p(0x05);
	p.WriteString(&l);
	p.WriteByte(0);//view distance
	p.WriteVarInt(0);
	p.WriteByte(1);
	p.WriteByte(127);
	p.WriteVarInt(1);
	p.WriteByte(1);
	try { 
		this->Send(&p); 
	}
	catch (int e) {
		throw e;
	}
}

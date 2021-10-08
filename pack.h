#pragma once
#include <string>
#include <objbase.h>
#include "streamarray.h"
#include "mc.h"

using namespace std;

class PackInPing {
public:
	std::wstring * json = nullptr;

	PackInPing(Pack* pk) {
		if (pk->GetId() != 0x00) return;
		json = pk->ReadString();
		delete pk;
	}
	~PackInPing() {
		if (json) delete json;
	}
};
class PackInCompress {
public:
	int size = 0;
	PackInCompress(Pack* p) {
		if (p->GetId() != 0x03)return;
		size = p->ReadVarInt();
		delete p;
	}
};
class PackInDisconnect {
public:
	std::wstring* reason = nullptr;
	PackInDisconnect(Pack* p) {
		if (p->GetId() != 0x00 && 
			p->GetId() != 0x1a) return;
		reason = p->ReadString();
		delete p;
	}
	~PackInDisconnect() {
		if (reason) delete reason;
	}
};
class PackInLoginSuccess {
public:
	std::wstring* user_name = nullptr;
	UUID uuid = {0,};
	PackInLoginSuccess(Pack* p) {
		if (p->GetId() != 0x02) return;
		p->ReadUUID(&uuid);
		user_name = p->ReadString();
		delete p;
	}
	~PackInLoginSuccess() {
		if (user_name) delete user_name;
	}
};
class PackInLoginPlug {
public:
	int id = 0;
	std::wstring* ident = nullptr;
	//unsigned char* data = nullptr;
	PackInLoginPlug(Pack* p) {
		if (p->GetId() != 0x04) return;
		id = p->ReadVarInt();
		ident = p->ReadString();
		delete p;
	}
	~PackInLoginPlug() {
		if (ident) delete ident;
	}
};
class PackInAcknowPlayerDig {
public:
	int x, y, z;
	int block;
	int status;
	bool success;
	PackInAcknowPlayerDig(Pack* p) {
		if (p->GetId() != 0x08) return;
		p->ReadPos(&x, &y, &z);
		status = p->ReadVarInt();
		success = p->ReadByte();
		delete p;
	}
};
class PackInChatMessage {
public:
	std::wstring* json = nullptr;
	int pos;
	UUID uuid;
	PackInChatMessage(Pack* p) {
		if (p->GetId() != 0x0f) return;
		json = p->ReadString();
		pos = p->ReadByte();
		p->ReadUUID(&uuid);
		delete p;
	}
	~PackInChatMessage() {
		if (json) delete json;
	}
};

class PackInWindowItems {
public:
	int id;
	int slot_count;
	Slot* pack[90] = { nullptr, };

	PackInWindowItems(Pack* p) {
		int i;
		id = p->ReadUByte();
		slot_count = p->ReadShort();

		for (i = 0; i < slot_count; i++) pack[i] = new Slot(p);
		delete p;
	}
};
class PackInSetSlot {
public:
	int id;
	int slot_id;
	Slot * slot;
	PackInSetSlot(Pack* p) {
		if (p->GetId() != 0x16)  return;
		id = p->ReadByte();
		//state = p->ReadVarInt();
		slot_id = p->ReadShort();
		slot = new Slot(p);
		delete p;
	}
};
class PackInKeepAlive {
public:
	long long id;
	PackInKeepAlive(Pack* p) {
		if (p->GetId() != 0x21) return;
		id = p->ReadLong();
		delete p;
	}
};
class PackInHeldItemChange {
public:
	int slot;
	PackInHeldItemChange(Pack* p) {
		slot = p->ReadByte();
		delete p;
	}

};
class PackInUpdateHealth {
public:
	float health;
	int food;
	PackInUpdateHealth(Pack* p) {
		health = p->ReadFloat();
		food = p->ReadVarInt();
		delete p;
	}
};
class PackInJoinGame {
public:
	int id;
	int world_count;
	std::wstring* w[3] = {nullptr,};

	PackInJoinGame(Pack* p) {
		if (p->GetId() != 0x26) return;
		id = p->ReadInt();
		p->ReadByte();
		p->ReadByte();
		p->ReadByte();
		world_count = p->ReadVarInt();
		if (world_count > 3) world_count = 3;
		for (int i = 0;i < world_count;i++)
			w[i] = p->ReadString();
		delete p;
	}
	~PackInJoinGame() {
		for (int i = 0;i < 3;i++)
			if (w[i]) delete w[i];
	}
};
class PackInPlayerPositionAndLook {
public:
	double x, y, z;
	int flags;
	int id;
	PackInPlayerPositionAndLook(Pack* p) {
		x = p->ReadDouble();
		y = p->ReadDouble();
		z = p->ReadDouble();
		p->ReadFloat();
		p->ReadFloat();
		flags = p->ReadByte();
		id = p->ReadVarInt();
		delete p;
	}
};
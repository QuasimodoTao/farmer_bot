#pragma once

#include "pack.h"

class NBT {
private:
	void _read(Pack* p) {
		int type;
		int count;
		std::wstring* name;
		int i;

		while (1) {
			type = p->ReadUByte();
			if (!type) return;
			name = p->ReadShortString();
			if (*name == L"Damage" && type == 3) {
				damage = p->ReadInt();
			}
			else if (*name == L"RepairCost" && type == 3) {
				repairCost = p->ReadInt();
			}
			else {
				switch (type) {
				case 1: p->ReadByte(); break;
				case 2: p->ReadShort(); break;
				case 3: p->ReadInt(); break;
				case 4: p->ReadLong(); break;
				case 5: p->ReadFloat(); break;
				case 6: p->ReadDouble(); break;
				case 7:
					count = p->ReadInt();
					while (count--) p->ReadByte();
					break;
				case 8:
					delete p->ReadShortString();
					break;
				case 9:
					type = p->ReadByte();
					count = p->ReadInt();

					if (count <= 0) break;
					for (int i = 0; i < count; i++) {
						switch (type) {
						case 1: p->ReadByte(); break;
						case 2: p->ReadShort(); break;
						case 3: p->ReadInt(); break;
						case 4: p->ReadLong(); break;
						case 5: p->ReadFloat(); break;
						case 6: p->ReadDouble(); break;
						case 7:
							count = p->ReadInt();
							while (count--) p->ReadByte();
							break;
						case 8: delete p->ReadShortString(); break;
						case 9:
							break;
						case 10: this->_read(p); break;
						case 11:
							count = p->ReadInt();
							while (count--) p->ReadInt();
							break;
						case 12:
							count = p->ReadInt();
							while (count--) p->ReadLong();
							break;
						}
					}
					break;
				case 10:
					this->_read(p); return;
				case 11:
					count = p->ReadInt();
					while (count--) p->ReadInt();
					break;
				case 12:
					count = p->ReadInt();
					while (count--) p->ReadLong();
					break;
				}
			}
			delete name;
		}
	}
public:
	unsigned int damage = 0;
	unsigned int repairCost = 0;
	unsigned int len = 0;
	unsigned long long enchantments = 0;
	unsigned char enchantmentsLvl[32] = { 0, };
	char* data = nullptr;

	static const unsigned char max_level[32];

	NBT() {}
	NBT(Pack* p) {
		read(p);
	}
	~NBT() {
		if (data) delete data;
	}
	void read(Pack* p) {
		unsigned int pos;

		pos = p->tellg();
		_read(p);
		len = p->tellg() - pos;
		data = new char[len];
		p->ReadArray(data, pos, len);
	}
	void write(Pack* p) {
		if (len) p->WriteArray(data, len);
	}
};
class Slot {
public:
	bool present = false;
	unsigned char count;
	int id;
	NBT nbt;

	Slot() {}
	Slot(Pack* p) {
		read(p);
	}
	void read(Pack* p) {
		present = p->ReadByte();
		if (!present) return;
		id = p->ReadVarInt();
		count = p->ReadByte();
		nbt.read(p);
	}
	void write(Pack* p) {
		p->WriteByte(present);
		if (!present) return;
		p->WriteVarInt(id);
		p->WriteByte(count);
		nbt.write(p);
	}
};
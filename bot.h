#pragma once

#include "net.h"
#include "pack.h"
#include "mc.h"
#include <Windows.h>
#include <atomic>
#include <fstream>

class Thread {
private:
	volatile bool isActiving = false;
	HANDLE hThread = nullptr;
	DWORD tid = 0;
	std::atomic_bool activedLock;
	int retVal = 0;
public:
	Thread() {
		this->activedLock = false;
	}
	~Thread() {
		this->stop(0);
	}
	virtual int start() { return 0; }
	virtual int finally() { return 0; }
	static DWORD WINAPI threadEntry(LPVOID lparam) {
		Thread* _this = (Thread*)lparam;
		_this->retVal = _this->start();
		_this->finally();
		_this->isActiving = false;
		return _this->retVal;
	}
	int run() {
		while ((activedLock).exchange(true));
		if (isActiving == true) return 1;
		isActiving = true;
		hThread = CreateThread(nullptr, 0, threadEntry, this, 0, &tid);
		if (!hThread) {
			isActiving = false;
			this->activedLock.store(false);
			return -1;
		}
		this->activedLock.store(false);
		return 0;
	}
	void join() {
		while (isActiving == true) ::Sleep(1);
	}
	void stop(int _retVal) {
		while (activedLock.exchange(true));
		if (!isActiving) {
			this->activedLock.store(false);
			return;
		}
		TerminateThread(hThread, _retVal);
		DeleteObject(hThread);
		retVal = _retVal;
		finally();
		isActiving = false;
		this->activedLock.store(false);
	}
	int getRet() {
		return retVal;
	}
	int getTID() {
		return tid;
	}
};
class Bot : public Thread {
	friend class Net;
public:
	struct CONFIG_FARM {
		wchar_t tpPoint[30];
		double oX, oY, oZ;
		double eX, eZ;
	};
	struct CONFIG_FILE_HEAD {
		int magic;
		int nextfarm;
		int farmCount;
		bool userVaild;
		bool passwordVaild;
		bool payNameVaild;
		bool annonceNameVaild;
		unsigned char hostCount;
		unsigned char hostVaild[7];
		wchar_t user[34];
		wchar_t password[34];
		wchar_t payName[34];
		wchar_t annonceName[34];
		wchar_t host[7][34];
		//farm point
	};
	static const int configMagic = 0xced8a034;
private:
	struct CONFIG_FARM* farm = nullptr;
	std::wstring host[7] = { 
		std::wstring(),std::wstring(), std::wstring(), std::wstring(), 
		std::wstring(), std::wstring(), std::wstring()};
	struct CONFIG_FILE_HEAD head = { 0, };

	typedef void (Bot::* FUNCTIONTYPE)(Pack*);

	void (Bot::*pack_in_event[0x67])(Pack*)  = {
		/*00*/nullptr,//spawn enity
		/*01*/nullptr,//spawn experience orb
		/*02*/nullptr,//spawn living entity
		/*03*/nullptr,//spawn painting
		/*04*/nullptr,//spawn player
		/*05*/nullptr,//sculk vibration signal
		/*06*/nullptr,//entity animation
		/*07*/nullptr,//statistics
		/*08*/nullptr,//Bot::pi_acknowledge_dig,//acknowledge player digging
		/*09*/nullptr,//block break animation
		/*0a*/nullptr,//block entity data;
		/*0b*/nullptr,//block action
		/*0c*/nullptr,//block change
		/*0d*/nullptr,//boss bar
		/*0e*/nullptr,//server diffcult
		/*0f*/& Bot::pi_chat_message,//chat message
		/*10*/nullptr,//clear titiles
		/*11*/nullptr,//tab-complete
		/*12*/nullptr,//declare commands
		/*13*/nullptr,//close window
		/*14*/& Bot::pi_window_items,//window items
		/*15*/nullptr,//window property
		/*16*/& Bot::pi_set_slot,//set slot
		/*17*/nullptr,//set cooldown
		/*18*/nullptr,//plugin message
		/*19*/nullptr,//named sound effect
		/*1a*/& Bot::pi_disconnect,//disconnect
		/*1b*/nullptr,//entity status
		/*1c*/nullptr,//explosion
		/*1d*/nullptr,//& Bot::pi_unload_chunk,//unload chunk
		/*1e*/nullptr,//change game state
		/*1f*/nullptr,//open horse window
		/*20*/nullptr,//initialize word border
		/*21*/& Bot::pi_keep_alive,//keep alive
		/*22*/nullptr,//chunk data
		/*23*/nullptr,//effect
		/*24*/nullptr,//particle
		/*25*/nullptr,//update light
		/*26*/& Bot::pi_join_game,//join game
		/*27*/nullptr,//Bot::pi_map_data,//map data
		/*28*/nullptr,//trade list
		/*29*/nullptr,//Bot::pi_entity_position,//entiry position
		/*2a*/nullptr,//Bot::pi_entiry_position_and_rotation,//entity position and totation
		/*2b*/nullptr,//entity rotation
		/*2c*/nullptr,//vehicle move
		/*2d*/nullptr,//open book
		/*2e*/nullptr,//open window
		/*2f*/nullptr,//open sign editor
		/*30*/nullptr,//ping
		/*31*/nullptr,//craft recipe response
		/*32*/nullptr,//player abilities
		/*33*/nullptr,//end combat event
		/*34*/nullptr,//enter combat event
		/*35*/nullptr,//death combat event
		/*36*/nullptr,//player info
		/*37*/nullptr,//face player
		/*38*/&Bot::pi_player_position_and_look,//player position and look
		/*39*/nullptr,//unlock recipes
		/*3a*/nullptr,//destory entity
		/*3b*/nullptr,//remove entity effect
		/*3c*/nullptr,//resource pack send
		/*3d*/& Bot::pi_respawn,//respawn
		/*3e*/nullptr,//entity head look
		/*3f*/nullptr,//mult block change
		/*40*/nullptr,//select advancement tab
		/*41*/nullptr,//action bar
		/*42*/nullptr,//world border center
		/*43*/nullptr,//world border lerp size
		/*44*/nullptr,//world border size
		/*45*/nullptr,//world border warning delay
		/*46*/nullptr,//world border warning reach
		/*47*/nullptr,//camera
		/*48*/& Bot::pi_held_item_change,//held item change
		/*49*/nullptr,//update view position
		/*4a*/nullptr,//update view distance
		/*4b*/nullptr,//Bot::pi_spawn_position,//spawn position
		/*4c*/nullptr,//display scoreboard
		/*4d*/nullptr,//Bot::pi_entity_metadata,//entity metadata
		/*4e*/nullptr,//attach entity
		/*4f*/nullptr,//entity velocity
		/*4f*/nullptr,//Bot::pi_entity_equipment,//entity equipment
		/*51*/nullptr,//set experience
		/*52*/& Bot::pi_update_health,//update health
		/*53*/nullptr,//scoreboard objective
		/*54*/nullptr,//set passengers
		/*55*/nullptr,//teams
		/*56*/nullptr,//update score
		/*57*/nullptr,//set title subtitile
		/*58*/nullptr,//time update
		/*59*/nullptr,//set title text
		/*5a*/nullptr,//set title times
		/*5b*/nullptr,//entity sound effect
		/*5c*/nullptr,//sound effect
		/*5d*/nullptr,//stop sound
		/*5e*/nullptr,//player list header and footer
		/*5f*/nullptr,//Bot::pi_nbt_query_response,//nbt query response
		/*60*/nullptr,//Bot::pi_collect_item,//collect item
		/*61*/nullptr,//entity teleport
		/*62*/nullptr,//advancements
		/*63*/nullptr,//enitty properties
		/*64*/nullptr,//entity effect
		/*65*/nullptr,//declare recipes
		/*66*/nullptr//Tags
	
	};
	std::wstring* curentHost = nullptr;
	std::wstring user = std::wstring(L"");
	std::wstring password = std::wstring(L"");

	class BotKeepAliveThread :public Thread {
	private:
		Bot* bot = nullptr;
	public:
		void set(Bot* _bot) {
			bot = _bot;
		}
		int start();
	};
	class BotAvdThread :public Thread {
	private:
		Bot* bot = nullptr;
	public:
		void set(Bot* _bot) {
			bot = _bot;
		}
		int start() {
			int cur_delay = 0;
			while (1) {
				::Sleep(1000 * 60);
				if (bot->adv_enable) {
					if (cur_delay <= 0) { 
						cur_delay = (volatile int)bot->adv_delay;
						bot->send_chat(&bot->adv);
					}
				}
				cur_delay--;
			}
		}
	};
	class BotMainThread : public Thread {
	private:
		Bot* bot = nullptr;
		BotKeepAliveThread keepAlive;
		BotAvdThread advThread;
		std::fstream cfg;
		std::wstring send;
	public:
		void set(Bot* _bot) {
			bot = _bot;
		}
		int start();
		int finally() {
			keepAlive.stop(0);
			advThread.stop(0);
			cfg.close();
			return 0;
		}
	};

	BotMainThread main;

	void pi_acknowledge_dig(Pack* p);
	void pi_block_entity_data(Pack* p);
	void pi_chat_message(Pack* p);
	void pi_set_slot(Pack* p);
	void pi_disconnect(Pack* p);
	void pi_keep_alive(Pack* p);
	void pi_join_game(Pack* p);
	void pi_map_data(Pack* p);
	void pi_respawn(Pack* p);
	void pi_held_item_change(Pack* p);
	void pi_spawn_position(Pack* p);
	void pi_entity_metadata(Pack* p);
	void pi_entity_equipment(Pack* p);
	void pi_update_health(Pack* p);
	void pi_nbt_query_response(Pack* p);
	void pi_collect_item(Pack* p);
	void pi_window_items(Pack* p);
	void pi_player_position_and_look(Pack* _p);

	bool loadConfig();

	std::wstring configName = std::wstring(L"");

public:
	double x = 0, y = 0, z = 0;
	volatile int food = 20;
	volatile bool login_success = false;
	volatile int delay = 0;

	volatile double ex = 0, ey = 0, ez = 0;
	volatile bool cmd_end = false;

	std::wstring pay = std::wstring(L"");
	std::wstring annonce = std::wstring(L"");
	Net net = Net();
	volatile time_t keep_alive_time = 0;
	volatile Slot* pack[46] = { nullptr, };
	volatile int hold_slot = 0;
	volatile int window_state = 0;
	volatile bool chunk_load_disable = false;

	volatile bool adv_enable = false;
	int adv_delay = 0;
	std::wstring adv;

	double * sealx = nullptr;
	double * sealy = nullptr;
	double * sealz = nullptr;
	int seal_total_len = 0;

	void make_seal_xyz() {
		double x;
		int i;

		if (!sealx) {
			sealx = new double[256];
			sealy = new double[256];
			sealz = new double[256];
		}
		i = 0;
		x = -7051.5;
		while (x <= -6897.5) {
			sealx[i] = x;
			sealy[i] = 62;
			sealz[i] = -6295.5;
			i++;
			x += 1.0;
		}
		x = -6896.5;
		while (x <= -6894.5) {
			sealx[i] = x;
			sealy[i] = 63;
			sealz[i] = -6295.5;
			i++;
			x += 1.0;
		}
		x = -6893.5;
		while (x <= -6888.5) {
			sealx[i] = x;
			sealy[i] = 73;
			sealz[i] = -6295.5;
			i++;
			x += 1.0;
		}
		x = -6295.5;
		while (x <= -6282.5) {
			sealx[i] = -6888.5;
			sealy[i] = 73;
			sealz[i] = x;
			i++;
			x += 1.0;
		}
		sealx[i] = -6888.5;
		sealy[i] = 64;
		sealz[i] = -6282.5;
		i++;
		seal_total_len = i;
	}

	void start_eat() throw(int) {
		try {
			this->net.PackOutUseItem(1);
		}
		catch (int e) {
			throw e;
		}
	}
	void end_eat() throw(int) {
		try {
			this->net.PackOutDig(5, 0, 0, 0, 0);
		}
		catch (int e) {
			throw e;
		}
	}
	void harvest(double x, double y, double z) throw(int) {
		try {
			this->net.PackOutDig(0, x, y + 0.99, z, 1);
			this->net.PackOutDig(2, x, y + 0.99, z, 1);
		}
		catch (int e) {
			throw e;
		}
	}
	void plant(double x, double y, double z) throw(int) {
		try {
			this->net.PackOutPlaceBlock(1, x, y + 0.99, z, 1, 0.5, 0.5, 0.5, false);
		}
		catch (int e) {
			throw e;
		}
	}
	void send_chat(const wchar_t* chat) throw(int) {
		std::wstring c(chat);
		try {
			this->net.PackOutChat(&c);
		}
		catch (int e) {
			throw e;
		}
	}
	void send_chat(std::wstring* c) throw(int) {
		try {
			this->net.PackOutChat(c);
		}
		catch (int e) {
			throw e;
		}
	}
	void click(int x, int y, int z, int face) throw(int) {
		try {
			this->net.PackOutDig(0, x, y, z, face);
			this->net.PackOutDig(1, x, y, z, face);
		}
		catch (int e) {
			throw e;
		}
	}
	void move_to(double x, double y, double z) throw(int) {
		try {
			this->net.PackOutPlayMovement(true);
			this->net.PackOutPlayPosRotation(x, y, z, 0, 0, true);
		}
		catch (int e) {
			throw e;
		}
		this->x = x;
		this->y = y;
		this->z = z;
	}
	static std::wstring* get_test_from_json(std::wstring* json);
	int harvest_farm(double orgx, double orgy, double orgz, double endx, double endz) throw(int);
	void seal() throw(int) {
		double x;
		int i;
		bool eat = false;
		wchar_t carrot_count[16];
		double rank,rank_;

		try {
			if (this->food < 20) {
				this->start_eat();
				eat = true;
			}
			while (1) {
				this->ex = -7053.920;
				this->ey = 63;
				this->ez = -6295.250;
				this->cmd_end = false;
				this->send_chat(L"/res tp fiby");
				i = 0;
				while (!this->cmd_end) {
					if (!net.connected) throw - 1;
					::Sleep(100);
					i++;
					if (i >= 120) {
						i = -1;
						break;
					}
				}
				if (i >= 0) break;
			}
			if (eat) this->end_eat();
			i = 0;

			while (1) {
retry:
				for (; i < seal_total_len; i++) {
					this->move_to(sealx[i], sealy[i], sealz[i]);
					::Sleep(50);
					if (sealx[i] != this->x || sealy[i] != this->y || sealz[i] != this->z) {
						rank = 1000 * 1000;
						for (i = 0; i < seal_total_len; i++) {
							if (sealx[i] == this->x && sealy[i] == this->y && sealz[i] == this->z) break;
							rank_ = (sealx[i] - this->x) * (sealx[i] - this->x) +
								(sealy[i] - this->y) * (sealy[i] - this->y) +
								(sealz[i] - this->z) * (sealx[i] - this->z);
							if (rank_ < rank) rank = rank_;
							else break;
						}
					}
				}
				this->cmd_end = false;
				this->click(-6884, 64, -6283, 5);
				while (!this->cmd_end) {
					if (!net.connected) throw - 1;
					::Sleep(100);
					if (sealx[i - 1] != this->x || sealy[i - 1] != this->y || sealz[i - 1] != this->z) {
						rank = 1000 * 1000;
						for (i = 0; i < seal_total_len; i++) {
							if (sealx[i] == this->x && sealy[i] == this->y && sealz[i] == this->z) break;
							rank_ = (sealx[i] - this->x) * (sealx[i] - this->x) +
								(sealy[i] - this->y) * (sealy[i] - this->y) +
								(sealz[i] - this->z) * (sealx[i] - this->z);
							if (rank_ < rank) rank = rank_;
							else break;
						}
						goto retry;
					}


				}
				swprintf_s(carrot_count, 16, L"%d", this->counter_item(947) - 32);
				this->send_chat(carrot_count);
				::Sleep(200);
				if (sealx[i - 1] != this->x || sealy[i - 1] != this->y || sealz[i - 1] != this->z) {
					if (counter_item(947) > 32) {
						rank = 1000 * 1000;
						for (i = 0; i < seal_total_len; i++) {
							if (sealx[i] == this->x && sealy[i] == this->y && sealz[i] == this->z) break;
							rank_ = (sealx[i] - this->x) * (sealx[i] - this->x) +
								(sealy[i] - this->y) * (sealy[i] - this->y) +
								(sealz[i] - this->z) * (sealx[i] - this->z);
							if (rank_ < rank) rank = rank_;
							else break;
						}
						goto retry;
					}
				}

				if (!this->pay.empty()) {
					this->cmd_end = false;
					this->send_chat(L"/money");
					while (!this->cmd_end) {
						if (!net.connected) throw - 1;
						::Sleep(100);
					}
				}
				return;
			}
		}
		catch (int e) {
			throw e;
		}
	}
	Bot() {}
	int counter_item(int item) {
		int i;
		int count = 0;
		Slot* s;
		for (i = 9; i < 45; i++) {
			s = (Slot*)pack[i];
			if (s && s->present && s->id == item) count += s->count;
		}
		return count;
	}
	int counter_rubbish(int item_count, int* item) {
		int i, j;
		int count = 0;
		Slot* s;
		
		for (i = 9; i < 45; i++) {
			s = (Slot*)pack[i];
			if (!s || !s->present) continue;
			for (j = 0; j < item_count; j++) 
				if (s->id == item[j]) break;
			if (j >= item_count) count++;
		}
		return count;
	}
	void drop_rubbish(int item_count, int* item) throw(int){
		int i, j;
		Slot* s;
		for (i = 9; i < 45; i++) {
			s = (Slot*)pack[i];
			if (!s || !s->present) continue;
			for (j = 0; j < item_count; j++)
				if (s->id == item[j]) break;
			if (j >= item_count) {
				Pack p0(0x08);

				s = (Slot*)pack[i];
				p0.WriteUByte(0);
				p0.WriteShort(i);
				p0.WriteByte(0);
				p0.WriteVarInt(0);
				p0.WriteVarInt(1);
				p0.WriteShort(i);
				p0.WriteByte(false);
				s->write(&p0);
				try {
					net.Send(&p0);
				}
				catch (int e) {
					throw e;
				}
				::Sleep(100);
				Pack p1(0x08);
				p1.WriteUByte(0);
				p1.WriteShort(-999);
				p1.WriteByte(0);
				p1.WriteVarInt(0);
				p1.WriteVarInt(0);
				p1.WriteByte(0);
				try {
					net.Send(&p1);
				}
				catch (int e) {
					throw e;
				}
			}
		}
	}
	void set(std::wstring* _configName) {
		configName = *_configName;
	}
	void set(const wchar_t* _configName) {
		if (_configName) configName = _configName;
		else configName = L"";
	}
	int start();
	int finally();
};
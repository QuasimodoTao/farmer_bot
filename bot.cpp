#include "main.h"
#include "bot.h"
#include <string>
#include <fstream>

extern Main botApp;

int Bot::BotMainThread::start() {
	int i, j;
	int unrubbish[] = { 947,728 };

	if (!bot) return 0;
	keepAlive.set(bot);
	keepAlive.run();
	advThread.set(bot);
	advThread.run();

	bot->net.PackOutClientSetting();
	while (!bot->login_success) ::Sleep(100);

	if (!bot->annonce.empty()) {
		send = L"/msg ";
		send += bot->annonce;
		send += L" Your bot is here";
		try {
			bot->send_chat(&send);
		}
		catch (int e) {
			return 0;
		}
	}
	while (1) {
		if (bot->head.farmCount) {
			bot->make_seal_xyz();
			while (1) {
				int rubbish_count;
				try {
					if (bot->counter_item(947) > 32) {
						bot->chunk_load_disable = true;
						bot->seal();
					}
				}
				catch (...) {
				}
				cfg.open(bot->configName, ios::in | ios::out | ios::binary);
				cfg.seekg(4);
				cfg.read((char*)&i, 4);
				j = i;
				i++;
				i %= bot->head.farmCount;
				cfg.seekp(4);
				cfg.write((char*)&i, 4);
				cfg.close();
				i = j;
				try {
					bot->chunk_load_disable = false;
					send = L"/res tp ";
					send += bot->farm[i].tpPoint;
					while (1) {
						bot->send_chat(&send);
						bot->cmd_end = false;
						j = 0;
						while (!bot->cmd_end) {
							j++;
							::Sleep(100);
							if (j >= 120) {
								j = 0;
								break;
							}
						}
						if (j) break;
					}
					if (!bot->annonce.empty()) {
						send = L"/msg ";
						send += bot->annonce;
						send += L" ";
						send += bot->farm[i].tpPoint;
						bot->send_chat(&send);
					}
					if (bot->counter_rubbish(2, unrubbish)) bot->drop_rubbish(2, unrubbish);
					bot->harvest_farm(bot->farm[i].oX, bot->farm[i].oY, bot->farm[i].oZ, bot->farm[i].eX, bot->farm[i].eZ);
				}
				catch (int e) {
					break;
				}
			}
		}
		else while (1) ::Sleep(10000);
	}
	keepAlive.stop(0);
	advThread.stop(0);
	return 0;
}
int Bot::BotKeepAliveThread::start() {
	time_t tm;

	if (!bot) return 0;
	time(&tm);
	bot->keep_alive_time = tm;
	while (this->bot->net.connected) {
		::Sleep(1000);
		tm++;
		if (tm - bot->keep_alive_time >= 20) {
			this->bot->net.Disconnect();
			return 0;
		}
	}
	return 0;
}

bool Bot::loadConfig() {

	int len;

	std::fstream file;
	if (configName.empty()) configName = L"config.bin";
	file.open(configName, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		file.open(configName, std::ios::out | std::ios::binary);
		memset(&head, 0, sizeof(head));
		head.magic = configMagic;
		//head.annocneNameLen = 0;
		//head.farmCount = 0;
		//head.hostCount = 0;
		//head.nameLen = 0;
		//head.nextfarm = 0;
		//head.passwordLen = 0;
		//head.payNameLen = 0;
		file.write((char*)&head, sizeof(head));
		file.close();
		file.open(configName, std::ios::in | std::ios::binary);
	}
	file.seekg(0);
	file.read((char*)&head, sizeof(head));
	if (head.magic != configMagic) {
		file.close();
		return false;
	}
	if (!head.userVaild) {
		file.close();
		return false;
	}
	else {
		user = head.user;
	}
	if (!head.passwordVaild) {
		file.close();
		return false;
	}
	else {
		password = head.password;
	}
	if (!head.hostCount) {
		file.close();
		return false;
	}
	else {
		int i, j;
		for (i = 0,j = 0;i < head.hostCount;i++) {
			if (head.hostVaild[i]) {
				host[j] = head.host[i];
				j++;
			}
		}
	}
	if (head.payNameVaild) pay = head.payName;
	if (head.annonceNameVaild) annonce = head.annonceName;
	
	if (head.farmCount) {
		farm = new struct CONFIG_FARM[head.farmCount];
		file.read((char*)farm, sizeof(struct CONFIG_FARM) * head.farmCount);
	}
	file.close();
	return true;
}
int Bot::finally() {
	try {
		main.stop(0);
		botApp.bot.net.Disconnect();
		botApp.startStopButton->SetWindowTextW(L"¿ªÊ¼");
		botApp.startStopButton->isStart = false;
	}
	catch (...) {
	
	}
	return 0;
}
int Bot::start() {
	Pack* p;
	PackInPing* ping;
	PackInLoginSuccess* login;
	PackInCompress* compress;
	std::wstring send;
	int i;
	PackInDisconnect* disconnect;
	int curHostIndex;

	if (!loadConfig()) return -1;
	//head.farmCount = 0;
	i = 0;
	//srand((unsigned int)time(NULL));
	this->main.set(this);
	while (1) {
		login_success = false;
		curHostIndex = 0;
		while (curHostIndex < head.hostCount) {
			this->net.Set(&this->host[curHostIndex], nullptr);
			this->curentHost = &this->host[curHostIndex];
			botApp.showMessage(&send);
			try {
				this->net.Connect();
			}
			catch (int e) {
				send = L"Connect host <";
				send += *this->curentHost;
				send += L"> fail.\r\n";;
				botApp.showMessage(&send);
				::Sleep(3000 + i * 1000);
				i++;
				curHostIndex++;
				curHostIndex %= head.hostCount;
				continue;
			}
			break;
		}
		send = L"Connect host <";
		send += *this->curentHost;
		send += L"> success.\r\n";
		botApp.showMessage(&send);
		try {
			this->net.PackOutPing(755, this->curentHost, 0);
			this->net.PackOutRequest();
		}
		catch (int e) {
			botApp.showMessage(L"Connect interrupted.\r\n");
			this->net.Disconnect();
			::Sleep(3000 + i * 1000);
			i++;
			continue;
		}
		p = this->net.Recive();
		this->net.Disconnect();
		if (!p || p->GetId() != 0x00) {
			send = L"Ping host <";
			send += *this->curentHost;
			send += L"> fail.\r\n";
			botApp.showMessage(&send);
			delete p;
			::Sleep(3000 + i * 1000);
			i++;
			continue;
		}
		send = L"Ping host <";
		send += *this->curentHost;
		send += L"> success.\r\n";
		botApp.showMessage(&send);
		ping = new PackInPing(p);
		if (!ping->json) {
			botApp.showMessage(L"Unsupport server.\r\n");
			delete ping;
			::Sleep(3000 + i * 1000);
			i++;
			continue;
		}
		delete ping;
		i = 0;
		botApp.showMessage(&send);
		try {
			this->net.Connect();
		}
		catch (int e) {
			send = L"Reconnect host <";
			send += *this->curentHost;
			send += L"> fail.\r\n";
			botApp.showMessage(&send);
		}
		send = L"Reconnect host <";
		send += *this->curentHost;
		send += L"> success.\r\n";
		botApp.showMessage(&send);
		try {
			this->net.PackOutHandshake(755, this->curentHost, 0);
			this->net.PackOutLogin(&this->user);
		}
		catch (int e) {
			botApp.showMessage(L"Connect interrupted.\r\n");
			this->net.Disconnect();
			::Sleep(3000 + i * 1000);
			i++;
			continue;
		}
		while (1) {
			if(!(p = this->net.Recive())) {
				botApp.showMessage(L"Connect interrupted.\r\n");
				this->net.Disconnect();
				::Sleep(3000 + i * 1000);
				i++;
				break;
			}
			if (p->GetId() == 0x00) {
				disconnect = new PackInDisconnect(p);
				std::wstring* out = this->get_test_from_json(disconnect->reason);
				*out += L"\r\n";
				botApp.showMessage(out);
				delete out;
				delete disconnect;
				botApp.showMessage(L"Handshake fail.\r\n");
				::Sleep(3000 + i * 1000);
				i++;
				break;
			}
			else if (p->GetId() == 0x01) {
				net.Disconnect();
				botApp.showMessage(L"Unsuppoet server.\r\n");
				return -1;
			}
			else if (p->GetId() == 0x02) {
				delete p;
				i = 0;
				break;
			}
			else if (p->GetId() == 0x03) {
				compress = new PackInCompress(p);
				if (compress->size > 0) this->net.SetCompress(true);
			}
			else delete p;
		}
		if (i) continue;

		this->main.run();

		while (1) {
			int id;

			if (!(p = net.Recive())) break;
			id = p->GetId();
			if (id < 0 || id >= 0x67) delete p;
			else if (pack_in_event[id]) (this->*(pack_in_event[id]))(p);
			else delete p;
		}
		this->main.stop(0);
		botApp.showMessage(L"Link disconnect.\r\n");
		this->net.Disconnect();
		::Sleep(5000);
		while (this->net.connected) ::Sleep(1000);
		i = 0;
	}
	return 0;
}
int Bot::harvest_farm(double orgx, double orgy, double orgz, double endx, double endz) throw (int) {
	int dx, dz;
	int _dx, _dz;
	double cx, cz;
	int i = 0;
	int width = 8;
	int delay1, delay2;
	int counter = 0;
	int block;
	double tmp;
#ifdef NEED_PLANT
	delay1 = 35;
	delay2 = 70;
#else
	delay2 = 90;
#endif

	if (endx > orgx) {
		if (endz > orgz) {
			if (endx - orgx > endz - orgz) {
				_dx = 36;
				_dz = width;
				cx = 1.0;
				cz = 1.0;
			}
			else {
				_dz = 36;
				_dx = width;
				cx = 1.0;
				cz = 1.0;
			}
		}
		else {
			if (endx - orgx > orgz - endz) {
				_dx = 36;
				_dz = width;
				cx = 1.0;
				cz = -1.0;
			}
			else {
				_dz = 36;
				_dx = width;
				cx = 1.0;
				cz = -1.0;
			}
		}
	}
	else {
		if (endz > orgz) {
			if (orgx - endx > endz - orgz) {
				_dx = 36;
				_dz = width;
				cx = -1.0;
				cz = 1.0;
			}
			else {
				_dz = 36;
				_dx = width;
				cx = -1.0;
				cz = 1.0;
			}
		}
		else {
			if (orgx - endx > orgz - endz) {
				_dx = 36;
				_dz = width;
				cx = -1.0;
				cz = -1.0;
			}
			else {
				_dz = 36;
				_dx = width;
				cx = -1.0;
				cz = -1.0;
			}
		}
	}

	if (_dx >= _dz) {
		_dx >>= 1;
		for (dx = 0;dx < _dx;dx++) {
			for (dz = 0;dz < _dz;dz++) {
				counter++;
				try {
					if(counter & 0x01) this->move_to(orgx, orgy, orgz);
					this->harvest(orgx, orgy, orgz);
#ifdef NEED_PLANT
					::Sleep(delay1);
					this->plant(orgx, orgy, orgz);
#endif
				}
				catch (int e) {
					throw e;
				}
				::Sleep(delay2);
				orgz += cz;
			}
			orgz -= cz;
			orgx += cx;
			for (dz = 0;dz < _dz;dz++) {
				counter++;
				try {
					if (counter & 0x01) this->move_to(orgx, orgy, orgz);
					this->harvest(orgx, orgy, orgz);
#ifdef NEED_PLANT
					::Sleep(delay1);
					this->plant(orgx, orgy, orgz);
#endif
				}
				catch (int e) {
					throw e;
				}
				::Sleep(delay2);
				orgz -= cz;
			}
			orgz += cz;
			orgx += cx;
		}
	}
	else {
		_dz >>= 1;
		for (dz = 0;dz < _dz;dz++) {
			for (dx = 0;dx < _dx;dx++) {
				counter++;
				try {
					if (counter & 0x01) this->move_to(orgx, orgy, orgz);
					this->harvest(orgx, orgy, orgz);
#ifdef NEED_PLANT
					::Sleep(delay1);
					this->plant(orgx, orgy, orgz);
#endif
				}
				catch (int e) {
					throw e;
				}
				::Sleep(delay2);
				orgx += cx;
			}
			orgx -= cx;
			orgz += cz;
			for (dx = 0;dx < _dx;dx++) {
				counter++;
				try {
					if (counter & 0x01) this->move_to(orgx, orgy, orgz);
					this->harvest(orgx, orgy, orgz);
#ifdef NEED_PLANT
					::Sleep(delay1);
					this->plant(orgx, orgy, orgz);
#endif
				}
				catch (int e) {
					throw e;
				}
				::Sleep(delay2);
				orgx -= cx;
			}
			orgx += cx;
			orgz += cz;
		}
	}
	return 0;
}

std::wstring* Bot::get_test_from_json(std::wstring* json) {
	std::wstring* res;
	wchar_t text[] = L"text";
	const wchar_t* rev;
	int off;
	int len;
	wchar_t ch;
	int i;

	res = new std::wstring(L"");

	rev = json->c_str();
	len = json->length();
	off = 0;
	while (off < len) {
		if (rev[off] != L't') {
			off++;
			continue;
		}
		if (!wcsncmp(rev + off, L"text", 4)) {
			off += 7;
			while (rev[off] != L'\"') {
				if (rev[off] == L'\\' && rev[off + 1] == L'u') {
					off += 2;
					ch = 0;
					for (i = 0;i < 4;i++) {
						if (rev[off] >= L'0' && rev[off] <= '9') {
							ch <<= 4;
							ch |= rev[off] - L'0';
						}
						else if (rev[off] >= L'a' && rev[off] <= 'f') {
							ch <<= 4;
							ch |= rev[off] - L'a' + 10;
						}
						else if (rev[off] >= L'A' && rev[off] <= 'F') {
							ch <<= 4;
							ch |= rev[off] - L'A' + 10;
						}
						else break;
						off++;
					}
					*res += ch;
				}
				else { 
					*res += rev[off]; 
					off++;
				}
			}
		}
		off++;
	}
	return res;
}
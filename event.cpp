#include "main.h"
#include "mc.h"
#include "bot.h"
#include "pack.h"

extern Main botApp;


void Bot::pi_acknowledge_dig(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a acknowledge digging pack.\r\n");
#endif
	delete p;
}
void Bot::pi_block_entity_data(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a block entity data pack.\r\n");
#endif
	delete p;
}
void Bot::pi_chat_message(Pack* _p) {
	std::wstring* text;
	wchar_t money[32];
	int delay;
	wchar_t ch;
	int i,off;
	const wchar_t* rev;
	PackInChatMessage p(_p);
	char* astr;

	text = Bot::get_test_from_json(p.json);
	*text += L"\r\n";
	botApp.showMessage(text);
	rev = text->c_str();
	
	if (wcsstr(rev, L"/login")) {
		std::wstring send = L"/login ";
		send += password;
		this->send_chat(&send);
	}
	else if (!wcsncmp(rev, L"[Wolfx服务器] 已成功登录", 16)) {
		login_success = true;
	}
	else if (!wcsncmp(rev,L"已传送到附近的领地",9)) {
		this->cmd_end = true;
	}
	else if (!wcsncmp(rev, L"准备传送..", 6)) {
		this->cmd_end = true;
	}
	else if (!wcsncmp(rev, L"待处理的传送请求已取消", 11)) {
		this->delay = 1;
		this->cmd_end = true;
	}
	else if(!wcsncmp(L"余额：", rev,3)){
		if (!this->pay.empty()) {
			off = 0;
			for (i = 0;i < 30;off++) {
				ch = rev[off];
				if (ch == L'\"' || !ch || ch == L'\r' || ch == L'.') break;
				if (ch > L'9' || ch < L'0') continue;
				money[i] = ch;
				i++;
			}
			money[i] = 0;
			if (money[0] != L'0') {
				std::wstring send = L"/pay ";
				send += this->pay;
				send += L" ";
				send += money;
				this->send_chat(&send);
			}
			this->cmd_end = true;
		}
	}
	else if (!wcsncmp(L"在聊天栏中输入想 出售 的物品数量", rev, 17)) {
		this->cmd_end = true;
	}
#ifdef MASK_ALL_CHAT
	else {
		if (rev[0] == L'[') while (*rev && *rev != L'<') rev++;
		if (rev[0] == L'<') {
			wchar_t player_name[256];
			int name_len = 0;
			rev++;
			while (*rev) {
				if (*rev == L'>') {
					player_name[name_len] = 0;
					std::wstring send = L"/ignore ";
					send += player_name;
					this->send_chat(&send);
					break;
				}
				player_name[name_len] = *rev;
				rev++;
				name_len++;
			}
		}
	}
#endif
	delete text;
}
void Bot::pi_set_slot(Pack* _p) {
	PackInSetSlot p(_p);
	if (p.id != 0) return;
	if (p.slot_id >= 46) return;

	if (pack[p.slot_id]) delete pack[p.slot_id];
	pack[p.slot_id] = p.slot;
}
void Bot::pi_disconnect(Pack* _p) {
	PackInDisconnect p(_p);
#ifdef _DEBUG
	botApp.showMessage(L"Recive a disconnect pack.\r\n");
#endif
	*(p.reason) += L"\r\n";
	botApp.showMessage(p.reason);
}
void Bot::pi_keep_alive(Pack* _p) {
	PackInKeepAlive p(_p);
	::time((time_t*)&this->keep_alive_time);
	this->net.PackOutKeepAlive(p.id);
}
void Bot::pi_join_game(Pack* p) {
	delete p;
	this->net.PackOutClientSetting();
}
void Bot::pi_map_data(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a map data pack.\r\n");
#endif
	delete p;
}
void Bot::pi_respawn(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a respawn pack.\r\n");
#endif
	delete p;
}
void Bot::pi_held_item_change(Pack* _p) {
	PackInHeldItemChange p(_p);
	this->hold_slot = p.slot + 36;
}
void Bot::pi_spawn_position(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a spawn position pack.\r\n");
#endif
	delete p;
}
void Bot::pi_entity_metadata(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a entity metadata pack.\r\n");
#endif
	delete p;
}
void Bot::pi_entity_equipment(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a entity equipment pack.\r\n");
#endif
	delete p;
}
void Bot::pi_update_health(Pack* _p) {
	PackInUpdateHealth p(_p);
#ifdef _DEBUG
	botApp.showMessage(L"Recive a update health pack.\r\n");
#endif
	Bot::food = p.food;
	if (p.health <= 0) {
		::Sleep(100);
		net.PackOutClientStatus(0);
		::Sleep(100);
		net.Disconnect();
	}
}
void Bot::pi_nbt_query_response(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a NBT query response pack.\r\n");
#endif
	delete p;
}
void Bot::pi_collect_item(Pack* p) {
#ifdef _DEBUG
	botApp.showMessage(L"Recive a collect item pack.\r\n");
#endif
	delete p;
}
void Bot::pi_window_items(Pack* _p) {
	PackInWindowItems p(_p);
	int i;

	if (p.id != 0) return;
	for (i = 0; i < p.slot_count; i++) {
		if (pack[i]) delete pack[i];
		pack[i] = p.pack[i];
	}
}
void Bot::pi_player_position_and_look(Pack* _p) {
	PackInPlayerPositionAndLook p(_p);

#ifdef _DEBUG
	botApp.showMessage(L"Recive a player position and look pack.\r\n");
#endif
	if (p.x == this->ex && p.y == this->ey && p.z == this->ez) {
		this->cmd_end = true;
	}
	x = p.x;
	y = p.y;
	z = p.z;
	this->net.PackOutTeleCofm(p.id);
}
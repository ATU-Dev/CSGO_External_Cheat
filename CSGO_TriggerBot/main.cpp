#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include "memory.h"
#include "offsets.h"
#include "dirent.h"
using namespace std;
using namespace offsets::signatures;
using namespace offsets::netvars;

const auto mem = Memory{ "csgo.exe" };
const auto client = mem.GetModuleAddress("client.dll");
const auto engine = mem.GetModuleAddress("engine.dll");

struct Vec3 {
	float x, y, z;
	Vec3 operator+(Vec3 d) {
		return { x + d.x, y + d.y, z + d.z };
	}
	Vec3 operator-(Vec3 d) {
		return { x - d.x, y - d.y, z - d.z };
	}
	Vec3 operator*(float d) {
		return { x *d, y *d, z *d };
	}
	void Normalize() {
		while (y < -180) y += 360;
		while (y > 180) y -= 360;
		if (x > 89) x = 89;
		if (x < -89) x = -89;
	}
};
string fileContent(string path) {
	ifstream file;
	file.open(path);
	stringstream content;
	content << file.rdbuf();
	return content.str();
}
void PrintCheats();
struct ToggleCheat {
	bool activated = true;
	int pressed = 0;
	int key;
	void CheckToggle()
	{
		if ((GetAsyncKeyState(key) && 0x8000) && pressed == 0) {
			pressed = 1;
			activated = !activated;
			PrintCheats();
		}
		else if (GetAsyncKeyState(key) == 0)
			pressed = 0;
	}
};

ToggleCheat triggerBot; 
ToggleCheat radar;
ToggleCheat glowEsp;
ToggleCheat noRecoil;

void PrintCheats() {
	printf("\033[A\33[2K\r");
	printf("\033[A\33[2K\r");
	printf("\033[A\33[2K\r");
	printf("\033[A\33[2K\r");
	cout << "* Trigger-Bot [F1 to toggle] - " << ((triggerBot.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
	cout << "* Radar       [F2 to toggle] - " << ((radar.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
	cout << "* Glow ESP    [F3 to toggle] - " << ((glowEsp.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
	cout << "* No Recoil   [F4 to toggle] - " << ((noRecoil.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
}

int main()
{	
	

	triggerBot.key = VK_F1;
	radar.key = VK_F2;
	glowEsp.key = VK_F3;
	noRecoil.key = VK_F4;

	system("title CS:GO Cheat by Aturan");
	cout << hex << "Started Cheat:  client.dll -> 0x" << client << dec;
	cout << hex << "  engine.dll -> 0x" << engine << dec << endl;
	cout << "* Trigger-Bot [F1 to toggle] - " << ((triggerBot.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
	cout << "* Radar       [F2 to toggle] - " << ((radar.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
	cout << "* Glow ESP    [F3 to toggle] - " << ((glowEsp.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
	cout << "* No Recoil   [F4 to toggle] - " << ((noRecoil.activated == 1) ? "\033[92mOn\033[0m " : "\033[91mOff\033[0m") << endl;
	
	uintptr_t engineState = mem.Read<uintptr_t>(engine + dwClientState);
	Vec3 oldPunch{ 0,0,0 };

	while (1)
	{
		triggerBot.CheckToggle();
		radar.CheckToggle();
		glowEsp.CheckToggle();
		noRecoil.CheckToggle();

		if (triggerBot.activated) {
			// Trigger-Bot
			uintptr_t locPlayer = mem.Read<uintptr_t>(client + dwLocalPlayer);
			int32_t locHealth = mem.Read<int32_t>(locPlayer + m_iHealth);

			if (locHealth)												// Am I dead?
			{
				int32_t crossId = mem.Read<int32_t>(locPlayer + m_iCrosshairId);

				if (crossId && crossId <= 64)							// Aiming to enemy?
				{
					uintptr_t player = mem.Read<uintptr_t>(client + dwEntityList + (crossId - 1) * 0x10);

					if (mem.Read<int32_t>(player + m_iHealth)) 			// Enemy dead?
					{
						if (mem.Read<int32_t>(player + m_iTeamNum) !=	// Team member?
							mem.Read<int32_t>(locPlayer + m_iTeamNum))
						{
							mem.Write<uintptr_t>(client + dwForceAttack, 6);
							this_thread::sleep_for(chrono::milliseconds(10));
							mem.Write<uintptr_t>(client + dwForceAttack, 4);
						}
					}
				}
			}
		}
		if (radar.activated) {
			// Radar
			for (int i = 1; i < 64; i++)
			{
				DWORD entity = mem.Read<DWORD>(client + dwEntityList + i * 0x10);
				if (entity) {
					mem.Write<bool>(entity + m_bSpotted, true);
				}
			}
		}
		if (glowEsp.activated) {
			// Glow ESP
			uintptr_t locPlayer = mem.Read<uintptr_t>(client + dwLocalPlayer);
			const auto glowObjectManager = mem.Read<uintptr_t>(client + dwGlowObjectManager);
			for (auto i = 1; i < 64; i++) {
				const auto entity = mem.Read<uintptr_t>(client + dwEntityList + i * 0x10);
				if (entity != NULL) {
					if (mem.Read<uintptr_t>(entity + m_iTeamNum) != mem.Read<uintptr_t>(locPlayer + m_iTeamNum)) {
						const auto glowIndex = mem.Read<int32_t>(entity + m_iGlowIndex);
						mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0x8, 255.0);
						mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0xC, 0.0);
						mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0x10, 0.0);
						mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0x14, 255.0);

						mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x27, true);
						mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
					}
				}
			}
		}
		if (noRecoil.activated) {
			// No Recoil
			uintptr_t locPlayer = mem.Read<uintptr_t>(client + dwLocalPlayer);
			Vec3 viewAngles = mem.Read<Vec3>(engineState + dwClientState_ViewAngles);
			Vec3 aimPunchAngle = mem.Read<Vec3>(locPlayer + m_aimPunchAngle);
			int shotsFired = mem.Read<int>(locPlayer + m_iShotsFired);

			Vec3 punchAngle = aimPunchAngle * 2;

			if (shotsFired > 1) {
				Vec3 newAngle = viewAngles + oldPunch - punchAngle;
				newAngle.Normalize();
				mem.Write<Vec3>(engineState + dwClientState_ViewAngles, newAngle);
			}
			oldPunch = punchAngle;
		}

		this_thread::sleep_for(chrono::milliseconds(1));
	}

	return 0;
}
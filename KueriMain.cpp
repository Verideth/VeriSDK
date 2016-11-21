#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <shlwapi.h>			
#include <math.h>
#include "checksum_crc.h"

#include "KueriSDK.h"

CVMTHookManager* panelVMT;
CVMTHookManager* engineVMT;
CVMTHookManager* clientModeVMT;

fnPaintTraverse paintTraverseHook;
fnCreateMove createMoveHook;

VeriSDK::Input* input;

VeriSDK::Panel* panel;

unsigned long tahoma;

VeriSDK::ClientEntityList* entList;
VeriSDK::Surface* surf = (VeriSDK::Surface*)tools->IFInfo("vguimatsurface.dll", "VGUI_Surface031");

class Render
{
public:
	void DrawInlineRect(int X, int Y, int W, int H, Color rgba)
	{
		DrawRect(X, Y, W, H, rgba);
		DrawRect(X + 1, Y + 1, W - 2, H - 2, Color(0, 0, 0, 255));
		DrawRect(X - 1, Y - 1, W + 2, H + 2, Color(0, 0, 0, 255));
	}

	void DrawRect(int X, int Y, int W, int H, Color rgba)
	{
		surf->DrawSetColor(rgba);
		surf->DrawOutlinedRect(X, Y, X + W, Y + H);
	}

	void InitFont(unsigned long &hFont, const char* szFontName, int size, VeriFontFlags flags, int weight)
	{
		hFont = surf->CreateFont();
		surf->SetFontGlyphSet(hFont, szFontName, size, weight, 0, 0, flags);
	}

	void DrawString1(int x, int y, Color rgba, int font, const char* text, ...)
	{
		if (text == NULL)
		{
			return;
		}

		va_list va_alist;
		char buffer[1024] = { '\0' };
		wchar_t string[1024] = { '\0' };

		va_start(va_alist, text);
		vsprintf_s(buffer, text, va_alist);
		va_end(va_alist);

		wsprintfW(string, L"%S", buffer);

		surf->DrawSetTextPos(x, y);
		surf->DrawSetTextFont(font);
		surf->DrawSetTextColor(rgba);
		surf->DrawPrintText(string, wcslen(string), 0);
	}
};

Render* render;

void __stdcall PaintTraverse(int vguiID, bool force, bool allowForcing)
{
	paintTraverseHook(panel, vguiID, force, allowForcing);

	if (!strcmp("FocusOverlayPanel", panel->GetName(vguiID)))
	{
		render->DrawString1(10, 10, Color(255, 25, 255, 255), 24, "K");
		render->DrawString1(30, 10, Color(25, 50, 255, 255), 24, "u");
		render->DrawString1(50, 10, Color(25, 255, 255, 255), 24, "e");
		render->DrawString1(70, 10, Color(255, 255, 255, 255), 24, "r");
		render->DrawString1(90, 10, Color(25, 25, 25, 255), 24, "i");
	}
}

void __stdcall CreateMove(int sequence_number, float input_sample_frametime, bool active)
{
	createMoveHook(clientMode, sequence_number, input_sample_frametime, active);
	VeriSDK::UserCmd* cmdlist = *reinterpret_cast<VeriSDK::UserCmd**>(DWORD(input) + 0xEC);
	VeriSDK::UserCmd* cmd = &cmdlist[sequence_number % 150];
	
	/* BHOP */
	//something is definetely wrong with hooking
	int nLocalPlayerID = engine->GetLocalPlayer();
	VeriSDK::ClientEntity* pLocal = static_cast<VeriSDK::ClientEntity*>(entList->GetClientEntity(nLocalPlayerID));

	if (cmd->buttons & IN_JUMP && !(pLocal->GetFlags() & FL_ONGROUND))
	{
		engine->SetViewAngles(VeriSDK::Vector(25, 0, 25));
	}
}

void Init()
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	SetConsoleTitle("Kueri");

	std::cout << "Welcome to Kueri" << std::endl;

	// Got this from dude aswell.

	std::cout << "Gathering interfaces!" << std::endl;

	clientMode = static_cast<VeriSDK::BaseClientDLL*>(tools->IFInfo("client.dll", "VClient018"));
	engine = (VeriSDK::Engine*)tools->IFInfo("engine.dll", "VEngineClient014");
	panel = (VeriSDK::Panel*)tools->IFInfo("vgui2.dll", "VGUI_Panel009");
	entList = (VeriSDK::ClientEntityList*)tools->IFInfo("client.dll", "VClientEntityList003");
	input = *reinterpret_cast<VeriSDK::Input**>((*reinterpret_cast<DWORD**>(clientMode))[15] + 0x1);
	
	panelVMT = new CVMTHookManager((DWORD**)panel);
	engineVMT = new CVMTHookManager((DWORD**)engine);
	clientModeVMT = new CVMTHookManager((DWORD**)clientMode);

	std::cout << "Hooking paint traverse..." << std::endl;

	paintTraverseHook = (fnPaintTraverse)panelVMT->HookMethod((DWORD)PaintTraverse, 41);

	std::cout << "Hooking create move..." << std::endl;

	createMoveHook = (fnCreateMove)clientModeVMT->HookMethod((DWORD)CreateMove, 21);

	if (paintTraverseHook)
	{
		std::cout << "Painthook was hooked successfully!" << std::endl;
	}

	if (createMoveHook)
	{
		std::cout << "Createmove was hooked successfully!" << std::endl;
	}
	
	return;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Init, 0, 0, 0);
		break;
	}

	return 1;
}

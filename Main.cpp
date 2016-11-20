#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <shlwapi.h>			

#include "SDK.h"

CVMTHookManager* panelVMT;

hkPaintTraverse paintTraverseHook;

VeriSDK::Panel* panel;
VeriSDK::Engine* engine;
VeriSDK::IFTools* tools = new VeriSDK::IFTools;

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
		render->DrawString1(50, 50, Color(255, 0, 255, 255), 20, "Hello, world!\n");
	}
}

void Init()
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	SetConsoleTitle("Veriscript 3.0");

	std::cout << "Welcome to Veriscript 3.0" << std::endl;

	std::cout << "Gathering interfaces!" << std::endl;

	engine = (VeriSDK::Engine*)tools->IFInfo("engine.dll", "VEngineClient014");
	panel = (VeriSDK::Panel*)tools->IFInfo("vgui2.dll", "VGUI_Panel009");

	std::cout << "Hooking paint traverse..." << std::endl;

	panelVMT = new CVMTHookManager((DWORD**)panel);

	paintTraverseHook = (hkPaintTraverse)panelVMT->HookMethod((DWORD)PaintTraverse, 41);	

	if (paintTraverseHook)
	{
		std::cout << "Painthook was hooked successfully!" << std::endl;
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

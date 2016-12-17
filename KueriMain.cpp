#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <shlwapi.h>			
#include <math.h>

#include "KureiSDK.h"

CVMTHookManager* panelVMT;
CVMTHookManager* engineVMT;
CVMTHookManager* baseClientDLLVMT;
CVMTHookManager* clientModeVMT;
CVMTHookManager* mdlRenderVMT;

fnPaintTraverse paintTraverseHook;
fnCreateMove createMoveHook;
fnOverrideView overrideViewHook;
fnDrawModelExecute drawModelExecuteHook;

VeriSDK::ClientEntityList* entList = (VeriSDK::ClientEntityList*)tools->IFInfo("client.dll", "VClientEntityList003");

VeriSDK::Surface* surf = (VeriSDK::Surface*)tools->IFInfo("vguimatsurface.dll", "VGUI_Surface031");
//
//static int ticks = 0;
//int ticksMax = 16;
//
//class Render
//{
//public:
//	void DrawInlineRect(int X, int Y, int W, int H, Color rgba)
//	{
//		DrawRect(X, Y, W, H, rgba);
//		DrawRect(X + 1, Y + 1, W - 2, H - 2, Color(0, 0, 0, 255));
//		DrawRect(X - 1, Y - 1, W + 2, H + 2, Color(0, 0, 0, 255));
//	}
//
//	void DrawRect(int X, int Y, int W, int H, Color rgba)
//	{
//		surf->DrawSetColor(rgba);
//		surf->DrawOutlinedRect(X, Y, X + W, Y + H);
//	}
//
//	void InitFont(unsigned long &hFont, const char* szFontName, int size, VeriFontFlags flags, int weight)
//	{
//		hFont = surf->CreateFont();
//		surf->SetFontGlyphSet(hFont, szFontName, size, weight, 0, 0, flags);
//	}
//
//	void DrawString1(int x, int y, Color rgba, int font, const char* text, ...)
//	{
//		if (text == NULL)
//		{
//			return;
//		}
//
//		va_list va_alist;
//		char buffer[1024] = { '\0' };
//		wchar_t string[1024] = { '\0' };
//
//		va_start(va_alist, text);
//		vsprintf_s(buffer, text, va_alist);
//		va_end(va_alist);
//
//		wsprintfW(string, L"%S", buffer);
//
//		surf->DrawSetTextPos(x, y);
//		surf->DrawSetTextFont(font);
//		surf->DrawSetTextColor(rgba);
//		surf->DrawPrintText(string, wcslen(string), 0);
//	}
//};
//
//Render* render;

void __stdcall PaintTraverse(int vguiID, bool force, bool allowForcing)
{
	paintTraverseHook(panel, vguiID, force, allowForcing);

	for (int i = 0; i < entList->GetHighestEntityIndex(); i++)
	{
		VeriSDK::BaseEntity* baseEntity = entList->GetClientEntity(i);
		VeriSDK::BaseEntity* localPlayer = entList->GetClientEntity(engine->GetLocalPlayer());

		VeriSDK::Vector vecMins = VeriSDK::Vector(0, 0, 0);
		VeriSDK::Vector vecMax = VeriSDK::Vector(20, 50, 20);

		dbgOverlay->AddBoxOverlay(baseEntity->GetVecOrigin(), vecMins, vecMax, baseEntity->GetAbsAngles(), 50, 50, 245, 5, 0.01);
	}
}

void ChangeName(const char* name)
{

}

//void __stdcall DrawModelExecute(VeriSDK::MatRenderContext* ctx, const VeriSDK::DrawModelState_t &state, const VeriSDK::ModelRenderInfo_t &info, VeriSDK::matrix3x4_t *customBoneToWorld)
//{
//	if (info.pModel)
//	{
//		std::string modelName = modelInfo->GetModelName(info.pModel);
//		std::string handsName = modelInfo->GetModelName(info.pModel);
//
//		if (modelName.find("models/player") != std::string::npos)
//		{
//			VeriSDK::Material* mat = matSys->FindMaterial(modelName.c_str(), TEXTURE_GROUP_MODEL);
//			mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
//			mat->SetMaterialVarFlag(MATERIAL_VAR_FLAT, true);
//			mat->SetMaterialVarFlag(MATERIAL_VAR_ALPHATEST, true);
//			mat->AlphaModulate(1.0);
//			mdlRender->ForcedMaterialOverride(mat);
//		}
//	}
//
//	drawModelExecuteHook(mdlRender, ctx, state, info, customBoneToWorld);
//}

/*	
	if (!GetAsyncKeyState(VK_INSERT))
	{
		return;
	}

	if (clientMode->GetPlayerView(setup))
	{
		setup.x = 500;
		setup.y = 500;
		setup.origin = pLocal->GetOrigin();
		setup.angles = pLocal->GetEyeAngles();
		setup.angles.y += 180.0;
		setup.fov = 90.0;
		clientMode->RenderView(setup, 0, 2);
	}
*/

void Init()
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	//freopen("CONOUT$", "w", stderr);

	SetConsoleTitle("Kueri");

	std::cout << "Welcome to Kueri chams" << std::endl;

	std::cout << "Gathering interfaces..." << std::endl;

	baseClientDLL = static_cast<VeriSDK::BaseClientDLL*>(tools->IFInfo("client.dll", "VClient018"));
	engine = (VeriSDK::Engine*)tools->IFInfo("engine.dll", "VEngineClient014");
	panel = (VeriSDK::Panel*)tools->IFInfo("vgui2.dll", "VGUI_Panel009");
	modelInfo = (VeriSDK::ModelInfo*)tools->IFInfo("engine.dll", "VModelInfoClient004");
	dbgOverlay = (VeriSDK::DebugOverlay*)tools->IFInfo("engine.dll", "VDebugOverlay004");
	//input = *reinterpret_cast<VeriSDK::Input**>((*reinterpret_cast<DWORD**>(baseClientDLL))[15] + 0x1);
	//clientMode = **(VeriSDK::ClientMode***)(utils::FindPattern((DWORD)GetModuleHandle("client.dll"), 0x7CE000, clientModeSignature, sizeof(clientModeSignature)) + 0x2); // again, credits @dude719
	mdlRender = (VeriSDK::ModelRender*)tools->IFInfo("engine.dll", "VEngineModel016");
	renderView = (VeriSDK::RenderView*)tools->IFInfo("engine.dll", "VEngineRenderView013");
	matSys = (VeriSDK::MatSys*)tools->IFInfo("materialsystem.dll", "VMaterialSystem080");
	cVar = (VeriSDK::CVar*)tools->IFInfo("vstdlib.dll", "VEngineCvar007");


	panelVMT = new CVMTHookManager((DWORD**)panel);
	engineVMT = new CVMTHookManager((DWORD**)engine);
	baseClientDLLVMT = new CVMTHookManager((DWORD**)baseClientDLL);
	clientModeVMT = new CVMTHookManager((DWORD**)clientMode);
	mdlRenderVMT = new CVMTHookManager((DWORD**)mdlRender);

	std::cout << "Hooking paint traverse..." << std::endl;

	paintTraverseHook = (fnPaintTraverse)panelVMT->HookMethod((DWORD)PaintTraverse, 41);

	////std::cout << "Hooking create move..." << std::endl;

	////createMoveHook = (fnCreateMove)clientModeVMT->HookMethod((DWORD)CreateMove, 24);

	////std::cout << "Hooking override view..." << std::endl;

	////overrideViewHook = (fnOverrideView)clientModeVMT->HookMethod((DWORD)OverrideView, 18);
/*
	std::cout << "Hooking draw model execute..." << std::endl;

	drawModelExecuteHook = (fnDrawModelExecute)mdlRenderVMT->HookMethod((DWORD)DrawModelExecute, 21);*/

	if (paintTraverseHook)	
	{
		std::cout << "Painthook was hooked successfully!" << std::endl;
	}

	//if (createMoveHook)
	//{
	//	std::cout << "Createmove was hooked successfully!" << std::endl;
	//}

	//if (overrideViewHook)
	//{
	//	std::cout << "Overrideview was hooked successfully!" << std::endl;
	//}

	//if (drawModelExecuteHook)
	//{
	//	std::cout << "Drawmodelexecute was hooked successfully!" << std::endl;
	//}
	
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

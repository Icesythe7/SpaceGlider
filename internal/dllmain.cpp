#define WIN32_LEAN_AND_MEAN
#include "Offsets.h"
#include "Decrypt.h"
#include "Utils.h"
#include "Console.h"
#include "UltimateHooks.h"
#include <windows.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <detours.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

struct SpellInfo {

};

typedef HRESULT(WINAPI* Prototype_Present)(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);
typedef HRESULT(WINAPI* Prototype_Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef int(__thiscall* fnOnProcessSpell)(void* spellBook, SpellInfo* spellData);

namespace Functions {
	Prototype_Reset Original_Reset;
	Prototype_Present Original_Present;
	WNDPROC Original_WndProc;
	fnOnProcessSpell OnProcessSpell;
}

LeagueDecrypt rito_nuke;
HMODULE g_module;
Console console;
UltimateHooks ulthook;
PVOID NewOnProcessSpell;

HWND GetHwndProc()
{
	HWND g_hWindow = GetTopWindow(NULL);
	DWORD currentPID = GetCurrentProcessId();
	do
	{
		char title[256];
		if ((GetWindowTextA(g_hWindow, title, 256) > 0) && (IsWindowVisible(g_hWindow)))
		{
			DWORD procId;
			GetWindowThreadProcessId(g_hWindow, &procId);

			if (procId == currentPID)
			{
				return g_hWindow;
			}
		}

		g_hWindow = GetNextWindow(g_hWindow, GW_HWNDNEXT);
	} while (g_hWindow);
	return NULL;
}
DWORD FindDevice(DWORD Len)
{
	DWORD dwObjBase = 0;

	dwObjBase = (DWORD)LoadLibraryA("d3d9.dll");
	while (dwObjBase++ < dwObjBase + Len)
	{
		if ((*(WORD*)(dwObjBase + 0x00)) == 0x06C7
			&& (*(WORD*)(dwObjBase + 0x06)) == 0x8689
			&& (*(WORD*)(dwObjBase + 0x0C)) == 0x8689
			) {
			dwObjBase += 2; break;
		}
	}
	return(dwObjBase);
}
DWORD GetDeviceAddress(int VTableIndex)
{
	PDWORD VTable;
	*(DWORD*)&VTable = *(DWORD*)FindDevice(0x128000);
	return VTable[VTableIndex];
}

HRESULT WINAPI Hooked_Present(LPDIRECT3DDEVICE9 Device, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion)
{
	static bool init = true;
	if (init) {
		init = false;
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplWin32_Init(GetHwndProc());
		ImGui_ImplDX9_Init(Device);
	}
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	console.Render();

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return Functions::Original_Present(Device, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
}
HRESULT WINAPI Hooked_Reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	HRESULT result = Functions::Original_Reset(pDevice, pPresentationParameters);

	if (result >= 0)
		ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}
LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	return CallWindowProcA(Functions::Original_WndProc, hWnd, msg, wParam, lParam);
}
int __fastcall hk_OnProcessSpell(void* spellBook, void* edx, SpellInfo* CastInfo) {
	if (spellBook == nullptr || CastInfo == nullptr)
		return Functions::OnProcessSpell(spellBook, CastInfo);
	console.Print("OnProcessSpell Was Called.");
	return Functions::OnProcessSpell(spellBook, CastInfo);
}

void ApplyHooks() {
	console.Print("Fixing VEH");
	ulthook.RestoreRtlAddVectoredExceptionHandler();
	console.Print("Fixing QueryInformationProcess");
	ulthook.RestoreZwQueryInformationProcess();
	console.Print("Applying Hooks");
	Functions::Original_Present = (Prototype_Present)GetDeviceAddress(17);
	Functions::Original_Reset = (Prototype_Reset)GetDeviceAddress(16);
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)Functions::Original_Present, Hooked_Present);
	DetourAttach(&(PVOID&)Functions::Original_Reset, Hooked_Reset);
	DetourTransactionCommit();
	Functions::Original_WndProc = (WNDPROC)SetWindowLongPtr(GetHwndProc(), GWLP_WNDPROC, (LONG_PTR)WndProc);
#ifndef _DEBUG
	if (rito_nuke.IsMemoryDecrypted((PVOID)DEFINE_RVA(Offsets::Functions::OnProcessSpell))) {

		console.Print("OnProcessSpell was decrypted so, we're going to hook it now");
		DWORD NewOnprocessSpellAddr = ulthook.VirtualAllocateRegion(NewOnProcessSpell, DEFINE_RVA(Offsets::Functions::OnProcessSpell), 0x60);
		ulthook.CopyRegion((DWORD)NewOnProcessSpell, DEFINE_RVA(Offsets::Functions::OnProcessSpell), 0x60);
		ulthook.FixFuncRellocation(DEFINE_RVA(Offsets::Functions::OnProcessSpell), (DEFINE_RVA(Offsets::Functions::OnProcessSpell) + 0x60), (DWORD)NewOnProcessSpell, 0x60);
		Functions::OnProcessSpell = (fnOnProcessSpell)(NewOnprocessSpellAddr);
		bool isOnProcessSpellHooked = ulthook.addHook(DEFINE_RVA(Offsets::Functions::OnProcessSpell), (DWORD)hk_OnProcessSpell, 1);
		if (!isOnProcessSpellHooked)
			console.Print("OnProcessSpell failed to hook Hooked.");
		else
			console.Print("OnProcessSpell Hooked.");
	}

#endif
}

void RemoveHooks() {
	SetWindowLongPtr(GetHwndProc(), GWLP_WNDPROC, (LONG_PTR)Functions::Original_WndProc);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)Functions::Original_Present, Hooked_Present);
	DetourDetach(&(PVOID&)Functions::Original_Reset, Hooked_Reset);
	DetourTransactionCommit();
#ifndef _DEBUG
	// need to unhook onprocessspell.
#endif
}

DWORD WINAPI MainThread(LPVOID param) {
#ifndef _DEBUG
	while (!(*(DWORD*)DEFINE_RVA(Offsets::Data::LocalPlayer)) && *(float*)(DEFINE_RVA(Offsets::Data::GameTime)) < 1)
		Sleep(1);

	Sleep(5000);
#endif
	rito_nuke._RtlDispatchExceptionAddress = rito_nuke.FindRtlDispatchExceptionAddress();
	LeagueDecryptData ldd = rito_nuke.Decrypt(nullptr);

	ApplyHooks();

	while (!(GetAsyncKeyState(VK_END) & 1))
		Sleep(1);

	RemoveHooks();

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	FreeLibraryAndExitThread(g_module, 0);
	return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	g_module = hModule;
	DisableThreadLibraryCalls(hModule);
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, MainThread, hModule, 0, 0);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
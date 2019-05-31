#pragma once
#include "stdafx.h"
#include "globals.h"
using namespace std;
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

typedef UINT(CALLBACK* ViewGetByLastActivationOrderType)(HWND *windows, UINT count, BOOL onlySwitcherWindows, BOOL onlyCurrentDesktop);

HMODULE hVirtualDesktopAccessor;
ViewGetByLastActivationOrderType ViewGetByLastActivationOrder;

class AltTabWindowInfo {
public:
	AltTabWindowInfo(HWND hwnd) {
		this->hwnd = hwnd;
	}

	~AltTabWindowInfo() {
		if (hIcon != 0) {
			DestroyIcon(hIcon);
		}
		return;
	}

	HICON GetIcon() {
		if (hIcon != 0) {
			return hIcon;
		}
		hIcon = LoadIcon(0, IDI_APPLICATION);
		return hIcon;
	}

	wstring GetName() {
		WCHAR get_title[512];
		GetWindowText(this->hwnd, get_title, sizeof(get_title));
		return get_title;
	}

	static vector<shared_ptr<AltTabWindowInfo>> GetAll() {
		if (hVirtualDesktopAccessor == nullptr) {
			hVirtualDesktopAccessor = LoadLibrary(L"C:\\Source\\CandCPP\\VirtualDesktopAccessor\\x64\\Debug\\VirtualDesktopAccessor.dll");
		}
		if (hVirtualDesktopAccessor == nullptr) {
			return {};
		}
		if (ViewGetByLastActivationOrder == nullptr) {
			ViewGetByLastActivationOrder = (ViewGetByLastActivationOrderType)GetProcAddress(hVirtualDesktopAccessor, "ViewGetByLastActivationOrder");
		}
		if (ViewGetByLastActivationOrder == nullptr) {
			return {};
		}

		HWND wnds[1024];
		auto count = ViewGetByLastActivationOrder(wnds, 1024, 1, 1);
		vector<shared_ptr<AltTabWindowInfo>> windowList;
		for (UINT i = 0; i < count; i++)
		{
			windowList.push_back(shared_ptr<AltTabWindowInfo>(new AltTabWindowInfo(wnds[i])));
		}
		return windowList;
	}

	static void Release() {
		FreeLibrary(hVirtualDesktopAccessor);
	}
private:
	HWND hwnd;
	HICON hIcon;
};
#pragma once
#include "stdafx.h"
#include "globals.h"
using namespace std;

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
			hIcon = 0;
		}
		return;
	}

	HICON GetIcon() {
		if (hIcon != nullptr) {
			return hIcon;
		}
		hIcon = LoadIcon(0, IDI_APPLICATION);
		return hIcon;
	}

	wstring GetName() {
		if (name.length() != 0) {
			return name;
		}
		int len = GetWindowTextLength(hwnd) + 1;
		std::vector<wchar_t> buf(len);
		GetWindowText(hwnd, &buf[0], len);
		name = &buf[0];
		return name;
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
			windowList.push_back(make_shared<AltTabWindowInfo>(wnds[i]));
		}
		return windowList;
	}

	static void Release() {
		FreeLibrary(hVirtualDesktopAccessor);
	}
private:
	HWND hwnd = 0;
	HICON hIcon = nullptr;
	wstring name = L"";
};
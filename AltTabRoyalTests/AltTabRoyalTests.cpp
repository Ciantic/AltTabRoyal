// AltTabRoyalTests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AltTabRoyalTests.h"
#include "../AltTabRoyal/AltTabRoyalWindow.h"
#include "../AltTabRoyal/AltTabWindowInfo.h"

HINSTANCE hInst;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst = hInstance;

	// Com initialize
	if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED))) {
		return 1;
	}
	
	// Performance tests
	auto altTabWindows = AltTabWindowInfo::GetAll();
	vector<shared_ptr<AltTabRoyalWindowD2D>> windows;
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindowD2D>(altTabWindows, 0, 0));

	// Cycling
	for (int i = 0; i < 150; i++)
	{
		for (auto &win : windows) {
			win->Select(i);
		}
	}
	windows.clear();


	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
	return (int)msg.wParam;
}



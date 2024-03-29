// AltTabRoyalTests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AltTabRoyalTests.h"
#include "../AltTabRoyal/AltTabRoyalWindow.h"
#include "../AltTabRoyal/AltTabWindowInfo.h"

HINSTANCE hInst;

struct ThreadCreateArgs {
	vector<shared_ptr<AltTabWindowInfo>> altTabWindows;
};

DWORD WINAPI myThread(ThreadCreateArgs* args) {
	args->altTabWindows.size();
	AltTabRoyalWindow royalWindow(0, args->altTabWindows, 0, 0);
	for (int i = 0; i < 150; i++)
	{
		royalWindow.Select(i);
	}
	return 0;
}

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

	// Performance tests, in new threads, we have a winner!
	auto altTabWindows = AltTabWindowInfo::GetAll();
	vector<shared_ptr<AltTabWindowInfo>> windowList;
	for (int i = 0; i < 30; i++)
	{
		windowList.push_back(altTabWindows[1]);
	}
	DWORD threadId = 0;
	ThreadCreateArgs arg = {
		altTabWindows = windowList
	};
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myThread, &arg, 0, &threadId);

	// Performance tests, in same thread, this loses by a big margin!
	/*
	auto altTabWindows = AltTabWindowInfo::GetAll();
	vector<shared_ptr<AltTabWindowInfo>> windowList;
	for (int i = 0; i < 30; i++)
	{
		windowList.push_back(altTabWindows[1]);
	}
	vector<shared_ptr<AltTabRoyalWindow>> windows;
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));
	windows.push_back(make_shared<AltTabRoyalWindow>(windowList, 0, 0));

	// Cycling
	for (int i = 0; i < 150; i++)
	{
		for (auto &win : windows) {
			win->Select(i);
		}
	}
	windows.clear();
	*/

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


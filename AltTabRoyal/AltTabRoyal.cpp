// AltTabRoyal.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "globals.h"
#include "AltTabRoyalController.h"

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

	auto mainController = new AltTabRoyalController();
	mainController->Show();
	mainController->Select(1);
	mainController->Select(2);
	mainController->Select(3);
	mainController->Hide();
	mainController->Show();
	mainController->Select(1);
	mainController->Select(2);
	mainController->Select(3);
	mainController->Hide();
	mainController->Select(1);
	mainController->Select(2);
	mainController->Select(3);
	mainController->Show();
	mainController->Hide();
	mainController->Show();
	mainController->Select(1);
	mainController->Select(2);
	mainController->Select(3);
	//mainController->Hide();

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	delete mainController;
	CoUninitialize();
    return (int) msg.wParam;
}



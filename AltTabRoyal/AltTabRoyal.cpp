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

	// Single instance only, apparently it's not required to release the mutex
	auto singleInstanceMutex = CreateMutex(NULL, TRUE, L"AltTabRoyalMutex");
	if (singleInstanceMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
		// HWND app = FindWindow(AltTabRoyalController::CLASSNAME, 0);
		// if (app) {
		// 	 MessageBox(0, L"Jo", L"Jo", 0);
		// }
		return FALSE;
	}

	hInst = hInstance;

	// Com initialize
	if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED))) {
		return 1;
	}

	auto mainController = new AltTabRoyalController();
#ifdef _DEBUG
	mainController->Show();
#endif

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



// AltTabRoyal.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "globals.h"
#include "AltTabRoyalController.h"
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

HINSTANCE hInst;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst = hInstance;

	// Initiate Gdiplus 
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	auto mainController = new AltTabRoyalController();

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	delete mainController;
	GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
}



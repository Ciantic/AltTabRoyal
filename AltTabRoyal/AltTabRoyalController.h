#pragma once
#include "stdafx.h"
#include "globals.h"
#include "resource.h"
#include "AltTabRoyalWindow.h"
#include "AltTabWindowInfo.h"
using namespace std;
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define WM_SHOW_ALTTABBER WM_USER + 1

class AltTabRoyalController {
public:
	static constexpr const auto CLASSNAME = L"ALT_TAB_ROYAL_CONTROLLER_CLASS";
	AltTabRoyalController() {
		auto szTitle = L"Alt Tab Royal";

		// Register window class
		WNDCLASSEXW wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProcRouter;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BIG));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDC_ALTTABROYALCPP);
		wcex.lpszClassName = CLASSNAME;
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
		ATOM windowAtom = RegisterClassExW(&wcex);

		// Create window
		hwnd = CreateWindowExW(
			WS_EX_APPWINDOW,
			CLASSNAME,
			L"Gui",
			0, // Style
			CW_DEFAULT, // x
			CW_DEFAULT, // y
			CW_DEFAULT, // w
			CW_DEFAULT, // h
			nullptr, // parent
			nullptr, // menu
			hInst,
			nullptr);

		if (hwnd == nullptr)
		{
			throw runtime_error("Unable to create window");
		}

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
		this->Show();
		
		//this->Select(2);
		//this->Select(3);
		//this->Select(4);
		//this->Select(5);
		//this->Select(6);
		//this->Select(7);
		//this->Select(8);
		//this->Select(9);
		for (int i = 0; i < 100; i++)
		{
			this->Select(i);
		}
		/*this->Hide();
		this->Show();
		this->Hide();
		this->Show();
		this->Hide();
		this->Show();
		this->Hide();
		this->Show();*/
	}

	HWND GetHandle() {
		return hwnd;
	}

	void Show() {
		if (windows.size() != 0) {
			return;
		}
		tabWindowInfos = AltTabWindowInfo::GetAll();
		windows.push_back(shared_ptr<AltTabRoyalWindow>(new AltTabRoyalWindow(tabWindowInfos, 1000, 600)));
	}

	void Hide() {
		windows.clear();
		tabWindowInfos.clear();
	}

	void Select(int n) {
		for (auto &win : windows) {
			win->Select(n);
		}
	}
private:
	vector<shared_ptr<AltTabRoyalWindow>> windows;
	vector<shared_ptr<AltTabWindowInfo>> tabWindowInfos;
	HWND hwnd;
	int selected;

	LRESULT CALLBACK WndProc(UINT message, WPARAM wParam, LPARAM lParam) {

		switch (message)
		{
		case WM_SHOW_ALTTABBER:
		{
			this->Show();
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		return 0;
	}

	static LRESULT CALLBACK WndProcRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// This must be handled here, theWindow->WndProc can't handle this
		if (message == WM_NCCREATE) {
			return 1;
		}

		// TODO: Is it possible to get a wrong hWnd here? This could break horribly in that case.
		AltTabRoyalController* theWindow = (AltTabRoyalController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (theWindow != nullptr) {
			return theWindow->WndProc(message, wParam, lParam);
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
};
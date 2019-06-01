#pragma once
#include "stdafx.h"
#include "globals.h"
#include "resource.h"
#include "AltTabRoyalWindow.h"
#include "AltTabWindowInfo.h"

using namespace std;

struct ThreadForAltTabWindowArgs {
	HWND hwnd = 0;
	DWORD threadId = 0;
	HANDLE threadSignal = 0;
	HWND parentHwnd = 0;
	vector<shared_ptr<AltTabWindowInfo>>* tabWindowInfos;
	int midX = 0;
	int midY = 0;
	int selected = 0;
	ThreadForAltTabWindowArgs() {

	}
	ThreadForAltTabWindowArgs(vector<shared_ptr<AltTabWindowInfo>>* tabWindowInfos, int midX, int midY) {
		this->tabWindowInfos = tabWindowInfos;
		this->midX = midX;
		this->midY = midY;
	}
	~ThreadForAltTabWindowArgs() {
		if (threadSignal != 0) {
			/*
			https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-postthreadmessagea
			MSDN
			The thread to which the message is posted must have created a message queue, or else the call to PostThreadMessage fails. Use the following method to handle this situation.

			1. Create an event object, then create the thread.
			2. Use the WaitForSingleObject function to wait for the event to be set to the signaled state before calling PostThreadMessage.
			3. In the thread to which the message will be posted, call PeekMessage as shown here to force the system to create the message queue. `PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE)`
			4. Set the event, to indicate that the thread is ready to receive posted messages.
			*/
			WaitForSingleObject(threadSignal, INFINITE);
			CloseHandle(threadSignal);
			if (!PostThreadMessage(threadId, WM_APP_QUIT, 0, 0)) {
				DWORD err = GetLastError();
				throw runtime_error("This used to work damit: " + to_string(err));
			}
			
		}
	}
};

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
			this);

		if (hwnd == nullptr)
		{
			throw runtime_error("Unable to create window");
		}

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	HWND GetHandle() {
		return hwnd;
	}

	void Show() {
		if (altTabWindowThreads.size() != 0) {
			return;
		}
		tabWindowInfos = AltTabWindowInfo::GetAll();

		// Create windows at the center of each monitor
		auto fn = [this](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor) -> BOOL {
			int midX = lprcMonitor->left + abs(lprcMonitor->right - lprcMonitor->left) / 2;
			int midY = lprcMonitor->top + abs(lprcMonitor->bottom - lprcMonitor->top) / 2;
			this->CreateAltTabRoyalWindow(midX, midY);
			return true;
		};
		auto thunk = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM lParam) -> BOOL {
			return (*reinterpret_cast<decltype(fn)*>(lParam))(hMonitor, hdcMonitor, lprcMonitor);
		};
		EnumDisplayMonitors(NULL, nullptr, thunk, (LPARAM) &fn);
	}

	void Hide() {
		altTabWindowThreads.clear();
		tabWindowInfos.clear();

		// Release some memory
		DWORD pid = GetCurrentProcessId();
		HANDLE ph = OpenProcess(PROCESS_SET_QUOTA, 0, pid);
		SetProcessWorkingSetSize(ph, -1, -1);
		CloseHandle(ph);
	}

	void Select(int n) {
		int size = tabWindowInfos.size();
		if (size != 0) {
			selected = n % size;
		}
		else {
			selected = 0;
		}
		SyncAll();

	}

private:
	map<DWORD, ThreadForAltTabWindowArgs> altTabWindowThreads;
	vector<shared_ptr<AltTabWindowInfo>> tabWindowInfos;
	HWND hwnd;
	int selected;

	static DWORD WINAPI ThreadForAltTabWindow(ThreadForAltTabWindowArgs* args) {
		MSG msg;
		PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
		SetEvent(args->threadSignal);

		AltTabRoyalWindow royalWindow(args->parentHwnd, *args->tabWindowInfos, args->midX, args->midY);

		// Main message loop:
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			if (msg.message == WM_APP_QUIT) {
				PostQuitMessage(0);
			}
			DispatchMessage(&msg);
		}
		return (int)msg.wParam;
	}

	void Sync(DWORD threadId) {
		if (altTabWindowThreads.count(threadId)) {
			PostMessage(altTabWindowThreads[threadId].hwnd, WM_USER_SELECT, selected, 0);
		}
	}

	void SyncAll() {
		for (auto &th : altTabWindowThreads) {
			Sync(th.first);
		}
	}

	void CreateAltTabRoyalWindow(int midX, int midY) {
		auto args = new ThreadForAltTabWindowArgs(&tabWindowInfos, midX, midY);
		DWORD threadId;
		HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadForAltTabWindow, (LPVOID) args, CREATE_SUSPENDED, &threadId);
		args->parentHwnd = hwnd;
		args->threadId = threadId;
		args->threadSignal = CreateEvent(NULL, FALSE, FALSE, NULL);
		altTabWindowThreads[threadId] = *args;
		ResumeThread(thread);
		CloseHandle(thread);
	}

	void AddAltTabRoyalWindow(DWORD threadId, HWND hwnd) {
		if (altTabWindowThreads.count(threadId)) {
			altTabWindowThreads[threadId].hwnd = hwnd;
		}
		Sync(threadId);
	}

	void RemoveAltTabRoyalWindow(DWORD threadId) {
		if (altTabWindowThreads.count(threadId)) {
			altTabWindowThreads[threadId].hwnd = 0;
		}
	}

	static LRESULT CALLBACK WndProcRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// This must be handled here, theWindow->WndProc can't handle this
		// Pass the `this` instance to the GWLP_USERDATA
		if (message == WM_CREATE) {
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pcs->lpCreateParams);
			return 0;
		}

		AltTabRoyalController *theWindow = reinterpret_cast<AltTabRoyalController *>(
			GetWindowLongPtr(hWnd,GWLP_USERDATA)
		);

		if (!theWindow) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		
		switch (message)
		{
			case WM_USER_SHOW:
				theWindow->Show();
				break;
			case WM_USER_HIDE:
				theWindow->Hide();
				break;
			case WM_USER_ROYAL_WINDOW_CREATED:
				theWindow->AddAltTabRoyalWindow(lParam, (HWND) wParam);
				break;
			case WM_USER_ROYAL_WINDOW_DELETED:
				theWindow->RemoveAltTabRoyalWindow(lParam);
				break;
			case WM_USER_SELECT:
				theWindow->Select(wParam);
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;

	}
};

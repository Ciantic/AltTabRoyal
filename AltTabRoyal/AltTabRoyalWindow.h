#pragma once
#include "stdafx.h"
#include "globals.h"
#include "AltTabWindowInfo.h"
using namespace std;
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"shcore.lib")

class AltTabRoyalWindow {
public:
	static constexpr const auto CLASSNAME = L"ALT_TAB_ROYAL_WINDOW_CLASS";
	AltTabRoyalWindow(vector<shared_ptr<AltTabWindowInfo>> windowInfos, int midX, int midY) {
		this->windowInfos = windowInfos;

		// Determine dpiX and dpiY scaler
		auto mon = MonitorFromPoint({ midX, midY }, MONITOR_DEFAULTTONEAREST);
		UINT dpiXv = 96;
		UINT dpiYv = 96;
		GetDpiForMonitor(mon, MDT_EFFECTIVE_DPI, &dpiXv, &dpiYv);
		dpiX = dpiXv / 96.0;
		dpiY = dpiXv / 96.0;

		// Scale the draw parameters
		midX *= dpiX;
		midY *= dpiY;
		windowPadding *= dpiX;
		gridWidth *= dpiX;
		gridHeight *= dpiX;
		contentPadding *= dpiX;
		selectRectPadding *= dpiX;
		iconSize *= dpiX;
		iconPaddingBottom *= dpiY;
		iconPaddingRight *= dpiX;

		this->gridRows = (int) ceil((double) windowInfos.size() / (double) this->gridCols);
		int windowWidth = (this->windowPadding * 2 + this->gridWidth * this->gridCols);
		int windowHeight = (this->windowPadding * 2 + this->gridHeight * this->gridRows);
		int windowX = (midX - (windowWidth / 2)); // DPI
		int windowY = (midY - (windowHeight / 2)); // DPI

		// Register window class
		WNDCLASSEXW wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProcRouter;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = NULL; 
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL; 
		wcex.lpszClassName = CLASSNAME;
		wcex.hIconSm = NULL; 
		ATOM windowAtom = RegisterClassExW(&wcex);

		// Create window
		this->hwnd = CreateWindowExW(
			WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
			CLASSNAME,
			L"Gui",
			0, // Style
			windowX, // x
			windowY, // y
			windowWidth, // w
			windowHeight, // h
			nullptr, // parent
			nullptr, // menu
			hInst,
			nullptr);

		if (!this->hwnd)
		{
			throw runtime_error("Unable to create window");
		}
		SetWindowLongPtr(this->hwnd, GWLP_USERDATA, (LONG_PTR)this);
		PostMessageW(this->hwnd, WM_USER+1, NULL, NULL);

		ShowWindow(this->hwnd, SW_SHOW);
		UpdateWindow(this->hwnd);
	}

	HWND GetHandle() {
		return this->hwnd;
	}

	void Select(int n) {
		this->selected = n % this->windowInfos.size();
		this->PaintLayeredWindow();
	}

	~AltTabRoyalWindow() {
		DestroyWindow(this->hwnd);
	}
private:
	HWND hwnd;
	vector<shared_ptr<AltTabWindowInfo>> windowInfos;
	int selected = 0;
	int gridCols = 5;
	int gridRows = 0;

	// Draw parameters in simulated 96 dpi pixels
	int windowPadding = 15;
	int gridWidth = 250;
	int gridHeight = 250;
	int contentPadding = 15;
	int selectRectPadding = 5;
	int iconSize = 50;
	int iconPaddingRight = 5;
	int iconPaddingBottom = 5;
	
	double dpiX = 1.0;
	double dpiY = 1.0;

	// Peek previews
	// vector<shared_ptr<PeekPreview>> peekPreviews;

	LRESULT CALLBACK WndProc(UINT message, WPARAM wParam, LPARAM lParam) {

		switch (message)
		{
			case WM_USER + 1:
			{
				this->PaintLayeredWindow();
				break;
			}
			case WM_ERASEBKGND:
				break;
			case WM_PAINT:
			{
				/*
				https://microsoft.public.win32.programmer.ui.narkive.com/ngoz2rJn/updatelayeredwindow-and-wm-paint-question
				"In the window proc for a layered window, you should always handle WM_PAINT, call BeginPaint and EndPaint. You don't have to do anything in the middle. If you don't do this, windows will assume that the window is still invalid and pump another WM_PAINT. It does this over and over causing 100% CPU utilization..."
				*/
				PAINTSTRUCT ps;
				BeginPaint(hwnd, &ps);
				EndPaint(hwnd, &ps);
				break;
			}
			case WM_DESTROY:
				//PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(this->hwnd, message, wParam, lParam);
		}
		return 0;
	}

	void InitPaint() {

	}

	void PaintLayeredWindow()
	{
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
		SIZE sizeWindow = { windowRect.right - windowRect.left, windowRect.bottom - windowRect.top };

		HDC hDC = GetDC(hwnd);
		HDC hdcMemory = CreateCompatibleDC(hDC);

		HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC, sizeWindow.cx, sizeWindow.cy);
		HGDIOBJ hMemOldBitmap = SelectObject(hdcMemory, hMemBitmap);

		Graphics graphics(hdcMemory);

		// Background
		static SolidBrush bgBrush(Color(0xbb000000));

		// Text
		static SolidBrush whiteBrush(Color(0xffffffff));
		static FontFamily family(L"Segoe UI");
		static Font font(&family, 8);
		static StringFormat textFormat;

		// Tiles
		static SolidBrush selectedBrush(Color(0xbb00aaff));
		static SolidBrush normalBrush(Color(0xbb000000));


		graphics.FillRectangle(&bgBrush, 0, 0, sizeWindow.cx, sizeWindow.cy);

		int i = -1;
		for (auto &winInfo : this->windowInfos) {
			i++;
			int col = i % gridCols;
			int row = floor((i) / (double)gridCols);
			int x = windowPadding + col * gridWidth;
			int y = windowPadding + row * gridHeight;
			int width = gridWidth;
			int height = gridHeight;
			int selectRectX = x + selectRectPadding;
			int selectRectY = y + selectRectPadding;
			int selectRectWidth = gridWidth - selectRectPadding * 2;
			int selectRectHeight = gridHeight - selectRectPadding * 2;
			int contentX = x + contentPadding;
			int contentY = y + contentPadding;
			int contentWidth = width - contentPadding * 2;
			int contentHeight = height - contentPadding * 2;
			int iconX = contentX;
			int iconY = contentY;
			int iconWidth = iconSize;
			int iconHeight = iconSize;
			int titleX = contentX + iconWidth + iconPaddingRight;
			int titleY = contentY;
			int titleWidth = contentWidth - iconWidth - iconPaddingRight;
			int titleHeight = iconHeight;
			int previewX = contentX;
			int previewY = contentY + iconHeight + iconPaddingBottom;
			int previewWidth = contentWidth;
			int previewHeight = contentHeight - iconHeight - iconPaddingBottom;
			bool isSelected = selected == i;
			
			// Tile
			graphics.FillRectangle(isSelected ? &selectedBrush : &normalBrush, selectRectX, selectRectY, selectRectWidth, selectRectHeight);

			// Text
			graphics.DrawString(winInfo->GetName().c_str(), -1, &font, RectF(titleX, titleY, titleWidth, titleHeight), &textFormat, &whiteBrush);

			// Icon
			graphics.DrawImage(Bitmap::FromHICON(winInfo->GetIcon()), Rect(iconX, iconY, iconWidth, iconHeight));
		}

		// Call UpdateLayeredWindow
		POINT ptWinPos = { windowRect.left, windowRect.top };
		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_ALPHA;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;
		POINT ptSrc = { 0, 0 };
		if (!UpdateLayeredWindow(hwnd, hDC, &ptWinPos, &sizeWindow, hdcMemory, &ptSrc, 0, &blend, ULW_ALPHA)) {
			throw runtime_error("Layered window attributes are not correct, ensure WS_EX_LAYERED is set");
		}

		graphics.ReleaseHDC(hdcMemory);
		SelectObject(hdcMemory, hMemOldBitmap);
		DeleteObject(hMemBitmap);
		DeleteObject(hMemOldBitmap);
		
		ReleaseDC(hwnd, hDC);
		ReleaseDC(hwnd, hdcMemory);
	}

	static LRESULT CALLBACK WndProcRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// This must be handled here, theWindow->WndProc can't handle this
		if (message == WM_NCCREATE) {
			return 1;
		}

		// TODO: Is it possible to get a wrong hWnd here? This could break horribly in that case.
		AltTabRoyalWindow* theWindow = (AltTabRoyalWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (theWindow != nullptr) {
			return theWindow->WndProc(message, wParam, lParam);
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
};
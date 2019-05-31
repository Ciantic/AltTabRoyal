#pragma once
#include "stdafx.h"
#include "globals.h"
#include "AltTabWindowInfo.h"
using namespace std;
using namespace D2D1;

#pragma comment (lib,"shcore.lib")
#pragma comment (lib, "d2d1")
#pragma comment (lib, "dwrite")
#pragma comment (lib, "dwmapi")

// Follows the guide here: https://docs.microsoft.com/en-us/windows/desktop/direct2d/direct2d-quickstart


class AltTabRoyalWindowD2D {
public:
	static constexpr const auto CLASSNAME = L"ALT_TAB_ROYAL_WINDOW_CLASS";
	AltTabRoyalWindowD2D(vector<shared_ptr<AltTabWindowInfo>> windowInfos, int midX, int midY)
	{
		this->windowInfos = windowInfos;

		// Initialize device-indpendent resources, such
		// as the Direct2D factory.
		if (!SUCCEEDED(CreateDeviceIndependentResources())) {
			throw runtime_error("Unable to create device independent resources");
		}

		// Determine dpiX and dpiY scaler
		auto mon = MonitorFromPoint({ midX, midY }, MONITOR_DEFAULTTONEAREST);
		UINT dpiXv = 96;
		UINT dpiYv = 96;
		GetDpiForMonitor(mon, MDT_EFFECTIVE_DPI, &dpiXv, &dpiYv);
		dpiX = dpiXv / 96.0;
		dpiY = dpiXv / 96.0;

		// Scale the draw parameters

		this->gridRows = (int)ceil((double)windowInfos.size() / (double)this->gridCols);
		int windowWidth = (this->windowPadding * 2 + this->gridWidth * this->gridCols);
		int windowHeight = (this->windowPadding * 2 + this->gridHeight * this->gridRows);
		int windowX = (midX - (windowWidth / 2)); 
		int windowY = (midY - (windowHeight / 2));

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
		hwnd = CreateWindowExW(
			WS_EX_COMPOSITED | WS_EX_TOPMOST,
			CLASSNAME,
			L"Gui",
			WS_POPUP, // Style
			windowX * dpiX, // x
			windowY * dpiY, // y
			windowWidth * dpiX, // w
			windowHeight * dpiY, // h
			nullptr, // parent
			nullptr, // menu
			hInst,
			this);

		if (!hwnd)
		{
			throw runtime_error("Unable to create window");
		}

		// Creates DWM surface 
		MARGINS m = { -1 };
		if (!SUCCEEDED(DwmExtendFrameIntoClientArea(hwnd, &m))) {
			throw runtime_error("Unable to extend");
		}

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	~AltTabRoyalWindowD2D() {
		DestroyWindow(hwnd);
		DiscardDeviceResources();
	}

	HWND GetHandle() {
		return hwnd;
	}

	void Select(int n) {
		selected = n % this->windowInfos.size();
		Redraw();
	}

private:
	HWND hwnd = 0;
	CComPtr<IWICImagingFactory> dwicFactory = nullptr;
	CComPtr<IDWriteFactory> dwriteFactory = nullptr;
	CComPtr<ID2D1Factory> d2dFactory = nullptr;
	CComPtr<ID2D1HwndRenderTarget> renderTarget = nullptr;
	CComPtr<ID2D1SolidColorBrush> brushBg = nullptr;
	CComPtr<ID2D1SolidColorBrush> brushSelected = nullptr;
	CComPtr<ID2D1SolidColorBrush> brushNotSelected = nullptr;
	CComPtr<ID2D1SolidColorBrush> brushText = nullptr;

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
	float fontSize = 12.0F;
	const wchar_t* fontFamily = L"Segoe UI";

	double dpiX = 1.0;
	double dpiY = 1.0;

	// Peek previews
	// vector<shared_ptr<PeekPreview>> peekPreviews;


	HRESULT CreateDeviceIndependentResources() {
		CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), reinterpret_cast<LPVOID*>(&dwicFactory));
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwriteFactory));
		return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);
	}

	HRESULT CreateDeviceResources() {
		if (renderTarget)
		{
			return S_OK;
		}

		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);

		// Create a Direct2D render target.
		if (!SUCCEEDED(d2dFactory->CreateHwndRenderTarget(
			RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, 
				PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			HwndRenderTargetProperties(hwnd, size),
			&renderTarget
		))) {
			return 1;
		}

		renderTarget->CreateSolidColorBrush(
			ColorF(0,0,0,0.85F),
			&brushBg
		);
		renderTarget->CreateSolidColorBrush(
			ColorF(0xbb00aaff),
			&brushSelected
		);
		renderTarget->CreateSolidColorBrush(
			ColorF(0xbb000000),
			&brushNotSelected
		);
		renderTarget->CreateSolidColorBrush(
			ColorF(0xffffffff),
			&brushText
		);

		return S_OK;
	}

	// Release device-dependent resource.
	void DiscardDeviceResources() {
		renderTarget = nullptr;
		brushBg = nullptr;
		brushNotSelected = nullptr;
		brushSelected = nullptr;
		brushText = nullptr;
	}

	// Resize the render target.
	void OnResize(
		UINT width,
		UINT height
	) {
		if (renderTarget)
		{
			// Note: This method can fail, but it's okay to ignore the
			// error here, because the error will be returned again
			// the next time EndDraw is called.
			renderTarget->Resize(SizeU(width, height));
		}
	}

	void OnRender()
	{
		if (!SUCCEEDED(CreateDeviceResources())) {
			return;
		}

		// Clear the window
		renderTarget->BeginDraw();
		renderTarget->SetTransform(Matrix3x2F::Identity());
		renderTarget->Clear(ColorF(0,0,0,0));

		// Background
		D2D1_SIZE_F rtSize = renderTarget->GetSize();
		renderTarget->FillRectangle(RectF(0, 0, rtSize.width, rtSize.height), brushBg);

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

			// Draw the tile
			renderTarget->FillRectangle(
				RectF(selectRectX, selectRectY, selectRectX + selectRectWidth, selectRectY + selectRectHeight), 
				isSelected ? brushSelected : brushNotSelected);

			// Draw the icon
			CComPtr<IWICBitmap> bitMap;
			HICON hIcon = winInfo->GetIcon();
			dwicFactory->CreateBitmapFromHICON(hIcon, &bitMap);
			CComPtr<IWICFormatConverter> converter;
			dwicFactory->CreateFormatConverter(&converter);
			converter->Initialize(bitMap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeMedianCut);
			D2D1_BITMAP_PROPERTIES bitmapProps;
			bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
			bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
			bitmapProps.dpiX = dpiX;
			bitmapProps.dpiY = dpiY;
			CComPtr<ID2D1Bitmap> outBitmap;
			renderTarget->CreateBitmapFromWicBitmap(bitMap, bitmapProps, &outBitmap);
			renderTarget->DrawBitmap(outBitmap, RectF(iconX, iconY, iconX + iconWidth, iconY + iconHeight));
			
			// Draw the text
			CComPtr<IDWriteTextLayout> textLayout;
			CComPtr<IDWriteTextFormat> textFormat;
			dwriteFactory->CreateTextFormat(fontFamily,
				NULL,
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				fontSize,
				L"en-us",
				&textFormat
			);
			auto name = winInfo->GetName();
			dwriteFactory->CreateTextLayout(name.c_str(), name.length(), textFormat, titleWidth, titleHeight, &textLayout);
			renderTarget->DrawTextLayout(Point2F(titleX, titleY), textLayout, (ID2D1Brush*) brushText);
		}

		HRESULT endHr = renderTarget->EndDraw();

		if (FAILED(endHr) || endHr == D2DERR_RECREATE_TARGET) {
			DiscardDeviceResources();
		}
	}

	void Redraw() {
		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);
	}

	static LRESULT CALLBACK WndProcRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Pass the `this` instance to the GWLP_USERDATA
		if (message == WM_CREATE) {
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
			SetWindowLongPtr(
				hWnd,
				GWLP_USERDATA,
				(LONG_PTR)pcs->lpCreateParams // this instance
			);
			return 0;
		}

		AltTabRoyalWindowD2D *theWindow = reinterpret_cast<AltTabRoyalWindowD2D *>(
			static_cast<LONG_PTR>(
				GetWindowLongPtr(
					hWnd,
					GWLP_USERDATA
				)));
		if (!theWindow) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		switch (message)
		{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					theWindow->OnResize(width, height);
				}
				break;
			case WM_ERASEBKGND:
				break;
			case WM_DISPLAYCHANGE:
				InvalidateRect(hWnd, NULL, FALSE);
				break;
			case WM_PAINT:
				{
					theWindow->OnRender();
					ValidateRect(hWnd, NULL);
				}
				break;
			case WM_DESTROY:
				//PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}
};
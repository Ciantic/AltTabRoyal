#pragma once

#include "stdafx.h"


class PeekPreview {
public:
	PeekPreview(HWND hwndDest, HWND hwndSource) {
		this->hwndDest = hwndDest;
		this->hwndSource = hwndSource;
	}
	HRESULT Register() {
		return DwmRegisterThumbnail(hwndDest, hwndSource, &thumb);
	}
	HRESULT GetSize(PSIZE size) {
		if (thumb == 0) {
			return 1;
		}
		return DwmQueryThumbnailSourceSize(thumb, size);
	}
	HRESULT SetProperties(const DWM_THUMBNAIL_PROPERTIES* ptnProperties) {
		if (thumb == 0) {
			return 1;
		}
		return DwmUpdateThumbnailProperties(thumb, ptnProperties);
	}
	~PeekPreview() {
		if (thumb != 0) {
			DwmUnregisterThumbnail(thumb);
		}
	}
private:
	HWND hwndDest = nullptr;
	HWND hwndSource = nullptr;
	HTHUMBNAIL thumb = 0;
};
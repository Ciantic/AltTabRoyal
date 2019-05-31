#pragma once

#include "stdafx.h"


class PeekPreview {
public:
	PeekPreview(HWND hwndDest, HWND hwndSource) {
		this->hwndDest = hwndDest;
		this->hwndSource = hwndSource;

	}
private:
	HWND hwndDest = nullptr;
	HWND hwndSource = nullptr;
};
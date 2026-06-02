#include "DX12Context.h"
#include <windows.h>

DX12Context dx12Context;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"DX12EngineWindowClass";

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClassEx(&wc);

	HWND hwnd = CreateWindowExW( // HWND - Handle to Window
								0, CLASS_NAME, L"Graphics Engine",
								WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
								nullptr, nullptr, hInstance, nullptr
	);

	if (hwnd == nullptr) return 0;

	ShowWindow(hwnd, nCmdShow);

	if (!dx12Context.Initialize(hwnd, 1280, 720))
	{
		OutputDebugString(L"Critial error when initializing the DirectX 12.\n");
		return 0;
	}

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else dx12Context.Render();
	}

	dx12Context.Shutdown();
	return 0;
}
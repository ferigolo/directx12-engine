#pragma once
#include <windows.h>

class Input
{
public:
	// The more significant bit tells if a key is physically pressed
	static bool IsKeyDown(int virtualKeyCode) { return (GetAsyncKeyState(virtualKeyCode) & 0x8000) != 0; }
};
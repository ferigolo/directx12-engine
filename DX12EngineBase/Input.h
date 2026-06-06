#pragma once
#include <windows.h>

class Input
{
public:
	static bool IsKeyDown(int virtualKeyCode)
	{
		// The more significant bit tells if a key is physically pressed
		return (GetAsyncKeyState(virtualKeyCode) & 0x8000) != 0;
	}
};
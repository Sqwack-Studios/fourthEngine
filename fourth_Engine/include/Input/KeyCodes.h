#pragma once

//From virtual keys
enum class fthKey
{
	fthKey_A = 0,
	fthKey_B, fthKey_C, fthKey_D, fthKey_E, fthKey_F, fthKey_G, fthKey_H, 
	fthKey_I, fthKey_J, fthKey_K, fthKey_L, fthKey_M, fthKey_N, fthKey_O, 
	fthKey_P, fthKey_Q, fthKey_R, fthKey_S, fthKey_T, fthKey_U, fthKey_V, 
	fthKey_W, fthKey_X, fthKey_Y, fthKey_Z,

	fthKey_0, fthKey_1, fthKey_2, fthKey_3, fthKey_4, fthKey_5, fthKey_6,
	fthKey_7, fthKey_8, fthKey_9,

	fthKey_Left, fthKey_Right, fthKey_Up, fthKey_Down,

	fthKey_PageUp, fthKey_PageDown,

	fthKey_Home, fthKey_End,

	fthKey_Insert, fthKey_Delete,

	fthKey_Back, fthKey_Space,

	fthKey_Enter, fthKey_Escape,

	fthKey_Apostrophe,    // '
	fthKey_Comma,         // ,
	fthKey_Minus,         // -
	fthKey_Plus,          // +
	fthKey_Period,        // .
	fthKey_Slash,         // /
	fthKey_Semicolon,     // ;
	fthKey_LeftBracket,   // [
	fthKey_Backslash,    // \ //
	fthKey_RightBracket,  // ]
	fthKey_GraveAccent,   // `
	fthKey_CapsLock,
	fthKey_ScrollLock,
	fthKey_PrintScreen,
	fthKey_Pause,

	fthKey_KeyPad0, fthKey_KeyPad1, fthKey_KeyPad2, fthKey_KeyPad3, fthKey_KeyPad4, 
	fthKey_KeyPad5, fthKey_KeyPad6, fthKey_KeyPad7, fthKey_KeyPad8, fthKey_KeyPad9,

	fthKey_KeyPadDecimal, fthKey_KeyPadDivide, fthKey_KeyPadMultiply, fthKey_KeyPadSubtract, 
	fthKey_KeyPadAdd, 

	fthKey_Tab,
	fthKey_Shift,
	fthKey_Control,
	NUM

};

enum class MouseKey
{
	LMB = 0,
	RMB,
	MMB,
	NUM
};

enum class KeyState
{
	KS_RELEASED = 0,
	KS_PRESSED,
	KS_DOWN,
	KS_ZERO,
	NUM
};

struct KeyStatePhysical
{
	KeyState state = KeyState::KS_ZERO;
};




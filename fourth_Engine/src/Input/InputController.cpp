#include "pch.h"

#pragma once
#include "include/Input/InputController.h"
#include "include/App.h"

namespace fth
{

	InputController::InputController()
	{
		
		m_Keys.resize(static_cast<size_t>(fthKey::NUM));
		m_MouseKeys.resize(static_cast<size_t>(MouseKey::NUM));
		m_keyboard.resize(static_cast<size_t>(fthKey::NUM));
		m_phyKeyMouse.resize(static_cast<size_t>(MouseKey::NUM));
	}

	void InputController::Update()
	{
		UpdateMouse();
		UpdateKeyboard();
	}
	void InputController::ResetMouseWheel()
	{
		m_Mouse.m_WheelDelta = 0;
	}

	void InputController::ProcessKeyboardInput(WPARAM wParam, LPARAM lParam)
	{

		uint32_t virtualKey{ static_cast<uint32_t>(wParam) };

		fthKey translatedVK{ TranslateVirtualKey(virtualKey) };

		if (translatedVK == fthKey::NUM)
			return;

		KeyStateInternal& keyState { m_Keys[static_cast<size_t>(translatedVK)]};
	
		//bool isToggled = GetKeyState(wParam) & 0x0001;
		bool isDown = GetKeyState(wParam) & 0x8000;
		//bool wasPressed{ ((lParam >> 30) & 1) != 0 };
		//LOG_ENGINE_INFO("", "ISPRESSED: {0}", isDown);
		//LOG_ENGINE_INFO("", "WASPRESSED: {0}", wasPressed);
		//LOG_ENGINE_INFO("", "isTOGGLED: {0}", isToggled);


		keyState.isDown = isDown;
	}
	
	void InputController::ProcessMouseInput(MouseKey key, WPARAM wParam, LPARAM lParam)
	{

		KeyStateInternal& keyRef{ m_MouseKeys[static_cast<size_t>(key)] };
		bool keyPressed{};

		switch (key)
		{
			case MouseKey::LMB:
			{
				keyPressed = ((wParam & MK_LBUTTON) == MK_LBUTTON);
			}
			break;
			case MouseKey::RMB:
			{
				keyPressed = ((wParam & MK_RBUTTON) == MK_RBUTTON);
			}
			break;
			case MouseKey::MMB:
			{
				keyPressed = ((wParam & MK_MBUTTON) == MK_MBUTTON);
			}
			break;
		}

		keyRef.isDown = keyPressed;


	}

	void InputController::UpdateKeyboard()
	{
		uint32_t numKeys{ static_cast<uint32_t>(fthKey::NUM) };

		for (uint32_t keyIdx = 0; keyIdx < numKeys; ++keyIdx)
		{
			KeyStatePhysical& keyState{ m_keyboard[keyIdx] };
			bool isDown{ m_Keys[keyIdx].isDown };


			switch (keyState.state)
			{
			case KeyState::KS_ZERO:
			{
				if (isDown)
					keyState.state = KeyState::KS_PRESSED;
				break;
			}
			case KeyState::KS_PRESSED:
			{
				if (isDown)
					keyState.state = KeyState::KS_DOWN;
				else
					keyState.state = KeyState::KS_RELEASED;
				break;
			}
			case KeyState::KS_DOWN:
			{
				if (isDown)
					break;
				else
					keyState.state = KeyState::KS_RELEASED;
				break;
			}
			case KeyState::KS_RELEASED:
			{
				keyState.state = KeyState::KS_ZERO;
				break;
			}
			}


		}

	}

	void InputController::UpdateMouse()
	{
		uint32_t numKeys{ static_cast<uint32_t>(MouseKey::NUM) };

		for (uint32_t keyIdx = 0; keyIdx < numKeys; ++keyIdx)
		{
			KeyStatePhysical& keyState{ m_phyKeyMouse[keyIdx] };
			bool isDown{ m_MouseKeys[keyIdx].isDown };


			switch (keyState.state)
			{
			case KeyState::KS_ZERO:
			{
				if (isDown)
					keyState.state = KeyState::KS_PRESSED;
				break;
			}
			case KeyState::KS_PRESSED:
			{
				if (isDown)
					keyState.state = KeyState::KS_DOWN;
				else
					keyState.state = KeyState::KS_RELEASED;
				break;
			}
			case KeyState::KS_DOWN:
			{
				if (isDown)
					break;
				else
					keyState.state = KeyState::KS_RELEASED;
				break;
			}
			case KeyState::KS_RELEASED:
			{
				keyState.state = KeyState::KS_ZERO;
				break;
			}
			}

		}


	}

	void InputController::ProcessMousePositionMetrics(uint16_t windowWidth, uint16_t windowHeight, WPARAM wParam, LPARAM lParam)
	{

		MousePos newMousePos;

		MousePos oldMousePos{ m_Mouse.GetMousePos() };
		int newXPos = GET_X_LPARAM(lParam);
		int newYPos = GET_Y_LPARAM(lParam);
		
		
		newMousePos.xPos = newXPos;
		newMousePos.yPos = newYPos;
		

		
		int newDeltaX{ newMousePos.xPos - oldMousePos.xPos };
		int newDeltaY{ newMousePos.yPos - oldMousePos.yPos };
		
		
		
		MouseDelta newMouseDelta{ newDeltaX, newDeltaY };
		MouseDelta newMiddleScreenOffset{ newXPos - windowWidth / 2, -newYPos + windowHeight / 2 };
		
		
		m_Mouse.m_MousePos = newMousePos;
		m_Mouse.m_MouseDelta = newMouseDelta;
		m_Mouse.m_MouseOffsetFromScreenCenter = newMiddleScreenOffset;

	}

	void InputController::ProcessMouseWheel(WPARAM wParam, LPARAM lParam)
	{
		int newMouseWheel{ GET_WHEEL_DELTA_WPARAM(wParam) };
		m_Mouse.m_WheelDelta = newMouseWheel / 120.0f;
	}

	fthKey InputController::TranslateVirtualKey(uint32_t virtualKey)
	{
		if (virtualKey >= 0x41 && virtualKey <= 0x5A) //Alphabet. VK_A = 0x41, VK_Z = 0x5A
			return static_cast<fthKey>(virtualKey - 0x41);
		else if (virtualKey >= 0x30 && virtualKey <= 0x39)
			return static_cast<fthKey>(virtualKey - 0x30 + (uint32_t)fthKey::fthKey_0);
		else 
			switch (virtualKey)
			{
				case VK_LEFT:         return fthKey::fthKey_Left;
				case VK_RIGHT:        return fthKey::fthKey_Right;
				case VK_UP:           return fthKey::fthKey_Up;
				case VK_DOWN:         return fthKey::fthKey_Down;
				case VK_PRIOR:        return fthKey::fthKey_PageUp;
				case VK_NEXT:         return fthKey::fthKey_PageDown;
				case VK_HOME:         return fthKey::fthKey_Home;
				case VK_END:          return fthKey::fthKey_End;
				case VK_INSERT:       return fthKey::fthKey_Insert;
				case VK_DELETE:       return fthKey::fthKey_Delete;
				case VK_BACK:         return fthKey::fthKey_Back;
				case VK_SPACE:        return fthKey::fthKey_Space;
				case VK_RETURN:       return fthKey::fthKey_Enter;
				case VK_ESCAPE:       return fthKey::fthKey_Escape;
				case VK_OEM_7:        return fthKey::fthKey_Apostrophe;
				case VK_OEM_COMMA:    return fthKey::fthKey_Comma;
				case VK_OEM_MINUS:    return fthKey::fthKey_Minus;
				case VK_OEM_PLUS:     return fthKey::fthKey_Plus;
				case VK_OEM_PERIOD:   return fthKey::fthKey_Period;
				case VK_OEM_2:        return fthKey::fthKey_Slash;
				case VK_OEM_1:        return fthKey::fthKey_Semicolon;
				case VK_OEM_4:        return fthKey::fthKey_LeftBracket;
				case VK_OEM_5:        return fthKey::fthKey_Backslash;
				case VK_OEM_6:        return fthKey::fthKey_RightBracket;
				case VK_OEM_3:        return fthKey::fthKey_GraveAccent;
				case VK_CAPITAL:      return fthKey::fthKey_CapsLock;
				case VK_SCROLL:       return fthKey::fthKey_ScrollLock;
				case VK_PRINT:        return fthKey::fthKey_PrintScreen;
				case VK_PAUSE:        return fthKey::fthKey_Pause;
				case VK_NUMPAD0:      return fthKey::fthKey_KeyPad0;
				case VK_NUMPAD1:      return fthKey::fthKey_KeyPad1;
				case VK_NUMPAD2:      return fthKey::fthKey_KeyPad2;
				case VK_NUMPAD3:      return fthKey::fthKey_KeyPad3;
				case VK_NUMPAD4:      return fthKey::fthKey_KeyPad4;
				case VK_NUMPAD5:      return fthKey::fthKey_KeyPad5;
				case VK_NUMPAD6:      return fthKey::fthKey_KeyPad6;
				case VK_NUMPAD7:      return fthKey::fthKey_KeyPad7;
				case VK_NUMPAD8:      return fthKey::fthKey_KeyPad8;
				case VK_NUMPAD9:      return fthKey::fthKey_KeyPad9;
				case VK_DECIMAL:      return fthKey::fthKey_KeyPadDecimal;
				case VK_DIVIDE:       return fthKey::fthKey_KeyPadDivide;
				case VK_MULTIPLY:     return fthKey::fthKey_KeyPadMultiply;
				case VK_SUBTRACT:     return fthKey::fthKey_KeyPadSubtract;
				case VK_ADD:          return fthKey::fthKey_KeyPadAdd;


				case VK_TAB:       return fthKey::fthKey_Tab;
				case VK_SHIFT:     return fthKey::fthKey_Shift;
				case VK_CONTROL:   return fthKey::fthKey_Control;


				default:
					return fthKey::NUM;
			}
	}


}
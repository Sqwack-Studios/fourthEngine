#include "pch.h"

#pragma once
#include "include/Input/Mouse.h"

namespace fth
{
	Mouse::Mouse():
		m_WheelDelta(0)
	{
	}

	void Mouse::UpdateMouse(Mouse& targetMouse, MousePos newPos, MouseDelta newDelta, MouseDelta newMiddleOffset)
	{
		targetMouse.m_MousePos = newPos;
		targetMouse.m_MouseDelta = newDelta;
		targetMouse.m_MouseOffsetFromScreenCenter = newMiddleOffset;
	}
}
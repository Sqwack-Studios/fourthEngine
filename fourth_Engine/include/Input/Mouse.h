#pragma once


namespace fth
{

	struct MousePos
	{
		int xPos;
		int yPos;
	};

	struct MouseDelta
	{
		int deltaX;
		int deltaY;

	};



	class Mouse
	{
	public:
		Mouse();

		const MousePos& GetMousePos() const { return m_MousePos; }
		const MouseDelta& GetMouseDelta() const { return m_MouseDelta; }
		const MouseDelta& GetOffsetFromScreenCenter() const { return m_MouseOffsetFromScreenCenter; }
		const int& GetDeltaWheel() const { return m_WheelDelta; }

		//Not being used rn. Using friend class to update mouse metrics
		static void UpdateMouse(Mouse& targetMouse, MousePos newPos, MouseDelta newDelta, MouseDelta offsetMiddle);

		
	private:
		friend class InputController;



		MousePos      m_MousePos;
		MouseDelta    m_MouseDelta;
		MouseDelta    m_MouseOffsetFromScreenCenter;
		float           m_WheelDelta;

		
	};
}
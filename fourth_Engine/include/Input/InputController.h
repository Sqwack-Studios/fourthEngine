#pragma once
#include <vector>
#include "include/Input/KeyCodes.h"
#include "include/Input/Mouse.h"

namespace fth
{
	class IApp;

	class InputController
	{
	public:

		//We update key states in WndProc
		InputController(const InputController&) = delete;
		InputController& operator=(const InputController&) = delete;

	public:
		static inline InputController& Get()
		{
			static InputController instance;
			return instance;
		};

		void Init() {};
		inline void Shutdown() {};

		void ResetMouseWheel();



		bool KeyIsDown(fthKey key)          { return m_keyboard[static_cast<size_t>(key)].state == KeyState::KS_DOWN; }
		bool KeyIsPressed(fthKey key)       { return m_keyboard[static_cast<size_t>(key)].state == KeyState::KS_PRESSED; }
		bool KeyIsReleased(fthKey key)      { return m_keyboard[static_cast<size_t>(key)].state == KeyState::KS_RELEASED; };
		bool KeyIsDownOrPressed(fthKey key) { return KeyIsDown(key) || KeyIsPressed(key); };


		
		bool KeyIsDown(MouseKey key)          { return m_phyKeyMouse[static_cast<size_t>(key)].state == KeyState::KS_DOWN; };
		bool KeyIsPressed(MouseKey key)       { return m_phyKeyMouse[static_cast<size_t>(key)].state == KeyState::KS_PRESSED; };
		bool KeyIsReleased(MouseKey key)      { return m_phyKeyMouse[static_cast<size_t>(key)].state == KeyState::KS_RELEASED; };
		bool KeyIsDownOrPressed(MouseKey key) { return KeyIsDown(key) || KeyIsPressed(key); };


		void Update();

		const Mouse& GetMouse() const { return m_Mouse; }
	private:

		void UpdateMouse();
		void UpdateKeyboard();

		InputController();


		friend class WindowListener;


		struct KeyStateInternal
		{
			bool isDown = false;
		};



		void ProcessKeyboardInput(WPARAM wParam, LPARAM lParam);
		void ProcessMouseInput(MouseKey key, WPARAM wParam, LPARAM lParam);
		void ProcessMousePositionMetrics(uint16_t windowWidth, uint16_t windowHeight, WPARAM wParam, LPARAM lParam);
		void ProcessMouseWheel(WPARAM wParam, LPARAM lParam);


		fthKey TranslateVirtualKey(uint32_t virtualKey);

	private:

		std::vector<KeyStateInternal>     m_Keys;
		std::vector<KeyStateInternal>     m_MouseKeys;




		std::vector<KeyStatePhysical>    m_keyboard;
		std::vector<KeyStatePhysical>    m_phyKeyMouse;

		Mouse                            m_Mouse;

	};
}
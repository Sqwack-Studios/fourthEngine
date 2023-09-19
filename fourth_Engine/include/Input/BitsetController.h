#pragma once

namespace fth
{
	

	class BitsetController
	{
	public:

		enum MoveDir : uint8_t
		{
			LEFT = 0b00100000,
			RIGHT = 0b00010000,
			UP = 0b00001000,
			DOWN = 0b00000100,
			FORWARD = 0b00000010,
			BACKWARD = 0b00000001,
			UP_LEFT = LEFT | UP,
			UP_RIGHT = RIGHT | UP,
			UP_FORWARD = UP | FORWARD,
			UP_FORWARD_LEFT = UP_FORWARD | LEFT,
			UP_FORWARD_RIGHT = UP_FORWARD | RIGHT,
			UP_BACKWARD = UP | BACKWARD,
			UP_BACKWARD_LEFT = UP_BACKWARD | LEFT,
			UP_BACKWARD_RIGHT = UP_BACKWARD | RIGHT,
			DOWN_LEFT = LEFT | DOWN,
			DOWN_RIGHT = RIGHT | DOWN,
			DOWN_FORWARD = DOWN | FORWARD,
			DOWN_FORWARD_LEFT = DOWN_FORWARD | LEFT,
			DOWN_FORWARD_RIGHT = DOWN_FORWARD | RIGHT,
			DOWN_BACKWARD = DOWN | BACKWARD,
			DOWN_BACKWARD_LEFT = DOWN_BACKWARD | LEFT,
			DOWN_BACKWARD_RIGHT = DOWN_BACKWARD | RIGHT,
			FORWARD_LEFT = FORWARD | LEFT,
			FORWARD_RIGHT = FORWARD | RIGHT,
			BACKWARD_LEFT = BACKWARD | LEFT,
			BACKWARD_RIGHT = BACKWARD | RIGHT,

			ZERO = 0b00000000
		};

		BitsetController() = default;

		inline void SetBitsetMovementDirection(MoveDir direction) { m_Bitset |= direction; }
		inline void UnsetBitsetMovementDirection(MoveDir direction) { m_Bitset &= ~direction; }
		inline const std::bitset<8>& GetBitsetMoveDir() const { return m_Bitset; }

		inline bool CurrentMoveDirectionIsEqualTo(MoveDir moveDir) const { return m_Bitset == moveDir; }

		void  FillSpaceMovementDirection(math::Vector3& outDirection);

	private:

		std::bitset<8>      m_Bitset = MoveDir::ZERO;

	};
}

#include "pch.h"

#pragma once
#include "include/Input/BitsetController.h"
#include "include/Math/MathUtils.h"

namespace fth
{
	
	using namespace math;
	//WS Forward vector
	//AD Right vector
	//Ctrl/Space Up vector

	void BitsetController::FillSpaceMovementDirection(Vector3& outDirection)
	{
		std::bitset<8> direction{ GetBitsetMoveDir() };

		if (direction != BitsetController::MoveDir::ZERO)
		{
			if (direction == BitsetController::MoveDir::LEFT)
			{
				outDirection = Vector3::Left;
			}
			else if (direction == BitsetController::MoveDir::RIGHT)
			{
				outDirection = Vector3::Right;
			}
			else if (direction == BitsetController::MoveDir::UP)
			{
				outDirection = Vector3::Up;

			}
			else if (direction == BitsetController::MoveDir::DOWN)
			{
				outDirection = Vector3::Down;
			}
			else if (direction == BitsetController::MoveDir::FORWARD)
			{
				outDirection = Vector3::Forward;
			}
			else if (direction == BitsetController::MoveDir::BACKWARD)
			{
				outDirection = Vector3::Backward;
			}
			else if (direction == BitsetController::MoveDir::UP_LEFT)
			{
				outDirection = Vector3::Left + Vector3::Up;
			}
			else if (direction == BitsetController::MoveDir::UP_RIGHT)
			{
				outDirection = Vector3::Right + Vector3::Up;
			}
			else if (direction == BitsetController::MoveDir::UP_FORWARD)
			{
				outDirection = Vector3::Up + Vector3::Forward;
			}
			else if (direction == BitsetController::MoveDir::UP_FORWARD_LEFT)
			{
				outDirection = Vector3::Up + Vector3::Forward + Vector3::Left;
			}
			else if (direction == BitsetController::MoveDir::UP_FORWARD_RIGHT)
			{
				outDirection = Vector3::Up + Vector3::Forward + Vector3::Right;
			}
			else if (direction == BitsetController::MoveDir::UP_BACKWARD)
			{
				outDirection = Vector3::Up + Vector3::Backward;
			}
			else if (direction == BitsetController::MoveDir::UP_BACKWARD_LEFT)
			{
				outDirection = Vector3::Up + Vector3::Backward + Vector3::Left;
			}
			else if (direction == BitsetController::MoveDir::UP_BACKWARD_RIGHT)
			{
				outDirection = Vector3::Up + Vector3::Backward + Vector3::Right;
			}
			else if (direction == BitsetController::MoveDir::DOWN_LEFT)
			{
				outDirection = Vector3::Left + Vector3::Down;
			}
			else if (direction == BitsetController::MoveDir::DOWN_RIGHT)
			{
				outDirection = Vector3::Right + Vector3::Down;
			}
			else if (direction == BitsetController::MoveDir::DOWN_FORWARD)
			{
				outDirection = Vector3::Down + Vector3::Forward;
			}
			else if (direction == BitsetController::MoveDir::DOWN_FORWARD_LEFT)
			{
				outDirection = Vector3::Down + Vector3::Forward + Vector3::Left;
			}
			else if (direction == BitsetController::MoveDir::DOWN_FORWARD_RIGHT)
			{
				outDirection = Vector3::Down + Vector3::Forward + Vector3::Right;
			}
			else if (direction == BitsetController::MoveDir::DOWN_BACKWARD)
			{
				outDirection = Vector3::Down + Vector3::Backward;
			}
			else if (direction == BitsetController::MoveDir::DOWN_BACKWARD_LEFT)
			{
				outDirection = Vector3::Down + Vector3::Backward + Vector3::Left;
			}
			else if (direction == BitsetController::MoveDir::DOWN_BACKWARD_RIGHT)
			{
				outDirection = Vector3::Down + Vector3::Backward + Vector3::Right;
			}
			else if (direction == BitsetController::MoveDir::FORWARD_LEFT)
			{
				outDirection = Vector3::Forward + Vector3::Left;
			}
			else if (direction == BitsetController::MoveDir::FORWARD_RIGHT)
			{
				outDirection = Vector3::Forward + Vector3::Right;
			}
			else if (direction == BitsetController::MoveDir::BACKWARD_LEFT)
			{
				outDirection = Vector3::Backward + Vector3::Left;
			}
			else if (direction == BitsetController::MoveDir::BACKWARD_RIGHT)
			{
				outDirection = Vector3::Backward + Vector3::Right;
			}
			/*outDirection.Normalize();*/
			
		}
		else
			outDirection = Vector3::Zero;
	}
}
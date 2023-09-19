#pragma once
#include "include/Systems/TransformSystem.h"


namespace fth
{
	struct BaseID;
	using TransformID = BaseID;

	struct IDragger
	{
		virtual void drag(const math::Vector3& offset) = 0;
	};

	struct ModelDragger : public IDragger
	{
		ModelDragger(Handle<math::Matrix> target):
			target(target) {}

		virtual void drag(const math::Vector3& offset);

		Handle<math::Matrix> target;
	};


}

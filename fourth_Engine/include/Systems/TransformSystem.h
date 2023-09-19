#pragma once
#include "include/Utils/SolidVector.h"


namespace fth
{
	class TransformSystem
	{
	public:
		static constexpr uint32_t INITIAL_POOL = 100;


		static TransformSystem& Get()
		{
			static TransformSystem instance;
			return instance;
		}
		void Init();
		void Shutdown();

		Handle<math::Matrix> AddTransform(const math::Matrix& transformMatrix);
		Handle<math::Matrix> AddTransform(const math::Transform& transform); 
		const math::Matrix& QueryTransformMatrix(Handle<math::Matrix>  id) const;
		math::Matrix& QueryTransformMatrix(Handle<math::Matrix>  id);


		void EraseTransform(Handle<math::Matrix> id);
	private:
		TransformSystem();
		SolidVector<math::Matrix>  m_transforms;

	};

}


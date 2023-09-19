#pragma once
#include <fourthE.h>

class IncinerationInstance
{
public:
	IncinerationInstance(const fth::shading::IncinerationGroup::InstanceData& data, float maxRange) : isUpdated(true), maxRange(maxRange), data(data){};
	IncinerationInstance() = default;


	bool HasFinishedAnimation();
	void Destroy();
	fth::Handle<fth::math::Matrix> getTransformHandle();
	fth::Handle<fth::ModelInstance> getModelInstanceHandle();
	fth::Handle<fth::RenderableObject> getRenderableObjectHandle();


	bool isUpdated;
private:
	void updateData();
	float maxRange;
	fth::shading::IncinerationGroup::InstanceData data;
};
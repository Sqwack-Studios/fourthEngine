#pragma once
#include <fourthE.h>

struct DissolutionInstance
{
public:

	DissolutionInstance(const fth::shading::DissolutionGroup::InstanceData& data) : instanceData(data), dataIsUpdated(true) {};
	DissolutionInstance() = default;
	//void ModifyTransformID...
	bool HasFinishedAnimation();
	void Destroy();
	fth::Handle<fth::math::Matrix> getTransformHandle();
	fth::Handle<fth::ModelInstance> getModelInstanceHandle();
	fth::Handle<fth::RenderableObject> getRenderableObjectHandle();

public:
	bool dataIsUpdated;
private:
	void updateData();
	//I could cache here all variables as a reflection of the internal instance representation in DissolutionGroup. If any of these fields is modified here, it will be updated in DissolutionGroup. I don't know. Need guidance.
	fth::shading::DissolutionGroup::InstanceData instanceData;
};

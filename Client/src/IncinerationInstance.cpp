#pragma once
#include "include/IncinerationInstance.h"

using namespace fth;

bool IncinerationInstance::HasFinishedAnimation()
{
	if (!isUpdated)
		updateData();
	return data.radius > maxRange;
}

void IncinerationInstance::Destroy()
{
	MeshSystem::Get().deleteInstance(data.handle_modelInstance);
}

fth::Handle<fth::math::Matrix> IncinerationInstance::getTransformHandle()
{
	if (!isUpdated)
		updateData();
	return data.handle_modelToWorld;
}

fth::Handle<fth::ModelInstance> IncinerationInstance::getModelInstanceHandle()
{
	if (!isUpdated)
		updateData();
	return data.handle_modelInstance;
}

fth::Handle<fth::RenderableObject> IncinerationInstance::getRenderableObjectHandle()
{
	if (!isUpdated)
		updateData();

	return data.handle_object;
}

void IncinerationInstance::updateData() 
{
	data = MeshSystem::Get().getIncinerationInstanceData(data.handle_modelInstance);
}

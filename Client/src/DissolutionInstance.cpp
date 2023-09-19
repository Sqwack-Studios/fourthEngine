#pragma once
#include "include/DissolutionInstance.h"

using namespace fth;




bool DissolutionInstance::HasFinishedAnimation()
{
	updateData();

	return instanceData.animationTime > instanceData.animationDuration;
}

fth::Handle<fth::math::Matrix>     DissolutionInstance::getTransformHandle()  
{ 
	updateData();

	return instanceData.handle_modelToWorld;
}
fth::Handle<fth::ModelInstance>    DissolutionInstance::getModelInstanceHandle() 
{ 
	updateData();
	return instanceData.handle_modelInstance; 
}
fth::Handle<fth::RenderableObject> DissolutionInstance::getRenderableObjectHandle() 
{ 
	updateData();
	return instanceData.handle_object;
}

void DissolutionInstance::updateData()
{
	if (!dataIsUpdated)
	{
		instanceData = MeshSystem::Get().getDissolutionInstanceData(instanceData.handle_modelInstance);
	}
}

void DissolutionInstance::Destroy()
{
	MeshSystem::Get().deleteInstance(instanceData.handle_modelInstance);
}
#pragma once

//High priority
//TODO: write CAMER_CENTER_WS for remaining shaders
//TODO: Shading groups can also take transform ID. A transform might be shared between other systems, so it makes sense to create the transform outside mesh 
// system, so that other systems can also be fed from the same ID.
//TODO: Camera update settings after resize

//Medium priority
//TODO: Check blending for the grid (look at the sphere!)
//TODO: Rework Texture structures to have CPU side Image data and another structure to promote it to GPU bindable
//TODO: Rework DXGI debug filters
//Low priority
//TODO: Send array of states ptr and fill it in depth stencil states 
//TODO: Send array of states ptr and fill it in renderer states /
//TODO: Check models AABB 
// 
//TODO: Add ImGUI layer
//TODO: Add multiple cameras. Switch between them with keys
//TODO: Add camera frustum gizmos
//TODO: Add gestures, i.e. combination of keys to create some behaviours. LMB moves the camera, but Ctrl + LMB selects a mesh, for example.
//TODO: Add accelerator data structure like we did in mesh intersections, but for models in space. Like, divide space in sectors and based on looking direction, search in those sectors
//TODO: Add multiple inputs to logging assertions->look at SolidVector.h


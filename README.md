fourthEngine
==================================

fourthEngine is a pet engine made from scratch as a personal project to learn programming in C++, computer graphics and engine architecture. It is made using Win32 API and DirectX11. The engine is a library by itself. However, an "EngineApp" class is provided in the engine, which can be inherited to create your own application as I did with the "Client" project which serves as a small showcase of the engine.

### Features ###
- Logging system using spdlog
- PBR rendering using Lambert-CookTorrance BRDF
- Image Based Lightning (IBL). A tool is provided to build diffuse and specular irradiance maps and reflectance LUTs. 
- HDR pipeline
- EV100 brightness control, gamma correction, ACES tonemapping
- Shadow mapping: ortographical and perspective. No cascades.
- Deferred pipeline
- Support for decals
- Support for spherical lights, directional and spotlights. Spotlights can be masked with textures.
- Support for 6-way lightmapped particles
- Support for GPU particle system, screen space collisions
- Opaque and translucent rendering
- Spawn and despawn of geometry
- Static shading groups
- Bloom postprocessing stage
- FXAA
- Texture loading and export
- Model (FBX supported) load and export
- Normal and wireframe debugging tools
- LUMA (luminance) debugging tool


### Controls ###

- WASD to move
- LMB to rotate the camera.
- RMB and hold to drag any element
- Q/E to move up/down to your relative UP vector
- K to enter wireframe mode.
- V to enter LUMA debugging mode
- M to spawn a model
- G to spawn a decal.
- DEL to delete a model. Doesn't delete lights yet.

### How to build ###

Clone the repo and run "GenerateProject.bat" in the root of the project. Open the solution and compile the project. SPDLOG is crashing the compiler using VisualStudio 17.7.
NOTE: IBLGenerator project is non-compilant right now.

### Configurations ###
->Debug: No optimizations, debug symbols, logging
->Release: Optimizations turned on, debug symbols, logging
->Dist: Optimizations on, no debug symbols, no logging (not working)


MODIFICATIONS MADE TO DirectX::SimpleMath library:

->Every single structure has default value initialization, which is weird considering that the underlying DirectXMath uses default constructors
I've added a macro called DEFAULT_CONSTRUCTORS, which replaces default value constructors to structName () noexcept = default

->This is also extended now to DirectXCollisions

->Now, SimpleMath.h is using my own DirectXCollisions.h versions instead of the SDK

->SimpleMath is right handed, so I modified default Vector3 const Forward and Backward values to match our convention. In any case, when operating
with Matrices, use DirectXMath LH functions.

->Moved Ray-Plane intersection to Plane class, it does not make sense that every single primitive class like box or sphere have the intersection
implementation in their own class, but Ray-Plane intersection was implemented in Ray 
->Modified Ray-Triangle intersection algorithm to give information about which face has been intersected. (DirectXCollision.h and DirectXCollision.inl)

![alt text](docs/showcase4)
![alt text](docs/bloom)
![alt text](docs/6wayParticles)
![alt text](docs/gpuParticleSys)
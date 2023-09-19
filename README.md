Notes:


Configurations:

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
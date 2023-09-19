#include <fourthE.h>

/// TODO: everything
/// Commands
/// 
/// -GGX_sRef        compute GGX specular reflectance -> you can use this without input filename
/// 
///
/// 
/// -i  <filename>   input filename
/// -dIrr            compute diffuse irradiance        
/// -sIrr            compute specular irradiance
/// -w  <N>          output  width
/// -h  <N>          output  height
/// -m               if you want to generate mips
/// -f  <format>     output format
/// -o  <filename>   output filename. If not supplied it will be input filename +"_<IBL command>"
/// 
/// 
/// 
using namespace fth;

int main(int argc, char* argv[])
{
	fth::Init();
	fth::ibl::IBLReflectionCapture::Get().Init();



	//XD
	uint32_t samples{ 5000 };
	std::shared_ptr<Texture> src = TextureManager::Get().LoadCubemaps("Textures/Cubemaps/grass_field.dds");

	Texture testCubemap;
	{
		uint32_t width{ 8 };
		uint32_t height{ 8 };
		ibl::IBLReflectionCapture::Get().GenerateCubemapDiffuseIrradiance(
			*src,
			"../../../Assets/Textures/Cubemaps/grass_field_diffuseIrradiance.dds",
			width,
			height,
			samples,
			testCubemap
		);
	}

	ibl::IBLReflectionCapture::Get().GenerateCubemapSpecularIrradiance(
		*src,
		"../../../Assets/Textures/Cubemaps/grass_field_specularIrradiance.dds",
		src->GetWidth(),
		src->GetHeight(),
		samples,
		testCubemap);


	ibl::IBLReflectionCapture::Get().GenerateGGXSpecularReflectanceLUT(
		src->GetWidth(),
		"../../../Assets/Textures/Cubemaps/grass_field_specularReflectance.dds",
		src->GetHeight(),
		samples,
		testCubemap);

	fth::Shutdown();

	return 0;
}
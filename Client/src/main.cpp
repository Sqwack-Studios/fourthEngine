#include <fourthE.h>
#include <iomanip>

extern std::unique_ptr<fth::EngineApp> fth::CreateApp(const fth::EngineAppSpecs& specs);

fth::math::Vector3 randomFibonacci(uint32_t i, uint32_t N, float& outCos);
float computeIntegral(uint32_t N);
void createConsole();


constexpr float SQRT_5 = 2.236067977f;
constexpr float PI = 3.141592654f;




int WINAPI WinMain(_In_ HINSTANCE appHandle, _In_opt_ HINSTANCE, _In_ LPSTR cmdLine, _In_ int windowShowParams)
{

	createConsole();

	uint32_t samples{ 100000 };
	//I'm used to theta as zenital angle, and phi as azimutal
	std::cout << "Integral of cos(theta) over hemisphere is: " << std::setprecision(10) << computeIntegral(samples) << std::endl;

	fth::EngineAppSpecs appSpecs{};
	auto app = fth::CreateApp(appSpecs);
	app->Init();


	fth::WindowSpecs winSpecs;
	winSpecs.windowWidth = appSpecs.windowWidth;
	winSpecs.windowHeight = appSpecs.windowHeight;
	winSpecs.backBufferSizeMultiplier = appSpecs.backBufferSizeMultiplier;
	winSpecs.fullscreen = appSpecs.fullscreen;

	std::unique_ptr<fth::Win32Window> window = std::make_unique<fth::Win32Window>(winSpecs);
	window->Init();
	app->AttachWindow(std::move(window));

	app->Run(app.get());
	app->Shutdown();
	

	return 0;
}


fth::math::Vector3 randomFibonacci(uint32_t i, uint32_t N, float& outCos)
{
	constexpr float GOLDEN_RATIO = (1.0f + SQRT_5) / 2.0f;


	float theta = 2.0f * PI * i / GOLDEN_RATIO;
	float phiCos = 1.0f - (i + 0.5f) / N;
	float phiSin = std::sqrt(1.0f - phiCos * phiCos);
	float thetaCos = std::cos(theta);
	float thetaSin = std::sin(theta);

	outCos = phiCos;


	return { thetaCos * thetaSin, thetaSin * phiSin, phiCos };
}

float computeIntegral(uint32_t  N)
{
	float integral{};
	for (uint32_t i = 0; i < N; ++i)
	{
		float cos;

		randomFibonacci(i, N, cos);
		integral += cos;
	}

	return (integral * 2.0f * PI) / static_cast<float>(N);
}

void createConsole()
{
	::AllocConsole();
	static FILE* dummy;
	auto s = freopen_s(&dummy, "CONOUT$", "w", stdout);// stdout will print to the newly created console
	auto sin = freopen_s(&dummy, "CONIN$", "r", stdin);
	if (s || sin)
		BREAK_NOLOG;

	std::cout.clear();
	std::cin.clear();
}
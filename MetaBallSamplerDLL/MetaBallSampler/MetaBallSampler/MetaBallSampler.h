// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the METABALLSAMPLER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// METABALLSAMPLER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef METABALLSAMPLER_EXPORTS
#define METABALLSAMPLER_API __declspec(dllexport)
#else
#define METABALLSAMPLER_API __declspec(dllimport)
#endif

class Ball {
public:
	int x;
	int y;
	float r;

	float getInfluenceTo(int pX, int pY);
};

extern "C" {
	METABALLSAMPLER_API char* getColorData(
		Ball** balls,
		int ballCount,
		float border,
		char* lightColor,
		char* darkColor,
		int width,
		int height,
		int& size
	);

	METABALLSAMPLER_API char* getColorDataMultiThreaded(
		Ball** balls,
		int ballCount,
		float border,
		char* lightColor,
		char* darkColor,
		int width,
		int height,
		int& size
	);

	METABALLSAMPLER_API void freeMemory(char* dataPtr);

	METABALLSAMPLER_API Ball* createBall(int x, int y, float r);	

	using LogCallback = void(const char*);
	LogCallback* registeredLogCallback = nullptr;
	METABALLSAMPLER_API void registerLogCallback(LogCallback* logCallback);
}

void Log(const char* message);

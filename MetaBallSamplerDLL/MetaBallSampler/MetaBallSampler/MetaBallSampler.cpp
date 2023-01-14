// MetaBallSampler.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "MetaBallSampler.h"
#include "Math.h"
#include "Thread"
#include "String"
using namespace std;


METABALLSAMPLER_API char* getColorData(Ball** balls, int ballCount, float border, char* lightColor, char* darkColor, int width,int height, int& size) {
	size = width * height * 3;
	char* data = new char[size];
	int i;
	float influence;
	char* col;

	//for all pixels
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			//for all balls
			influence = 0;
			for (int b = 0; b < ballCount; b++) {
				influence += balls[b]->getInfluenceTo(x,y);
			}
			col = influence >= border ? lightColor : darkColor;

			i = (x * width + y) * 3;
			data[i] = col[0];
			data[i + 1] = col[1];
			data[i + 2] = col[2];
		}
	}

	return data;
}

METABALLSAMPLER_API char* getColorDataMultiThreaded(Ball** balls, int ballCount, float border, char* lightColor, char* darkColor, int width, int height, int& size) {
	size = width * height * 3;
	char* data = new char[size];
	const int cores = std::thread::hardware_concurrency();

	//define lambda func
	auto sampleSeq = [&](const int startX,const int endX) {
		int i;
		float influence;
		char* col;

		for (int x = startX; x < endX; x++) {
			for (int y = 0; y < height; y++) {
				//for all balls
				influence = 0;
				for (int b = 0; b < ballCount; b++) {
					influence += balls[b]->getInfluenceTo(x, y);
				}
				col = influence >= border ? lightColor : darkColor;

				i = (x * width + y) * 3;
				data[i] = col[0];
				data[i + 1] = col[1];
				data[i + 2] = col[2];
			}
		}
	};
	
	//start threads
	thread* threads = new thread[cores];
	const int threadSize = width / cores + 1;
	for (int i = 0; i < cores; i++) {
		int startX = i * threadSize;
		int endX = startX + threadSize;
		if (endX > width)endX = width;

		threads[i] = thread(sampleSeq,startX,endX);
	}

	//wait threads finish
	for (int i = 0; i < cores; i++) {
		threads[i].join();

	}

	//deallocate memory
	delete[] threads;

	return data;
}

METABALLSAMPLER_API void freeMemory(char* dataPtr) {
	delete[] dataPtr;
}

METABALLSAMPLER_API Ball* createBall(int x, int y, float r) {
	Ball* ball = new Ball();
	ball->r = r;
	ball->x = x;
	ball->y = y;

	
	return ball;
}

float Ball::getInfluenceTo(int pX, int pY) {
	if (pX == x && pY == y) {
		return r;
	}
	else {
		return 1.f / (pow((x - pX) / r, 2) + pow((y - pY) / r, 2));
	}
}

METABALLSAMPLER_API void registerLogCallback(LogCallback* logCallback) {
	registeredLogCallback = logCallback;
	Log("Callback Registered!");
}

void Log(const char* message) {
	if (registeredLogCallback != nullptr) {
		registeredLogCallback(message);
	}
}

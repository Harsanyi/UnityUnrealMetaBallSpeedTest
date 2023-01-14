// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "TestGameMode.h"
#include "TestUI.h"
#include "TestSettings.h"
#include "MemoryTexture.h"
#include "MetaBall.h"
#include "LogPanel.h"
#include "Thread"
#include "MetaBallShaderCS.h"

using namespace std;

void ATestGameMode::StartPlay() {
	Super::StartPlay();

	TestGameMode = Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;

	if (!TestSettings->TryLoadSettings()) {
		TestSettings->SaveSettings();
	}

	UIInstance = CreateWidget<UTestUI>(GetWorld(), StartUIClass);
	UIInstance->AddToViewport();
	UIInstance->SetSettings(TestSettings);
	UIInstance->OnStartTests.AddUniqueDynamic(this, &ATestGameMode::StartTesting);

	LogPanel = CreateWidget<ULogPanel>(GetWorld(), LogPanelClass);
	LogPanel->AddToViewport();

	GenerateBaseTexture();

	UE_LOG(LogTemp, Warning, TEXT("Game intited!"));
}

void ATestGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TestSettings->SaveSettings();
}

ATestGameMode::ATestGameMode() {
	if (TestSettings == nullptr) {
		TestSettings = CreateDefaultSubobject<UTestSettings>(TEXT("Settings"));
	}
}

void ATestGameMode::SettingsChanged() {
	UE_LOG(LogTemp,Warning,TEXT("Settings Changed!"));
}

void ATestGameMode::StartTesting()
{
	OnStartTesting.Broadcast();

	FString message = "----[Start Testing]----";
	LogPanel->Log(message);

	//Create texture
	if (MemoryTexture == nullptr || MemoryTexture->GetWidth() != TestSettings->Width || MemoryTexture->GetHeight() != TestSettings->Height) {
		GenerateBaseTexture();
	}

	//Generate Balls
	GenerateMetaBalls();

	double TestStart;
	double TestEnd;

	//Single
	if (TestSettings->SingleThread) {		
		TestStart = FPlatformTime::Seconds();

		for (int i = 0; i < TestSettings->RunCount; i++) {
			SingleThreadUpdate();
		}

		TestEnd = FPlatformTime::Seconds();

		message = FString::Printf(TEXT("Single Thread Finished: %f ms"), (TestEnd-TestStart)/TestSettings->RunCount*1000);
		LogPanel->Log(message);
	}

	//Multi
	if (TestSettings->MultiThread) {
		TestStart = FPlatformTime::Seconds();

		for (int i = 0; i < TestSettings->RunCount; i++) {
			MultiThreadUpdate();
		}

		TestEnd = FPlatformTime::Seconds();

		message = FString::Printf(TEXT("MultiThread Finished: %f ms"), (TestEnd - TestStart) / TestSettings->RunCount * 1000);
		LogPanel->Log(message);
	}

	//Blueprint
	if (TestSettings->Blueprint) {
		TestStart = FPlatformTime::Seconds();

		for (int i = 0; i < TestSettings->RunCount; i++) {
			OnStartBlueprintFill(MemoryTexture, TestSettings, MetaBalls);
		}

		TestEnd = FPlatformTime::Seconds();

		message = FString::Printf(TEXT("Blueprint Finished: %f ms"), (TestEnd - TestStart) / TestSettings->RunCount * 1000);
		LogPanel->Log(message);
	}

	//ComputeShader
	if (TestSettings->ComputeShader) {
		TestStart = FPlatformTime::Seconds();

		for (int i = 0; i < TestSettings->RunCount; i++) {
			ComputeShaderUpdate();
		}

		TestEnd = FPlatformTime::Seconds();

		message = FString::Printf(TEXT("Compute Shader Finished: %f ms"), (TestEnd - TestStart) / TestSettings->RunCount * 1000);
		LogPanel->Log(message);
	}

	message = "----[Testing Finished]----";
	LogPanel->Log(message);

	OnEndTesting.Broadcast();
}

void ATestGameMode::SingleThreadUpdate()
{
	float Influence;
	for (int x = 0; x < MemoryTexture->GetWidth(); x++) {
		for (int y = 0; y < MemoryTexture->GetHeight(); y++) {

			Influence = 0;
			for (int b = 0; b < MetaBalls.Num(); b++) {
				Influence += MetaBalls[b]->GetInfluenceTo(x,y);
			}

			MemoryTexture->SetPixel(Influence > TestSettings->Border ? TestSettings->LightColor : TestSettings->DarkColor, x, y);
		}
	}
	MemoryTexture->Apply();
}

void ATestGameMode::GenerateBaseTexture()
{
	if (MemoryTexture != nullptr) {
		MemoryTexture->ConditionalBeginDestroy();
	}

	FString message = FString::Printf(TEXT("Generate texture %d x %d"), TestSettings->Width, TestSettings->Height);
	LogPanel->Log(message);

	MemoryTexture = NewObject<UMemoryTexture>();
	MemoryTexture->Init(TestSettings->Width, TestSettings->Height);
	MemoryTexture->Fill(DefaultColor);
	MemoryTexture->Apply();
	UIInstance->SetTexture(MemoryTexture->GetTexture());
}

void ATestGameMode::GenerateMetaBalls()
{
	if (MetaBalls.Num() > 0) {
		MetaBalls.Empty();
	}

	UMetaBall* curBall;
	for (int i = 0; i < TestSettings->BallCount; i++) {
		curBall = NewObject<UMetaBall>();
		curBall->Init(
			FMath::RandRange(0, TestSettings->Width),
			FMath::RandRange(0, TestSettings->Height),
			FMath::RandRange(TestSettings->MinBallRadious, TestSettings->MaxBallRadious)
		);
		MetaBalls.Add(curBall);
	}
}

void ATestGameMode::MultiThreadUpdate()
{
	const int CoreNum = std::thread::hardware_concurrency();
	const int ThreadSize = TestSettings->Width / CoreNum + 1;
	thread* Threads = new thread[CoreNum];

	auto ThreadTask = [&](const int StartX, const int EndX) {
		float Influence;
		for (int x = StartX; x < EndX; x++) {
			for (int y = 0; y < MemoryTexture->GetHeight(); y++) {

				Influence = 0;
				for (int b = 0; b < MetaBalls.Num(); b++) {
					Influence += MetaBalls[b]->GetInfluenceTo(x, y);
				}

				MemoryTexture->SetPixel(Influence > TestSettings->Border ? TestSettings->LightColor : TestSettings->DarkColor, x, y);
			}
		}
	};

	for (int t = 0; t < CoreNum; t++) {
		const int StartX = t * ThreadSize;
		const int EndX = StartX + ThreadSize > TestSettings->Width ? TestSettings->Width : StartX + ThreadSize;
		Threads[t] = thread(ThreadTask, StartX, EndX);
	}

	for (int t = 0; t < CoreNum; t++) {
		Threads[t].join();
	}

	delete[] Threads;

	MemoryTexture->Apply();
}

void ATestGameMode::ComputeShaderUpdate() {
	bool done = false;
	uint8* result;

	UE_LOG(LogTemp,Warning,TEXT("Dispatch started on thread:%d"), FPlatformTLS::GetCurrentThreadId());
	
	TResourceArray<int> BallX = TResourceArray<int>();
	TResourceArray<int> BallY = TResourceArray<int>();
	TResourceArray<float> BallR = TResourceArray<float>();
	for (int i = 0; i < MetaBalls.Num(); i++) {
		BallX.Add(MetaBalls[i]->X);
		BallY.Add(MetaBalls[i]->Y);
		BallR.Add(MetaBalls[i]->R);
	}

	FMetaBallShaderCS::Dispatch(done, result, BallX, BallY, BallR, TestSettings->Width, TestSettings->Height, TestSettings->Border, TestSettings->LightColor, TestSettings->DarkColor);

	int cicle = 0;
	while (!done && cicle++<1000 ) {
		FPlatformProcess::Sleep(0.001f);
	}

	MemoryTexture->SetRawPixels(result);
	MemoryTexture->Apply();

	UE_LOG(LogTemp, Warning, TEXT("Compute Shader Returend."));
}


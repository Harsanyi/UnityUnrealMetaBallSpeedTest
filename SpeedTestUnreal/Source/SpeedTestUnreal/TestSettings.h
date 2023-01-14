// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSettings.generated.h"

UCLASS(BlueprintType, DefaultToInstanced)
class SPEEDTESTUNREAL_API UTestSettings : public UObject
{
	GENERATED_BODY()	
public:
	const FString GetPath();
	void SaveSettings();
	bool TryLoadSettings();

	//Methodes
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool SingleThread = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool MultiThread = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool Blueprint = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool ComputeShader = true;

	//Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FColor DarkColor = FColor::Black;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FColor LightColor = FColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 Width = 1024;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 Height = 1024;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 BallCount = 50;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 RunCount = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float MinBallRadious = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float MaxBallRadious = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float Border = 1.;
};

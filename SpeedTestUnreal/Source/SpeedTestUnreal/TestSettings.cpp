// Fill out your copyright notice in the Description page of Project Settings.


#include "TestSettings.h"

const FString UTestSettings::GetPath()
{
	return FPaths::Combine(FPaths::ProjectDir(), "Settings.ini");
}

void UTestSettings::SaveSettings()
{
	FString saveData;

	saveData += FString::Printf(TEXT("Width=%d\n"), Width);
	saveData += FString::Printf(TEXT("Height=%d\n"), Height);
	saveData += FString::Printf(TEXT("BallCount=%d\n"), BallCount);
	saveData += FString::Printf(TEXT("RunCount=%d\n"), RunCount);

	saveData += FString::Printf(TEXT("MinBallRadious=%f\n"), MinBallRadious);
	saveData += FString::Printf(TEXT("MaxBallRadious=%f\n"), MaxBallRadious);
	saveData += FString::Printf(TEXT("Border=%f\n"), Border);

	auto ToString = [](const bool b) { return b ? FString("True") : FString("False"); };

	saveData += FString::Printf(TEXT("SingleThread=%s\n"), *ToString(SingleThread));
	saveData += FString::Printf(TEXT("MultiThread=%s\n"), *ToString(MultiThread));
	saveData += FString::Printf(TEXT("Blueprint=%s\n"), *ToString(Blueprint));
	saveData += FString::Printf(TEXT("ComputeShader=%s\n"), *ToString(ComputeShader));

	saveData += FString::Printf(TEXT("LightColor=#%s\n"), *LightColor.ToHex());
	saveData += FString::Printf(TEXT("DarkColor=#%s\n"), *DarkColor.ToHex());

	FString filePath = GetPath();
	FFileHelper::SaveStringToFile(saveData, *filePath);

	UE_LOG(LogTemp, Log, TEXT("Settings saved to: %s"), *filePath);
}

bool UTestSettings::TryLoadSettings()
{
	FString path = GetPath();
	if (FPaths::FileExists(path)) {
		TArray<FString> lines;
		FFileHelper::LoadFileToStringArray(lines, *path);
		
		typedef TTuple<FString, FString> TPair;
		TArray<TPair> values;

		FString Key, Value;
		for (int i = 0; i < lines.Num(); i++) {
			lines[i].Split("=",&Key,&Value);
			values.Add(TPair(Key,Value));
		}

		auto GetValue = [&](FString key){
			for (int i = 0; i < values.Num(); i++) {
				if (values[i].Get<0>() == key) {
					return values[i].Get<1>();
				}
			}

			UE_LOG(LogTemp,Fatal,TEXT("Key not found in Settings.ini file: %s"), *key);
			return FString("");
		};

		Width = FCString::Atoi(*GetValue("Width"));
		Height = FCString::Atoi(*GetValue("Height"));
		BallCount = FCString::Atoi(*GetValue("BallCount"));
		RunCount = FCString::Atoi(*GetValue("RunCount"));

		MinBallRadious = FCString::Atof(*GetValue("MinBallRadious"));
		MaxBallRadious = FCString::Atof(*GetValue("MaxBallRadious"));
		Border = FCString::Atof(*GetValue("Border"));

		SingleThread = FCString::ToBool(*GetValue("SingleThread"));
		MultiThread = FCString::ToBool(*GetValue("MultiThread"));
		Blueprint = FCString::ToBool(*GetValue("Blueprint"));
		ComputeShader = FCString::ToBool(*GetValue("ComputeShader"));

		LightColor =  FColor::FromHex(*GetValue("LightColor"));
		DarkColor = FColor::FromHex(*GetValue("DarkColor"));

		UE_LOG(LogTemp, Log, TEXT("Settings loaded from: %s"), *path);

		return true;
	}
	else {
		return false;
	}
}

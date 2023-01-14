// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MemoryTexture.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SPEEDTESTUNREAL_API UMemoryTexture : public UObject
{
	GENERATED_BODY()

	UMemoryTexture();
	~UMemoryTexture();

public:
	UFUNCTION()
	UTexture2D* GetTexture();
	void Fill(const FColor& Color,const int X=0, const int Y=0, const int SizeX=0, const int SizeY=0);
	UFUNCTION(BlueprintCallable)
	void Apply();
	UFUNCTION()
	void Init(const int TWidth, const int THeight);
	UFUNCTION(BlueprintCallable)
	void SetPixel(const FColor& Color, int X, int Y);

	void SetRawPixels(uint8* RawData);

	UFUNCTION(BlueprintCallable)
	int GetWidth() const;
	UFUNCTION(BlueprintCallable)
	int GetHeight() const;
protected:
	UTexture2D* Texture;
	int Width;
	int Height;
	FName Name;
	uint8* Data;
	FTexture2DMipMap Mip;
};

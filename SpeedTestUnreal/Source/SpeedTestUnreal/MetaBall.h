// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MetaBall.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SPEEDTESTUNREAL_API UMetaBall : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	int X;
	UPROPERTY(BlueprintReadOnly)
	int Y;
	UPROPERTY(BlueprintReadOnly)
	float R;

	void Init(const uint32& nX, const uint32& nY, const float& nR);
	float GetInfluenceTo(const uint32& pX, const uint32& pY);
};

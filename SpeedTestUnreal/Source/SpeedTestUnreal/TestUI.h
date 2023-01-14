// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSingleVoidDelegate);

class UTestSettings;

UCLASS()
class SPEEDTESTUNREAL_API UTestUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	FOnSingleVoidDelegate OnStartTests;

	UFUNCTION(BlueprintImplementableEvent)
	void OnTextureChanged();
	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingsChanged();

	UFUNCTION()
	void SetTexture(UTexture2D* newTexture);
	UFUNCTION()
	void SetSettings(UTestSettings* newSettings);
	
protected:
	UPROPERTY(BlueprintReadWrite)
		UTexture2D* texture;
	UPROPERTY(BlueprintReadWrite)
		UTestSettings* settings;

	UFUNCTION(BlueprintCallable)
		void StartTests();
};

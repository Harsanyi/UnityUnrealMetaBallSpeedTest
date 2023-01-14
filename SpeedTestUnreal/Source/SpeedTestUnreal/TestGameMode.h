// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoidDelegate);

class UTestSettings;
class UMemoryTexture;
class UMetaBall;

UCLASS()
class SPEEDTESTUNREAL_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:	
	void StartPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly)
		FColor DefaultColor = FColor::Silver;

	UFUNCTION(BlueprintImplementableEvent)
		void OnStartBlueprintFill(UMemoryTexture* MemTexture, UTestSettings* Settings, const TArray<UMetaBall*>& Balls);

	UPROPERTY(BlueprintAssignable)
	FVoidDelegate OnStartTesting;
	UPROPERTY(BlueprintAssignable)
	FVoidDelegate OnEndTesting;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Start Settings")
		TSubclassOf<class UTestUI> StartUIClass;
	UPROPERTY(EditDefaultsOnly, Category = "Start Settings")
		TSubclassOf<class ULogPanel> LogPanelClass;

	UPROPERTY(EditAnywhere, Instanced, Category="Start Settings")
		UTestSettings* TestSettings;
	UFUNCTION()
		void SettingsChanged();
	UFUNCTION()
		void StartTesting();
		void SingleThreadUpdate();
		void MultiThreadUpdate();
		void ComputeShaderUpdate();
		void GenerateBaseTexture();
		void GenerateMetaBalls();

private:
	UPROPERTY()
	UTestUI* UIInstance;
	UPROPERTY()
	ULogPanel* LogPanel;
	UPROPERTY()
	ATestGameMode* TestGameMode;
	UPROPERTY()
	UMemoryTexture* MemoryTexture;
	UPROPERTY()
	TArray<UMetaBall*> MetaBalls;

	ATestGameMode();
};

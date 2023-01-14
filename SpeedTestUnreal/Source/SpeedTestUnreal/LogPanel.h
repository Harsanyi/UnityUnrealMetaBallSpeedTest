// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LogPanel.generated.h"

UCLASS()
class SPEEDTESTUNREAL_API ULogPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void Log(FString& newLog);
	UFUNCTION()
	void Clear();
	UFUNCTION()
	void GetFullLog(FString& fullLog) const;

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateText(const FString& fullLog);
private:
	UPROPERTY()
	TArray<FString> logs;
};

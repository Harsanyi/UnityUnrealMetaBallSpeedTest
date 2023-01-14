// Fill out your copyright notice in the Description page of Project Settings.


#include "LogPanel.h"

void ULogPanel::Log(FString& newLog)
{
	logs.Add(newLog);
	FString full;
	GetFullLog(full);
	OnUpdateText(full);

	UE_LOG(LogTemp, Display, TEXT("%s"), *newLog);
}

void ULogPanel::Clear()
{
	logs.Empty();
	FString full;
	GetFullLog(full);
	OnUpdateText(full);
}

void ULogPanel::GetFullLog(FString& result) const
{
	result = "";
	for (int i = logs.Num()-1; i > -1; i--) {
		result.Append(logs[i]);
		if (i < logs.Num()) {
			result.Append("\n");
		}
	}
}

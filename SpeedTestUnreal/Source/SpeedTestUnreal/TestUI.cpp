// Fill out your copyright notice in the Description page of Project Settings.


#include "TestUI.h"

void UTestUI::SetTexture(UTexture2D* newTexture)
{
	texture = newTexture;
	OnTextureChanged();
}

void UTestUI::SetSettings(UTestSettings* newSettings)
{
	settings = newSettings;
	OnSettingsChanged();
}

void UTestUI::StartTests()
{
	OnStartTests.Broadcast();
}

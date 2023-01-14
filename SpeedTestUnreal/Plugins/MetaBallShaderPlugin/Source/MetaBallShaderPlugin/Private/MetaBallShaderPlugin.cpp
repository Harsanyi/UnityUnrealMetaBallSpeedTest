// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaBallShaderPlugin.h"

#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FMetaBallShaderPluginModule"

void FMetaBallShaderPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString ShaderPath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("MetaBallShaderPlugin"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/MetaBallShader"), ShaderPath);
}

void FMetaBallShaderPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogTemp, Warning, TEXT("MetaballShaderPlugin Shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMetaBallShaderPluginModule, MetaBallShaderPlugin)
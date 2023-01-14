#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

DECLARE_STATS_GROUP(TEXT("MetaBallShaderCS"), STATGROUP_MetaBallShaderCS, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("MetaBallShaderCS Execute"), STAT_MetaBallShaderCS_Execute, STATGROUP_MetaBallShaderCS);

class METABALLSHADERPLUGIN_API FMetaBallShaderCS : public FGlobalShader{

public:
	DECLARE_GLOBAL_SHADER(FMetaBallShaderCS);
	SHADER_USE_PARAMETER_STRUCT(FMetaBallShaderCS, FGlobalShader);

	//Settings struct
	BEGIN_SHADER_PARAMETER_STRUCT(FSettings, )
		SHADER_PARAMETER(int, Width)
		SHADER_PARAMETER(int, Height)
		SHADER_PARAMETER(float, Border)
	END_SHADER_PARAMETER_STRUCT()

	//Shader Params Struct
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint8>, Output)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<int>, BallX)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<int>, BallY)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float>, BallR)
		SHADER_PARAMETER(int, BallCount)
		SHADER_PARAMETER(int, Width)
		SHADER_PARAMETER(int, Height)
		SHADER_PARAMETER(float, Border)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<uint8>, lightColor)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<uint8>, darkColor)
	END_SHADER_PARAMETER_STRUCT()

	static void Dispatch(bool& done, uint8*& result,const TResourceArray<int>& BallX,const TResourceArray<int>& BallY,const TResourceArray<float>& BallR,const int& Width, const int& Height, const float& Border,const FColor& lightColor,const FColor& darkColor);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
};
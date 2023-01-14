#pragma once

#include "MetaBallShaderCS.h"
#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "RHIGPUReadback.h"
#include "HAL/ThreadManager.h"

void FMetaBallShaderCS::Dispatch(bool& done, uint8*& result,const TResourceArray<int>& BallX,const TResourceArray<int>& BallY,const TResourceArray<float>& BallR,const int& Width,const int& Height,const float& Border,const FColor& lightColor,const FColor& darkColor)
{
	UE_LOG(LogTemp,Warning,TEXT("Dispatch Inputs"));

	//On Render Thread
	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[&](FRHICommandListImmediate& RHICmdList) {
			
			FRDGBuilder GraphBuilder(RHICmdList);

			SCOPE_CYCLE_COUNTER(STAT_MetaBallShaderCS_Execute);
			DECLARE_GPU_STAT(MetaBallShaderCS)
			//Disabled these, because the causes crash the engine.
			//RDG_EVENT_SCOPE(GraphBuilder, "MetaBallShaderCS");
			//RDG_GPU_STAT_SCOPE(GraphBuilder, MetaBallShaderCS);
			
			//Create Shader
			typename FMetaBallShaderCS::FPermutationDomain PermutationVector;
			TShaderMapRef<FMetaBallShaderCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);
			
			if (ComputeShader.IsValid()) {
				FMetaBallShaderCS::FParameters* PassParams = GraphBuilder.AllocParameters<FMetaBallShaderCS::FParameters>();

				//Output Buffer
				FRDGBufferRef OutputBuffer = GraphBuilder.CreateBuffer(
					FRDGBufferDesc::CreateBufferDesc(sizeof(uint8),Width*Height*4),
					TEXT("OutputBuffer")
				);
				PassParams->Output = GraphBuilder.CreateUAV(FRDGBufferUAVDesc(OutputBuffer, PF_R8_UINT));

				//Settings
				PassParams->Width = Width;
				PassParams->Height = Height;
				PassParams->Border = Border;
				PassParams->BallCount = BallX.Num();

				//BallX
				FRDGBufferRef BallXBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(int), BallX.Num()), TEXT("BallX"));
				GraphBuilder.QueueBufferUpload(BallXBuffer, BallX.GetResourceData(), BallX.GetResourceDataSize());
				PassParams->BallX = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(BallXBuffer));

				//BallY
				FRDGBufferRef BallYBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(int), BallY.Num()), TEXT("BallY"));
				GraphBuilder.QueueBufferUpload(BallYBuffer, BallY.GetResourceData(), BallY.GetResourceDataSize());
				PassParams->BallY = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(BallYBuffer));

				//BallR
				FRDGBufferRef BallRBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(float), BallR.Num()), TEXT("BallR"));
				GraphBuilder.QueueBufferUpload(BallRBuffer, BallR.GetResourceData(), BallR.GetResourceDataSize());
				PassParams->BallR = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(BallRBuffer));

				//Light Color
				uint8 lightColorArray[] = {lightColor.R,lightColor.G, lightColor.B, lightColor.A};
				FRDGBufferRef lightColorBuffer = CreateUploadBuffer(GraphBuilder, TEXT("lightColor"), sizeof(uint8), 4, (void*)lightColorArray, sizeof(uint8) * 4);
				PassParams->lightColor = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(lightColorBuffer, EPixelFormat::PF_R8_UINT));

				//Dark Color
				uint8 darkColorArray[] = {darkColor.R, darkColor.G, darkColor.B, darkColor.A};
				FRDGBufferRef darkColorBuffer = CreateUploadBuffer(GraphBuilder, TEXT("darkColor"), sizeof(uint8), 4, (void*)darkColorArray, sizeof(uint8)*4);
				PassParams->darkColor = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(darkColorBuffer, EPixelFormat::PF_R8_UINT));

				//Set Pass
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("ExecuteMetaBallShader"),
					PassParams,
					ERDGPassFlags::Compute,
					[ComputeShader, &PassParams](FRHIComputeCommandList& RHICmdList) {
						FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParams, FIntVector(1,1,1));
					}
				);

				//Readback Result
				FRHIGPUBufferReadback* GPUBufferReadback = new FRHIGPUBufferReadback(TEXT("ExecuteMetaShaderOutput"));
				AddEnqueueCopyPass(GraphBuilder, GPUBufferReadback, OutputBuffer, Width*Height*4*sizeof(uint8));

				GraphBuilder.Execute();		
				RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);

				while (!GPUBufferReadback->IsReady()) {	
					FPlatformProcess::Sleep(0.001f);
				}

				result = (uint8*)GPUBufferReadback->Lock(Width * Height * 4 * sizeof(uint8));
				GPUBufferReadback->Unlock();

				delete GPUBufferReadback;

				done = true;
			}
			else {
				//Shader Wrong.
				UE_LOG(LogTemp, Error, TEXT("Meta Ball Shader is Invalid!"));
			}
		}
	);
}

IMPLEMENT_GLOBAL_SHADER(FMetaBallShaderCS, "/MetaBallShader/MetaBallShaderCS.usf", "MetaBallShaderCS", SF_Compute);

bool FMetaBallShaderCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	//const FPermutationDomain PermutationVector(Parameters.PermutationId);

	return true;
}

void FMetaBallShaderCS::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

	const FPermutationDomain PermutationVector(Parameters.PermutationId);

	/*
	* Here you define constants that can be used statically in the shader code.
	* Example:
	*/
	// OutEnvironment.SetDefine(TEXT("MY_CUSTOM_CONST"), TEXT("1"));

	/*
	* These defines are used in the thread count section of our shader
	*/
	/*
	OutEnvironment.SetDefine(TEXT("THREADS_X"), 1);
	OutEnvironment.SetDefine(TEXT("THREADS_Y"), 1);
	OutEnvironment.SetDefine(TEXT("THREADS_Z"), 1);
	*/
	// This shader must support typed UAV load and we are testing if it is supported at runtime using RHIIsTypedUAVLoadSupported
	//OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

	// FForwardLightingParameters::ModifyCompilationEnvironment(Parameters.Platform, OutEnvironment);
}

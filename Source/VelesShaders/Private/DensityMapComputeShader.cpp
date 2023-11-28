// Fill out your copyright notice in the Description page of Project Settings.


#include "DensityMapComputeShader.h"

#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderUtils.h"
#include "RHIStaticStates.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTarget2D.h"

namespace
{
	class FPermutationParamUseWeightmap : SHADER_PERMUTATION_BOOL("USE_WEIGHTMAP");
	class FPermutationParamDrawDebug : SHADER_PERMUTATION_BOOL("DRAW_DEBUG");
	class FPermutationParamUseHeightfield : SHADER_PERMUTATION_BOOL("USE_HEIGHTFIELD");
}

class FVelesDensityMapCS : public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FVelesDensityMapCS);
	SHADER_USE_PARAMETER_STRUCT(FVelesDensityMapCS, FGlobalShader);

	using FPermutationDomain = TShaderPermutationDomain<FPermutationParamUseWeightmap, FPermutationParamDrawDebug>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>,	InMaskTexture)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D,			InWeightmapTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState,				InWeightmapSampler)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>,OutCombined)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>,DebugTexture)
		SHADER_PARAMETER(FVector4f,							InClampPowMul)
		SHADER_PARAMETER(FVector4f,							InWeightmapScaleBias)
		SHADER_PARAMETER(FVector4f,							InWeightmapChannel)
		SHADER_PARAMETER(float,								InSize)
	END_SHADER_PARAMETER_STRUCT()

	static FIntVector GetGroupSize()
	{
		return FIntVector(8, 8, 1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_X"), GetGroupSize().X);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Y"), GetGroupSize().Y);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Z"), GetGroupSize().Z);
	}
};

IMPLEMENT_GLOBAL_SHADER(FVelesDensityMapCS, "/Plugin/Veles/Private/DensityMap.usf", "DensityMapCS", SF_Compute);

class FVelesGenerateCS : public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FVelesGenerateCS);
	SHADER_USE_PARAMETER_STRUCT(FVelesGenerateCS, FGlobalShader);

	using FPermutationDomain = TShaderPermutationDomain<FPermutationParamUseHeightfield>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>,	DensityMapTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState,				DensityMapSampler)
		SHADER_PARAMETER(FVector4f,							Bounds)
		SHADER_PARAMETER(FVector4f,							PatternOffsetAndScale)

		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float4>,		PatternBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float4>,	OutPointCloudBuffer)

		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D,			HeightfieldTexture)
		SHADER_PARAMETER(FVector4f,							HeightmapScaleBias)
		SHADER_PARAMETER(FVector3f,							HeightfieldLocation)
		SHADER_PARAMETER(FVector3f,							HeightfieldScale)
	
		SHADER_PARAMETER(FIntVector,						NumSize)
		
	END_SHADER_PARAMETER_STRUCT()

	static FIntVector GetGroupSize()
	{
		return FIntVector(4, 4, 16);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_X"), GetGroupSize().X);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Y"), GetGroupSize().Y);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Z"), GetGroupSize().Z);
	}
};

IMPLEMENT_GLOBAL_SHADER(FVelesGenerateCS, "/Plugin/Veles/Private/Generate.usf", "GenerateCS", SF_Compute);

class FVelesPlacementCS : public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FVelesPlacementCS);
	SHADER_USE_PARAMETER_STRUCT(FVelesPlacementCS, FGlobalShader);

	using FPermutationDomain = FShaderPermutationNone;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float4>,		PointCloudBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FInstanceTransformPayload>, OutInstancesBuffer)
		SHADER_PARAMETER(uint32,							NumElements)
		SHADER_PARAMETER(uint32,							NumX)
		SHADER_PARAMETER(uint32,							NumY)
	
		SHADER_PARAMETER(uint32,							Seed)

		SHADER_PARAMETER(FVector2f,							InScale)
		SHADER_PARAMETER(FVector2f,							InYawRotation)

		SHADER_PARAMETER(FVector3f,							InLocationOffset)

		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float>,		AssetsProbabilityBuffer)
		SHADER_PARAMETER(uint32,							NumAssets)
		SHADER_PARAMETER(float,								TotalAssetsProbability)
		
	END_SHADER_PARAMETER_STRUCT()

	static FIntVector GetGroupSize()
	{
		return FIntVector(4, 4, 16);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_X"), GetGroupSize().X);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Y"), GetGroupSize().Y);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Z"), GetGroupSize().Z);
	}
};

IMPLEMENT_GLOBAL_SHADER(FVelesPlacementCS, "/Plugin/Veles/Private/Placement.usf", "PlacementCS", SF_Compute);

class FVelesDebugCS : public FGlobalShader
{
public:
	
	DECLARE_GLOBAL_SHADER(FVelesDebugCS);
	SHADER_USE_PARAMETER_STRUCT(FVelesDebugCS, FGlobalShader);

	using FPermutationDomain = FShaderPermutationNone;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector,					GridSize)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FInstanceTransformPayload>, OutInstancesBuffer)
	END_SHADER_PARAMETER_STRUCT()

	static FIntVector GetGroupSize()
	{
		return FIntVector(8, 8, 8);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_X"), GetGroupSize().X);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Y"), GetGroupSize().Y);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Z"), GetGroupSize().Z);
	}
};

IMPLEMENT_GLOBAL_SHADER(FVelesDebugCS, "/Plugin/Veles/Private/Debug.usf", "DebugCS", SF_Compute);

void VelesShaderUtils::AddComputeDensityMap(FRDGBuilder& GraphBuilder, const FVelesDensityMapCSDispatchRDGParams& InParams)
{
	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	
	FVelesDensityMapCS::FPermutationDomain PermutationVectorCS;
	PermutationVectorCS.Set<FPermutationParamUseWeightmap>(InParams.WeightmapChannel != INDEX_NONE);
	PermutationVectorCS.Set<FPermutationParamDrawDebug>(InParams.bDebug);
	TShaderMapRef<FVelesDensityMapCS> ComputeShader(ShaderMap, PermutationVectorCS);

	if (ComputeShader.IsValid())
	{
		FVelesDensityMapCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FVelesDensityMapCS::FParameters>();
					
		{
			PassParameters->InMaskTexture = InParams.Texture;
			PassParameters->InWeightmapTexture= InParams.Texture;
			PassParameters->InWeightmapSampler = TStaticSamplerState<ESamplerFilter::SF_Bilinear>::GetRHI();
			PassParameters->InClampPowMul = InParams.ClampPowMul;
			PassParameters->InWeightmapScaleBias = InParams.WeightmapScaleBias;
			FVector4f Channels = FVector4f::Zero();
			Channels[InParams.WeightmapChannel] = 1.0f;
			PassParameters->InWeightmapChannel = Channels;
			PassParameters->OutCombined = InParams.OutputTexture;
			PassParameters->DebugTexture = InParams.DebugTexture;
			PassParameters->InSize = InParams.TextureSize;
		}

		FIntVector GroupCount;
		GroupCount.X = FMath::DivideAndRoundUp<uint32>(InParams.TextureSize, FVelesDensityMapCS::GetGroupSize().X);
		GroupCount.Y = FMath::DivideAndRoundUp<uint32>(InParams.TextureSize, FVelesDensityMapCS::GetGroupSize().Y);
		GroupCount.Z = FMath::DivideAndRoundUp<uint32>(1, FVelesDensityMapCS::GetGroupSize().Z);

		FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Veles::DensityMapCS"),
				ERDGPassFlags::AsyncCompute,
				ComputeShader,
				PassParameters,
				GroupCount
			);
	}
}

void VelesShaderUtils::AddComputeGenerate(FRDGBuilder& GraphBuilder, const FVelesGenerateCSDispatchRDGParams& InParams)
{
	{
		FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

		FVelesGenerateCS::FPermutationDomain PermutationVectorCS;
		PermutationVectorCS.Set<FPermutationParamUseHeightfield>(InParams.HeightfieldTexture != nullptr);
		TShaderMapRef<FVelesGenerateCS> ComputeShader(ShaderMap, PermutationVectorCS);
		
		if (ComputeShader.IsValid())
		{
			FVelesGenerateCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FVelesGenerateCS::FParameters>();
			PassParameters->DensityMapTexture = InParams.DensityTexture;
			PassParameters->DensityMapSampler = TStaticSamplerState<ESamplerFilter::SF_Bilinear>::GetRHI();
			PassParameters->Bounds = InParams.WorldBounds;
			PassParameters->PatternOffsetAndScale = InParams.PatternOffsetAndScale;
			PassParameters->NumSize = FIntVector(InParams.PatternNumX, InParams.PatternNumY, InParams.PatternElements);
			PassParameters->PatternBuffer = InParams.PatternBuffer;
			PassParameters->OutPointCloudBuffer = InParams.PointCloudBuffer;

			PassParameters->HeightfieldTexture = InParams.HeightfieldTexture;
			PassParameters->HeightmapScaleBias = InParams.HeightmapScaleBias;
			PassParameters->HeightfieldLocation = InParams.HeightfieldLocation;
			PassParameters->HeightfieldScale = InParams.HeightfieldScale;

			FIntVector GroupCount;
			GroupCount.X = FMath::DivideAndRoundUp<uint32>(InParams.PatternNumX, FVelesGenerateCS::GetGroupSize().X);
			GroupCount.Y = FMath::DivideAndRoundUp<uint32>(InParams.PatternNumY, FVelesGenerateCS::GetGroupSize().Y);
			GroupCount.Z = FMath::DivideAndRoundUp<uint32>(InParams.PatternElements, FVelesGenerateCS::GetGroupSize().Z);

			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Veles::GenerateCS"),
				ERDGPassFlags::AsyncCompute,
				ComputeShader,
				PassParameters,
				GroupCount
			);
		}
	}
}

void VelesShaderUtils::AddComputePlacement(FRDGBuilder& GraphBuilder, const FVelesPlacementCSDispatchRDGParams& InParams)
{
	{
		FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

		TShaderMapRef<FVelesPlacementCS> ComputeShader(ShaderMap);
		
		if (ComputeShader.IsValid())
		{
			FVelesPlacementCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FVelesPlacementCS::FParameters>();
			PassParameters->PointCloudBuffer = InParams.PointCloudBuffer;
			PassParameters->OutInstancesBuffer = InParams.OutInstancesBuffer;
			PassParameters->NumElements = InParams.NumElements;
			PassParameters->NumX = InParams.NumX;
			PassParameters->NumY = InParams.NumY;

			PassParameters->Seed = InParams.Seed;
			PassParameters->InScale = InParams.RandomScale;
			PassParameters->InYawRotation = InParams.RandomYawRotation;
			
			PassParameters->InLocationOffset = InParams.LocationOffset;

			PassParameters->AssetsProbabilityBuffer = InParams.AssetsProbabilityBuffer;
			PassParameters->NumAssets = InParams.NumAssets;
			PassParameters->TotalAssetsProbability = InParams.TotalAssetsProbability;
			
			FIntVector GroupCount;
			GroupCount.X = FMath::DivideAndRoundUp<uint32>(InParams.NumX, FVelesPlacementCS::GetGroupSize().X);
			GroupCount.Y = FMath::DivideAndRoundUp<uint32>(InParams.NumY, FVelesPlacementCS::GetGroupSize().Y);
			GroupCount.Z = FMath::DivideAndRoundUp<uint32>(InParams.NumElements, FVelesPlacementCS::GetGroupSize().Z);
			
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("VelesPlacementCS"),
				PassParameters,
				ERDGPassFlags::AsyncCompute,
				[PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
			{
				FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
			});
		}
	}
}

void VelesShaderUtils::AddComputeDebug(FRDGBuilder& GraphBuilder, const FVelesDebugCSDispatchRDGParams& InParams)
{
	{
		FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
		TShaderMapRef<FVelesDebugCS> ComputeShader(ShaderMap);
		
		if (ComputeShader.IsValid())
		{
			FVelesDebugCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FVelesDebugCS::FParameters>();
			PassParameters->GridSize = InParams.GridSize;
			PassParameters->OutInstancesBuffer = InParams.OutInstancesBuffer;
			
			FIntVector GroupCount;
			GroupCount.X = FMath::DivideAndRoundUp<uint32>(InParams.GridSize.X, FVelesDebugCS::GetGroupSize().X);
			GroupCount.Y = FMath::DivideAndRoundUp<uint32>(InParams.GridSize.Y, FVelesDebugCS::GetGroupSize().Y);
			GroupCount.Z = FMath::DivideAndRoundUp<uint32>(InParams.GridSize.Z, FVelesDebugCS::GetGroupSize().Z);
			
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("Veles::DebugCS"),
				ERDGPassFlags::AsyncCompute,
				ComputeShader,
				PassParameters,
				GroupCount
			);
		}
	}
}
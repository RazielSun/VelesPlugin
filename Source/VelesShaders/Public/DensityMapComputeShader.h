// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

struct FInstanceTransformPayload
{
	FVector4f Location;
	FVector4f Rotation;
	FVector4f Scale;
};

struct VELESSHADERS_API FVelesDensityMapCSDispatchRDGParams
{
	FRDGTextureSRVRef Texture;
	FRDGTextureUAVRef OutputTexture;
	FVector4f ClampPowMul;
	FVector4f WeightmapScaleBias;
	int32 WeightmapChannel = INDEX_NONE;
	float TextureSize;
	bool bDebug = false;
	FRDGTextureUAVRef DebugTexture;
};

struct VELESSHADERS_API FVelesGenerateCSDispatchRDGParams
{
	FRDGTextureSRVRef DensityTexture;
	FVector4f WorldBounds;
	
	FRDGBufferSRVRef PatternBuffer;
	uint32 PatternElements;
	uint32 PatternNumX;
	uint32 PatternNumY;
	FVector4f PatternOffsetAndScale;
	
	FRDGTextureSRVRef HeightfieldTexture;
	FVector4f HeightmapScaleBias;
	FVector3f HeightfieldLocation;
	FVector3f HeightfieldScale;

	FRDGBufferUAVRef PointCloudBuffer;
};

struct VELESSHADERS_API FVelesPlacementCSDispatchRDGParams
{
	FRDGBufferSRVRef PointCloudBuffer;
	FRDGBufferUAVRef OutInstancesBuffer;
	uint32 NumElements;
	uint32 NumX;
	uint32 NumY;
	
	uint32 Seed;

	FVector2f RandomScale;
	FVector2f RandomYawRotation;
	
	FVector3f LocationOffset;

	FRDGBufferSRVRef AssetsProbabilityBuffer;
	uint32 NumAssets;
	float TotalAssetsProbability;
};

struct VELESSHADERS_API FVelesDebugCSDispatchRDGParams
{
	FIntVector GridSize;
	FRDGBufferUAVRef OutInstancesBuffer;
};

namespace VelesShaderUtils
{
	VELESSHADERS_API void AddComputeDensityMap(
		FRDGBuilder& GraphBuilder, const FVelesDensityMapCSDispatchRDGParams& InParams
	);

	VELESSHADERS_API void AddComputeGenerate(
		FRDGBuilder& GraphBuilder, const FVelesGenerateCSDispatchRDGParams& InParams
	);

	VELESSHADERS_API void AddComputePlacement(
		FRDGBuilder& GraphBuilder, const FVelesPlacementCSDispatchRDGParams& InParams
	);

	VELESSHADERS_API void AddComputeDebug(
		FRDGBuilder& GraphBuilder, const FVelesDebugCSDispatchRDGParams& InParams
	);
}



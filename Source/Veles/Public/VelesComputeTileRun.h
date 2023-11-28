// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UVelesPlacementPatternDataAsset;
class UVelesSubBiomeSchemeDataAsset;
class ULandscapeComponent;

struct FVelesMaskTileInfo
{
	UTexture2D* Texture = nullptr;
	FVector4f ClampPowMul;
	bool bDebugDraw = false;
};

struct FVelesWeightmapTileInfo : public FVelesMaskTileInfo
{
	FVector4f ScaleBias;
	int32 Channel = 0;
};

class FVelesComputeTileRun
{
public:
	FVelesComputeTileRun() = default;
	FVelesComputeTileRun(int32 X, int32 Y);
	~FVelesComputeTileRun();

	//
	FIntPoint Coord;
	FBox WorldBounds;

	FVector4f GetBounds() const
	{
		return FVector4f(WorldBounds.Min.X, WorldBounds.Min.Y, WorldBounds.Max.X, WorldBounds.Max.Y);
	}

	//
	TWeakObjectPtr<UVelesSubBiomeSchemeDataAsset> SubBiome;

	//
	TWeakObjectPtr<UVelesPlacementPatternDataAsset> Pattern;
	FVector4f PatternOffsetAndScale;
	FIntPoint PatternNumXY;

	int32 Seed;
	FVector2f RandomScale;
	FVector2f RandomYawRotation;
	
	FVector3f LocationOffset;

	int32 NumAssets;
	float TotalAssetProbability;
	TArray<float> AssetsProbabilities;

	// Density Maps
	int32 TextureSize;
	TArray<FVelesMaskTileInfo> MasksInfos;
	TArray<FVelesWeightmapTileInfo> WeightmapInfos;
	UTextureRenderTarget2D* DebugTexture;
	bool bDebugDraw;

	// Landscape
	ULandscapeComponent* LandscapeComponent;
	UTexture2D* HeightmapTexture;
	FVector4f HeightmapScaleBias;
	// FBox LandscapeBounds;
	FVector3f LandscapeLocation;
	FVector3f LandscapeScale;

	//
	FRHIGPUBufferReadback* GPUReadback;

	TMap<int32, TArray<FTransform>> InstancedObjectTransforms;
	
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesComputeTileRun.h"

FVelesComputeTileRun::FVelesComputeTileRun(int32 X, int32 Y) :
	Coord(X, Y)
	, WorldBounds(ForceInit)
	, Pattern(nullptr)
	, PatternOffsetAndScale(0, 0, 1, 1)
	, PatternNumXY(0, 0)
	, Seed(0)
	, RandomScale(0, 0)
	, RandomYawRotation(0, 0)
	, LocationOffset(0,0,0)
	, NumAssets(0)
	, TotalAssetProbability(0)
	, TextureSize(0)
	, DebugTexture(nullptr)
	, bDebugDraw(false)
	, LandscapeComponent(nullptr)
	, HeightmapTexture(nullptr)
	, GPUReadback(nullptr)
{
}

FVelesComputeTileRun::~FVelesComputeTileRun()
{
}

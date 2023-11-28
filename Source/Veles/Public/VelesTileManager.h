// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FVelesComputeTileRun;
class UVelesCoreSubsystem;

class FVelesTileManager : public FRenderResource
{
public:
	FVelesTileManager(UVelesCoreSubsystem* InCoreSubsystem);
	
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	void Update(float DeltaTime);
	
	void ComputeTile(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& InTile);

protected:
	int32 MaxReadbackBuffersNum = 4;
	TArray<FRHIGPUBufferReadback*, TInlineAllocator<8>> ReadbackBuffersInUse;
	TArray<FRHIGPUBufferReadback*, TInlineAllocator<8>> ReadbackBuffersReady;

	TWeakObjectPtr<UVelesCoreSubsystem> CoreSubsystem;

	TArray<TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>> ActiveTiles;
	TArray<TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>> PostponedTiles;

	void ComputeTileInternal(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& InTile);
	void PostponeTile(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& InTile);
	bool RenderThread_ReadbackPositions(FRHIGPUBufferReadback* GPUReadback);

	void OnReadbackPositionDone();
	
};

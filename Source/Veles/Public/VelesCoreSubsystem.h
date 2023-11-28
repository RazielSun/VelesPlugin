// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesTileManager.h"
#include "Subsystems/WorldSubsystem.h"

#include "VelesCoreSubsystem.generated.h"

class AVelesInstancedMeshActor;
class FVelesComputeTileRun;
class UVelesBiomeSchemeDataAsset;
class UVelesSubBiomeSchemeDataAsset;
class AVelesCore;
class AVelesLayerBaseActor;
class ALandscape;

UCLASS()
class VELES_API UVelesCoreSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	friend class FVelesTileManager;

public:
	UVelesCoreSubsystem();
	virtual ~UVelesCoreSubsystem();

	DECLARE_EVENT(UVelesCoreSubsystem, FOnLayerPaintChanged);

	static FOnLayerPaintChanged LayerPaintChangedEvent;

	// Begin FTickableGameObject overrides
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override { return true; }
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual TStatId GetStatId() const override;
	// End FTickableGameObject overrides

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem
	
	void RegisterCoreActor(AVelesCore* InCore);
	void UnregisterCoreActor(AVelesCore* InCore);

	void RegisterLayer(AVelesLayerBaseActor* InActor);
	void UnregisterLayer(AVelesLayerBaseActor* InActor);

	const UTexture2D* GetTextureFromActor(const FName& InName, const FIntPoint& InKey) const;

	UFUNCTION(BlueprintCallable)
	void RebuildAll();

	void OnLayerPaintChanged();

	AVelesInstancedMeshActor* GetOrCreateInstancedMeshActor(const FIntPoint& InBase);

protected:
	UPROPERTY(Transient)
	TMap<FName, TWeakObjectPtr<AVelesLayerBaseActor>> LayersMap;

	UPROPERTY(Transient)
	TWeakObjectPtr<AVelesCore> CurrentCore = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<ALandscape> CurrentLandscape = nullptr;
	bool bLandscapeNotFound = false;

	UPROPERTY(Transient)
	TMap<FIntPoint, TSoftObjectPtr<AVelesInstancedMeshActor>> InstancedActors;
	bool bInstancedActorsCollected = false;

	TUniquePtr<FVelesTileManager> TileManager;
	TArray<TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>> Tiles;
	TMap<FIntPoint, TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>> ReadyTiles;

	FDelegateHandle LayerPaintHandle;

	void SendTileToCompute(int32 X, int32 Y, const TObjectPtr<UVelesSubBiomeSchemeDataAsset>& SubBiome);
	
	void GameThread_HandleTile(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& InTile);

	void CollectInstancedActors();
};

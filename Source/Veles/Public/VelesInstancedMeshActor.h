// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VelesInstancedMeshActor.generated.h"

class UVelesSubBiomeSchemeDataAsset;
class UHierarchicalInstancedStaticMeshComponent;

USTRUCT()
struct FVelesInstancedMeshTypeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 AssetIndex = 0;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UVelesSubBiomeSchemeDataAsset> SubBiome;

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component;
};

UCLASS(NotPlaceable)
class VELES_API AVelesInstancedMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AVelesInstancedMeshActor(const FObjectInitializer& ObjectInitializer);

	virtual void Init(int32 InBaseX, int32 InBaseY);

	UPROPERTY(VisibleAnywhere, Category="Veles")
	int32 ActorBaseX;

	UPROPERTY(VisibleAnywhere, Category="Veles")
	int32 ActorBaseY;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<USceneComponent> Parent;

protected:
	UPROPERTY()
	TArray<FVelesInstancedMeshTypeInfo> Infos;

	FVelesInstancedMeshTypeInfo* GetOrCreateInfo(int32 InAssetIndex, UVelesSubBiomeSchemeDataAsset* InSubBiome);
	UHierarchicalInstancedStaticMeshComponent* CreateComponent();
	bool RemoveComponent(UHierarchicalInstancedStaticMeshComponent* InComponent);

public:
	FIntPoint GetActorBase() const
	{
		return FIntPoint(ActorBaseX, ActorBaseY);
	}

	void SetActorBase(FIntPoint InSectionBase)
	{
		ActorBaseX = InSectionBase.X;
		ActorBaseY = InSectionBase.Y;
	}

	void ClearAllInstances();
	void ClearInstances(int32 InAssetIndex);
	void UpdateInstances(int32 InAssetIndex, UVelesSubBiomeSchemeDataAsset* InSubBiome, const TArray<FTransform>& InInstanceTransforms);
	void UpdateInstances(UVelesSubBiomeSchemeDataAsset* InSubBiome, const TMap<int32, TArray<FTransform>>& InInstanceTransforms);
};

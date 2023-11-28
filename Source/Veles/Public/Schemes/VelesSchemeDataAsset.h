// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VelesSchemeDataAsset.generated.h"

class UStaticMesh;
class UVelesPlacementPatternDataAsset;

UENUM()
enum class FVelesLayerType : uint8
{
	None = 0,
	Mask,
	Weightmap
};

USTRUCT()
struct FVelesSchemeMaskParam
{
	GENERATED_BODY()

	FVelesSchemeMaskParam();

	UPROPERTY(EditAnywhere)
	FString LayerName;

	UPROPERTY(EditAnywhere)
	FVelesLayerType LayerType;

	UPROPERTY(EditAnywhere)
	FVector2D ClampRange;

	UPROPERTY(EditAnywhere)
	float PowerFactor;

	UPROPERTY(EditAnywhere)
	float Multiplier;

	UPROPERTY(EditAnywhere, Transient, Category=Debug)
	bool bDebugDraw;
	
};

USTRUCT()
struct FVelesSchemeAssetParam
{
	GENERATED_BODY()

	FVelesSchemeAssetParam();

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> Asset;

	UPROPERTY(EditAnywhere)
	float Probability;
};

UCLASS(BlueprintType)
class VELES_API UVelesSubBiomeSchemeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UVelesSubBiomeSchemeDataAsset();
	
	UPROPERTY(EditAnywhere, Category="Masks")
	TArray<FVelesSchemeMaskParam> Masks;

	UPROPERTY(EditAnywhere, Category="Pattern")
	TObjectPtr<UVelesPlacementPatternDataAsset> Pattern;

	UPROPERTY(EditAnywhere, Category="Pattern")
	FVector2D PatternScaleSize;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UHierarchicalInstancedStaticMeshComponent> ClassTemplate;

	// @todo: ?
	// UPROPERTY(EditAnywhere, Instanced)
	// TObjectPtr<UHierarchicalInstancedStaticMeshComponent> ClassTemplate;

	UPROPERTY(EditAnywhere, Category=Assets)
	TArray<FVelesSchemeAssetParam> Assets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Assets)
	FVector2f RandomYawRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Assets)
	FVector2f RandomScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Assets)
	FVector3f LocationOffset;

	UPROPERTY(EditAnywhere, Category=Assets)
	int32 RandomSeed;

	UPROPERTY(EditAnywhere, Transient, Category=Debug)
	bool bDebugDraw;
};

UCLASS(BlueprintType)
class VELES_API UVelesBiomeSchemeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UVelesSubBiomeSchemeDataAsset>> SubBiomes;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VelesPlacementPatternGenerator.generated.h"

UCLASS(Abstract, EditInlineNew)
class VELES_API UVelesPlacementPatternGenerator : public UObject
{
	GENERATED_BODY()

public:
	virtual TArray<FVector4f> Generate(int32 InSeed) const;

protected:
	bool IsIntersected(TArray<FSphere3d>& InSpheres, const FVector4& InPoint, float MaxFootprint, bool bAsPoint = false) const;
	bool IsIntersected(TArray<FSphere3d>& InSpheres, const FSphere3d& InSphere, float MaxFootprint, bool bAsPoint = false) const;
};


UCLASS()
class UVelesPlacementPatternGridGenerator : public UVelesPlacementPatternGenerator
{
	GENERATED_BODY()

public:
	UVelesPlacementPatternGridGenerator();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	FIntPoint GridSize;
	
	virtual TArray<FVector4f> Generate(int32 InSeed) const override;
};

UCLASS()
class UVelesPlacementPatternRandomGenerator : public UVelesPlacementPatternGenerator
{
	GENERATED_BODY()

public:
	UVelesPlacementPatternRandomGenerator();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	int32 ItemCount;
	
	virtual TArray<FVector4f> Generate(int32 InSeed) const override;
};

UCLASS()
class UVelesPlacementPatternBlueNoiseGenerator : public UVelesPlacementPatternGenerator
{
	GENERATED_BODY()

public:
	UVelesPlacementPatternBlueNoiseGenerator();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	int32 ItemCount;

	// We normalize it to divide on 1000
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	FVector2D Footprint;
	
	virtual TArray<FVector4f> Generate(int32 InSeed) const override;
};


UCLASS()
class UVelesPlacementPatternHexGenerator : public UVelesPlacementPatternGenerator
{
	GENERATED_BODY()

public:
	UVelesPlacementPatternHexGenerator();

	// We normalize it to divide on 1000
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	FVector2D Footprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	FVector2D JitterXY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	float JitterRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	float ExtendBorder;
	
	virtual TArray<FVector4f> Generate(int32 InSeed) const override;
};
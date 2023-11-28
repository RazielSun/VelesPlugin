// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "VelesWorldDataGenerator.generated.h"

class AVelesLayerWorldDataActor;

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, meta = (ShowOnlyInnerProperties))
class VELES_API UVelesWorldDataGenerator : public UObject
{
	GENERATED_BODY()

public:
	virtual void Generate(AVelesLayerWorldDataActor* InWorldData) {}
};

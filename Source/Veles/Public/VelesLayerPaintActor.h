// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayerBaseActor.h"
#include "VelesLayerPaintActor.generated.h"

class UVelesLayerComponent;

UCLASS()
class VELES_API AVelesLayerPaintActor : public AVelesLayerBaseActor
{
	GENERATED_BODY()

public:
	AVelesLayerPaintActor(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
};

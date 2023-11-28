// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayerBaseActor.h"

#include "VelesLayerWorldDataActor.generated.h"

class UVelesWorldDataGenerator;

UCLASS()
class VELES_API AVelesLayerWorldDataActor : public AVelesLayerBaseActor
{
	GENERATED_BODY()

public:
	AVelesLayerWorldDataActor(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<UVelesWorldDataGenerator> Generator = nullptr;
#endif

#if WITH_EDITOR
	UFUNCTION(CallInEditor)
	void Generate();
#endif
};


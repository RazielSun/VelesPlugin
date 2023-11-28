// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayerWorldDataActor.h"

#if WITH_EDITORONLY_DATA
#include "VelesWorldDataGenerator.h"
#endif

AVelesLayerWorldDataActor::AVelesLayerWorldDataActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

#if WITH_EDITOR
void AVelesLayerWorldDataActor::Generate()
{
#if WITH_EDITORONLY_DATA
	if (Generator != nullptr)
	{
		Generator->Generate(this);
	}
#endif
}
#endif

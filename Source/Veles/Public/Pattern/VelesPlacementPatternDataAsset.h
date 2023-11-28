// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "VelesPlacementPatternDataAsset.generated.h"

class UVelesPlacementPatternGenerator;

UCLASS()
class VELES_API UVelesPlacementPatternDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UVelesPlacementPatternDataAsset(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pattern")
	TArray<FVector4f> Pattern;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category="Debug Draw")
	int32 DebugTextureSize = 1024;

	UPROPERTY(VisibleAnywhere, Transient, Category="Debug Draw")
	TObjectPtr<UTextureRenderTarget2D> DebugTexture;
#endif

#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category="Generation")
	void Generate();

	UFUNCTION(CallInEditor, Category="Generation")
	void GetNewSeed();

	UFUNCTION(CallInEditor, Category="Debug Draw")
	void Draw();
#endif

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Generation")
	TObjectPtr<UVelesPlacementPatternGenerator> Generator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generation")
	int32 Seed;
	
#if WITH_EDITOR
	void InitDebugTexture();
#endif
};

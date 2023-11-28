// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "VelesCore.generated.h"

class AVelesBaseActor;
class UVelesBiomeSchemeDataAsset;
class UVelesPlacementPatternDataAsset;

UCLASS(NotPlaceable, HideCategories=(Actor,Networking,Collision,Physics,Rendering,Replication,Transform,Cooking))
class VELES_API AVelesCore : public AInfo
{
	GENERATED_BODY()

public:
	AVelesCore(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	int32 NumComponentsX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	int32 NumComponentsY;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	int32 ComponentsSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	FVector2D WorldOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	FVector WorldLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	FVector2D WorldScale;

	UPROPERTY(EditAnywhere, Category="Veles|Biomes")
	TArray<TObjectPtr<UVelesBiomeSchemeDataAsset>> BiomesSchemes;

	UPROPERTY(VisibleAnywhere, Transient, Category="Veles|Debug")
	TMap<FIntPoint, TObjectPtr<UTextureRenderTarget2D>> CompositeTextures;

	UFUNCTION(CallInEditor)
	void Clear();

	virtual void PostRegisterAllComponents() override;
	virtual void UnregisterAllComponents(bool bForReregister = false) override;

	TObjectPtr<UTextureRenderTarget2D> GetOrCreateCompositeTexture(const FIntPoint& InKey);

	FBox GetTileBounds(int32 X, int32 Y, int32 Size);

#if WITH_EDITOR
	void Setup(const FGuid& InGuid, int32 InNumComponentsX, int32 InNumComponentsY, int32 InComponentsSizeQuads, const FVector2D& InWorldOffset, const FVector& InWorldLocation, const FVector2D& InWorldScale);
#endif

protected:
	UPROPERTY()
	FGuid VelesGuid;

	TOptional<FTransform> CachedTransform;
	
	UTextureRenderTarget2D* CreateRenderTargetTexture() const;
};

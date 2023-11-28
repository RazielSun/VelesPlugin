// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "VelesLayerComponent.generated.h"

class AVelesLayerBaseActor;

UCLASS()
class VELES_API UVelesLayerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UVelesLayerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void Init(int32 InBaseX, int32 InBaseY, int32 InComponentSizeQuads);

	UPROPERTY(VisibleAnywhere, Category="VelesComponent")
	int32 ComponentBaseX;

	UPROPERTY(VisibleAnywhere, Category="VelesComponent")
	int32 ComponentBaseY;

	UPROPERTY(VisibleAnywhere, Category="VelesComponent")
	int32 ComponentSizeQuads;

	UTexture2D* GetComponentTexture() const { return ComponentTexture; }
	UTexture2D* GetOrCreateComponentTexture();
	void ClearComponentTexture();

	AVelesLayerBaseActor* GetProxyActor();
	const AVelesLayerBaseActor* GetProxyActor() const;

	void RequestDataUpdate(bool bUpdateAll = true);

protected:
	void InitComponentTexture();

	virtual FName GetComponentTextureName();

	UPROPERTY(VisibleAnywhere, Category="VelesComponent")
	TObjectPtr<UTexture2D> ComponentTexture = nullptr;

	UPROPERTY(Transient)
	int32 LayerUpdateFlag;
	
public:
	/** @return Component section base as FIntPoint */
	FIntPoint GetComponentBase() const
	{
		return FIntPoint(ComponentBaseX, ComponentBaseY);
	}

	/** @param InSectionBase new section base for a component */
	void SetComponentBase(FIntPoint InSectionBase)
	{
		ComponentBaseX = InSectionBase.X;
		ComponentBaseY = InSectionBase.Y;
	}

	void GetComponentExtent(int32& MinX, int32& MinY, int32& MaxX, int32& MaxY) const;
};

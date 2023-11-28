// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorPartition/PartitionActor.h"
#include "VelesLayerBaseActor.generated.h"

class AVelesCore;
class UVelesLayerComponent;

UCLASS()
class VELES_API AVelesLayerBaseActor : public APartitionActor
{
	GENERATED_BODY()

	friend class UVelesLayerComponent;

public:
	AVelesLayerBaseActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	FName LayerName;

	virtual TObjectPtr<UVelesLayerComponent> GetLayerComponent(FIntPoint InKey) const;
	const TArray<TObjectPtr<UVelesLayerComponent>>& GetLayerComponents() const;
	bool GetComponentsToUpdate(TArray<FIntPoint>& OutKeys);
	
	bool IsInitialized() const { return bInitialized; }

	AVelesCore* GetCore() const;

	bool HasContentToUpdate() const { return !UpdatedComponentKeys.IsEmpty(); }
	void RequestContentUpdate(const UVelesLayerComponent* InComponent);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Veles|Base")
	TSoftObjectPtr<AVelesCore> CurrentCore;
	
	UPROPERTY()
	FGuid VelesGuid;

	UPROPERTY()
	TArray<TObjectPtr<UVelesLayerComponent>> LayersComponents;

	UPROPERTY()
	bool bInitialized = false;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(Transient)
	TArray<FIntPoint> UpdatedComponentKeys;

	void Initialize();

	virtual void PostRegisterAllComponents() override;
	virtual void UnregisterAllComponents(bool bForReregister = false) override;

	void RegisterToSubsystem();
	void UnregisterFromSubsystem();

public:
#if WITH_EDITOR
	void Setup(const FGuid& InGuid, AVelesCore* InCore, FName InName);
#endif

	bool GetExtent(FIntRect& OutExtent) const;
	virtual bool GetExtent(int32& MinX, int32& MinY, int32& MaxX, int32& MaxY) const;

protected:
#if WITH_EDITOR
	virtual void SetupInternal(const FGuid& InGuid, AVelesCore* InCore, FName InName);
#endif
};

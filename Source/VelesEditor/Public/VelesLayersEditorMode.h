// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"
#include "VelesLayersEditorMode.generated.h"

class AVelesCore;
class AVelesLayerBaseActor;
class UVelesLayersEditorObject;
class UMaterialInstanceDynamic;

const FName VELES_SURFACE_TAG("VelesSurface");

UENUM()
enum class EVelesLayersToolMode
{
	None = 0,
	NewActor,
	Modify
};

USTRUCT()
struct FVelesLayerInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString LayerName;

	UPROPERTY()
	TWeakObjectPtr<AVelesLayerBaseActor> LayerActor;
};

UCLASS()
class UVelesLayersEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	const static FEditorModeID EM_VelesLayersEditorModeId;

	UPROPERTY(Transient)
	TObjectPtr<UVelesLayersEditorObject> UISettings;

	EVelesLayersToolMode CurrentToolMode;

	UVelesLayersEditorMode();
	virtual ~UVelesLayersEditorMode();

	/** UEdMode interface */
	virtual void Enter() override;
	virtual void ModeTick(float DeltaTime) override;
	virtual void Exit() override;
	virtual void BindCommands() override;
	
	virtual void ActorSelectionChangeNotify() override;
	virtual void CreateToolkit() override;
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;

	void SetCurrentToolMode(EVelesLayersToolMode InNewToolMode);

	bool HasCoreActor() const;
	AVelesCore* GetCoreActor();

	bool CanEditCurrentTarget() const;
	AVelesLayerBaseActor* GetEditCurrentTarget() const;
	
	void UpdateSettings();
	void UpdateLayerPaintList();
	void RedrawWidgets();

	const TArray<FVelesLayerInfo>& GetLayersList() const;

	void SetUseBrush(bool bInUseBrush);
	void SetBrushLocation(bool bInBrushTraceValid, const FVector& InBrushLocation);

protected:

	// BRUSH

	bool bUseBrush;

	bool bBrushTraceValid;
	FVector BrushLocation;

	float DefaultBrushOpacity;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> BrushComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<AVelesCore> CoreActor;

	/** The dynamic material of the sphere brush. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BrushMID;
	
	FColor BrushDefaultHighlightColor;
	FColor BrushCurrentHighlightColor;

	//
	
	TArray<FVelesLayerInfo> LayersList;

	//
	void InitializeBrush();

	//
	bool bToolUpdated = false;
	
	void ForceRealTimeViewports(const bool bEnable);
	void DisableOverrideViewports();
};

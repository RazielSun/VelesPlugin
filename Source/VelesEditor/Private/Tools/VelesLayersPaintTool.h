// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayerPaintActor.h"
#include "VelesLayersBaseTool.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "VelesLayerEditDataInterface.h"

#include "VelesLayersPaintTool.generated.h"

class AVelesCore;
// 
// Paint Strokes
//
template<class ToolTarget>
class FVelesLayersToolStrokePaintBase : public FVelesLayersToolStrokeBase
{
public:
	FVelesLayersToolStrokePaintBase(UVelesLayersEditorMode* InEdMode, AVelesCore* InCore, AVelesLayerBaseActor* InTarget)
		: FVelesLayersToolStrokeBase(InEdMode, InCore, InTarget)
		, Cache(InCore, InTarget)
	{
	}

protected:
	typename ToolTarget::CacheClass Cache;
};

struct FVelesLayersPaintToolTarget
{
	typedef FLandscapeAlphaCache CacheClass;

	// @todo:
	// static FMatrix ToWorldMatrix(ULandscapeInfo* LandscapeInfo) { return FMatrix::Identity; }
	// static FMatrix FromWorldMatrix(ULandscapeInfo* LandscapeInfo) { return FMatrix::Identity; }
};

class FVelesLayersToolStrokePaint : public FVelesLayersToolStrokePaintBase<FVelesLayersPaintToolTarget>
{
	typedef FVelesLayersPaintToolTarget ToolTarget;

public:
	FVelesLayersToolStrokePaint(UVelesLayersEditorMode* InEdMode, AVelesCore* InCore, AVelesLayerBaseActor* InTarget)
		: FVelesLayersToolStrokePaintBase<FVelesLayersPaintToolTarget>(InEdMode, InCore, InTarget)
	{
	}

	void Apply(const FHitResult& InHit, bool bInvert);
	virtual float CalculateFalloff(float Distance, float Radius, float Falloff);
};

UCLASS()
class UVelesLayersPaintToolBuilder : public UVelesLayersBaseToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UVelesLayersBaseTool* CreateToolInstance(const FToolBuilderState& SceneState) const;
};

UCLASS(Transient)
class UVelesLayersPaintBrushProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UVelesLayersPaintBrushProperties();

	UPROPERTY(EditAnywhere, Category = "Hit Trace", meta = (DisplayName = "Heightfield"))
	bool bHeightfield;

	UPROPERTY(EditAnywhere, Category = "Hit Trace", meta = (DisplayName = "World Static"))
	bool bWorldStatic;

	UPROPERTY(EditAnywhere, Category = "Hit Trace", meta = (DisplayName = "World Dynamic"))
	bool bWorldDynamic;

	UPROPERTY(EditAnywhere, Category = "Hit Trace", meta = (DisplayName = "Has Veles Tag"))
	bool bHasVelesTag;
};

UCLASS()
class UVelesLayersPaintTool : public UVelesLayersBaseTool, public IClickDragBehaviorTarget, public IHoverBehaviorTarget
{
	GENERATED_BODY()

public:
	UVelesLayersPaintTool();

	virtual void Setup() override;

	virtual bool IsBrush() const override { return true; }

	// IClickDragBehaviorTarget implementation
	virtual FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& PressPos) override;
	virtual void OnClickPress(const FInputDeviceRay& PressPos) override;
	virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;
	virtual void OnClickRelease(const FInputDeviceRay& ReleasePos) override;
	virtual void OnTerminateDragSequence() override;
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;


	// IHoverBehaviorTarget implementation
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) override;
	virtual void OnBeginHover(const FInputDeviceRay& DevicePos) override {}
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;
	virtual void OnEndHover() override {}

protected:
	UPROPERTY()
	TObjectPtr<UVelesLayersPaintBrushProperties> Properties;
	
	/** Current state of the shift modifier toggle */
	bool bShiftToggle = false;
	
	FInputDeviceRay LastDeviceRay = FInputDeviceRay(FRay());

	bool bValidHitTrace = false;
	FHitResult ValidHitTrace;

	bool bTransactionStarted = false;
	TOptional<FVelesLayersToolStrokePaint> ToolStroke;

	void UpdateBrushLocation(const FInputDeviceRay& DevicePos);
	
	bool IsEqual(const FInputDeviceRay& InDevicePosA, const FInputDeviceRay& InDevicePosB) const;
};
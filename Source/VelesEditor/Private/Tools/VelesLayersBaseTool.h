// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"

#include "VelesLayersBaseTool.generated.h"

class AVelesCore;
class UVelesLayersEditorMode;
class UVelesLayersBaseTool;
class AVelesLayerBaseActor;
class AVelesLayerPaintActor;

// 
// Base Stroke
//
class FVelesLayersToolStrokeBase : protected FGCObject
{
public:
	FVelesLayersToolStrokeBase(UVelesLayersEditorMode* InEdMode, AVelesCore* InCore, AVelesLayerBaseActor* InTarget)
		: EdMode(InEdMode)
		, Core(InCore)
		, Target(InTarget)
	{
	}
	
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Core);
		Collector.AddReferencedObject(Target);
	}
	virtual FString GetReferencerName() const override
	{
		return TEXT("FVelesLayersBrushToolStrokeBase");
	}

protected:
	UVelesLayersEditorMode* EdMode = nullptr;
	AVelesCore* Core = nullptr;
	AVelesLayerBaseActor* Target = nullptr;
};

// 
// Base Builder
//
UCLASS()
class UVelesLayersBaseToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override { return true; }
	virtual UVelesLayersBaseTool* CreateToolInstance(const FToolBuilderState& SceneState) const;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;

protected:
	UVelesLayersEditorMode* GetEditorMode() const;
};

// 
// Base Tool
//
UCLASS()
class UVelesLayersBaseTool : public UInteractiveTool
{
	GENERATED_BODY()

public:
	UVelesLayersBaseTool();

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	void SetEdMode(UVelesLayersEditorMode* InEdMode);
	virtual void SetWorld(UWorld* World);

	virtual bool IsBrush() const { return false; }

protected:
	TWeakObjectPtr<UVelesLayersEditorMode> EdMode;
	UWorld* TargetWorld;
};

// 
// Setup Tool
//
UCLASS()
class UVelesLayersSetupToolBuilder : public UVelesLayersBaseToolBuilder
{
	GENERATED_BODY()

public:
	virtual UVelesLayersBaseTool* CreateToolInstance(const FToolBuilderState& SceneState) const override;
};

UCLASS()
class UVelesLayersSetupTool : public UVelesLayersBaseTool
{
	GENERATED_BODY()

public:
	UVelesLayersSetupTool();
};

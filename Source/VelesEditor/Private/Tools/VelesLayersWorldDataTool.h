// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayersBaseTool.h"

#include "VelesLayersWorldDataTool.generated.h"

UCLASS()
class UVelesLayersWorldDataToolBuilder : public UVelesLayersBaseToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UVelesLayersBaseTool* CreateToolInstance(const FToolBuilderState& SceneState) const override;
};

UCLASS()
class UVelesLayersWorldDataTool : public UVelesLayersBaseTool
{
	GENERATED_BODY()

public:
	UVelesLayersWorldDataTool();
};
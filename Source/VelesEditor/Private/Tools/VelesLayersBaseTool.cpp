// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayersBaseTool.h"

#include "InteractiveToolManager.h"
#include "VelesLayersEditorMode.h"

// localization namespace
#define LOCTEXT_NAMESPACE "VelesLayersBaseTool"

UVelesLayersBaseTool* UVelesLayersBaseToolBuilder::CreateToolInstance(const FToolBuilderState& SceneState) const
{
	return NewObject<UVelesLayersBaseTool>(SceneState.ToolManager);
}

UInteractiveTool* UVelesLayersBaseToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UVelesLayersBaseTool* NewTool = CreateToolInstance(SceneState);
	NewTool->SetEdMode(GetEditorMode());
	NewTool->SetWorld(SceneState.World);
	return NewTool;
}

UVelesLayersEditorMode* UVelesLayersBaseToolBuilder::GetEditorMode() const
{
	return Cast<UVelesLayersEditorMode>(GetOuter());
}

UVelesLayersBaseTool::UVelesLayersBaseTool()
{
}

void UVelesLayersBaseTool::Setup()
{
	Super::Setup();

	// if (EdMode.IsValid())
	// {
	// 	EdMode->RedrawWidgets();
	// }
}

void UVelesLayersBaseTool::Shutdown(EToolShutdownType ShutdownType)
{
	Super::Shutdown(ShutdownType);
}

void UVelesLayersBaseTool::SetEdMode(UVelesLayersEditorMode* InEdMode)
{
	EdMode = InEdMode;
}

void UVelesLayersBaseTool::SetWorld(UWorld* InWorld)
{
	TargetWorld = InWorld;
}

UVelesLayersBaseTool* UVelesLayersSetupToolBuilder::CreateToolInstance(const FToolBuilderState& SceneState) const
{
	return NewObject<UVelesLayersSetupTool>(SceneState.ToolManager);
}

UVelesLayersSetupTool::UVelesLayersSetupTool()
{
}

#undef LOCTEXT_NAMESPACE

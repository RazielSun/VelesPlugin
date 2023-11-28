// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayersWorldDataTool.h"

#include "InteractiveToolManager.h"
#include "VelesLayersEditorMode.h"

bool UVelesLayersWorldDataToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	if (const auto* EdMode = GetEditorMode())
		return EdMode->HasCoreActor();
	return Super::CanBuildTool(SceneState);
}

UVelesLayersBaseTool* UVelesLayersWorldDataToolBuilder::CreateToolInstance(const FToolBuilderState& SceneState) const
{
	return NewObject<UVelesLayersWorldDataTool>(SceneState.ToolManager);
}

UVelesLayersWorldDataTool::UVelesLayersWorldDataTool()
{
}

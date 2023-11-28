// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/VelesLayersEditorDetailCustomization_Base.h"

#include "EditorModeManager.h"
#include "InteractiveToolManager.h"
#include "VelesLayersEditorMode.h"

namespace VelesLayersCustomization
{
	UVelesLayersEditorMode* GetEditorMode()
	{
		return Cast<UVelesLayersEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(UVelesLayersEditorMode::EM_VelesLayersEditorModeId));
	}

	bool IsToolActive(const FString& ToolName)
	{
		UVelesLayersEditorMode* EdMode = GetEditorMode();
		if (EdMode)
		{
			return EdMode->GetToolManager()->GetActiveToolName(EToolSide::Left).Contains(ToolName);
		}

		return false;
	}

	// bool IsBrushSetActive(FName BrushSetName)
	// {
	// 	UVelesLayersEditorMode* EdMode = GetEditorMode();
	// 	if (EdMode && EdMode->CurrentBrushSetIndex >= 0)
	// 	{
	// 		const FName CurrentBrushSetName = EdMode->BrushSets[EdMode->CurrentBrushSetIndex].BrushSetName;
	// 		return CurrentBrushSetName == BrushSetName;
	// 	}
	//
	// 	return false;
	// }
}

bool FVelesLayersEditorDetailCustomization_Base::IsToolActive(const FString& ToolName)
{
	return VelesLayersCustomization::IsToolActive(ToolName);
}

UVelesLayersEditorMode* FVelesLayersEditorDetailCustomization_Base::GetEditorMode()
{
	return VelesLayersCustomization::GetEditorMode();
}

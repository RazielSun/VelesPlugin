// Copyright Epic Games, Inc. All Rights Reserved.

#include "VelesLayersEditorModeCommands.h"
#include "VelesLayersEditorMode.h"
#include "EditorStyleSet.h"
#include "Styles/VelesEditorStyle.h"

#define LOCTEXT_NAMESPACE "VelesLayersPluginEditorModeCommands"

FVelesLayersEditorModeCommands::FVelesLayersEditorModeCommands()
	: TCommands<FVelesLayersEditorModeCommands>("VelesLayersEditorModeCommands",
		NSLOCTEXT("VelesLayersEditorModeCommands", "VelesLayersEditorModeCommands", "VelesLayers Editor Mode Commands"),
		NAME_None,
		FVelesEditorStyle::GetStyleSetName())
{
}

void FVelesLayersEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(SetupTool, "Setup", "Create new Veles Core and Setup Map Parameters", EUserInterfaceActionType::RadioButton, FInputChord());
	ToolCommands.Add(SetupTool);

	UI_COMMAND(PaintTool, "Paint", "Create new VelesLayerPaint and paint", EUserInterfaceActionType::RadioButton, FInputChord());
	ToolCommands.Add(PaintTool);

	UI_COMMAND(WorldDataTool, "World", "Create new VelesLayerWorldData", EUserInterfaceActionType::RadioButton, FInputChord());
	ToolCommands.Add(WorldDataTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FVelesLayersEditorModeCommands::GetCommands()
{
	return FVelesLayersEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE

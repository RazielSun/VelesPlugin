// Copyright Epic Games, Inc. All Rights Reserved.

#include "VelesEditorCommands.h"

#include "Styles/VelesEditorStyle.h"

#define LOCTEXT_NAMESPACE "FVelesEditorModule"

FVelesEditorCommands::FVelesEditorCommands()
: TCommands<FVelesEditorCommands>(TEXT("VelesPlugin"), NSLOCTEXT("Contexts", "VelesPlugin", "VelesPlugin Plugin"), NAME_None, FVelesEditorStyle::GetStyleSetName())
{
}

void FVelesEditorCommands::RegisterCommands()
{
	UI_COMMAND(RebuildAllAction, "Rebuild All", "Rebuild All Schemes", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

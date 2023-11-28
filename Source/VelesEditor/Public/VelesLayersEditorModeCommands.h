// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * This class contains info about the full set of commands used in this editor mode.
 */
class FVelesLayersEditorModeCommands : public TCommands<FVelesLayersEditorModeCommands>
{
public:
	FVelesLayersEditorModeCommands();

	virtual void RegisterCommands() override;
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();

	TSharedPtr<FUICommandInfo> SetupTool;
	TSharedPtr<FUICommandInfo> PaintTool;
	TSharedPtr<FUICommandInfo> WorldDataTool;

protected:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands;
};

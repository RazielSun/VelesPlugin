// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FVelesEditorCommands : public TCommands<FVelesEditorCommands>
{
public:

	FVelesEditorCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > RebuildAllAction;
};

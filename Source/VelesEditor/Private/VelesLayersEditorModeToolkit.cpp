// Copyright Epic Games, Inc. All Rights Reserved.

#include "VelesLayersEditorModeToolkit.h"
#include "VelesLayersEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"
#include "InteractiveToolManager.h"
#include "SlateOptMacros.h"
#include "VelesLayersEditorObject.h"
#include "Tools/VelesLayersBaseTool.h"

#define LOCTEXT_NAMESPACE "VelesLayersEditorModeToolkit"

FVelesLayersEditorModeToolkit::FVelesLayersEditorModeToolkit()
{
}

void FVelesLayersEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	EditorWidgets = SNew(SVelesLayersEditor, SharedThis(this));
	
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FVelesLayersEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}

TSharedPtr<SWidget> FVelesLayersEditorModeToolkit::GetInlineContent() const
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			ModeDetailsView.ToSharedRef()
		]
		+ SVerticalBox::Slot()
		[
			DetailsView.ToSharedRef()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			EditorWidgets.ToSharedRef()
		];
}

FName FVelesLayersEditorModeToolkit::GetToolkitFName() const
{
	return FName("VelesLayersEditorMode");
}

FText FVelesLayersEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "Veles Layers Toolkit");
}

void FVelesLayersEditorModeToolkit::NotifyToolChanged()
{
	EditorWidgets->NotifyToolChanged();
}

#undef LOCTEXT_NAMESPACE


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SVelesLayersEditor::Construct(const FArguments& InArgs, TSharedRef<FVelesLayersEditorModeToolkit> InParentToolkit)
{
	ParentToolkit = InParentToolkit;
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;
	
	DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateSP(this, &SVelesLayersEditor::GetIsPropertyVisible));
	
	UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		DetailsPanel->SetObject(EdMode->UISettings);
	}
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 5)
		[
			SAssignNew(Error, SErrorText)
		]
		+ SVerticalBox::Slot()
		.Padding(0)
		[
			SNew(SVerticalBox)
			.IsEnabled(this, &SVelesLayersEditor::GetLayersEditorIsEnabled)
			+ SVerticalBox::Slot()
			.Padding(0)
			[
				DetailsPanel.ToSharedRef()
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SVelesLayersEditor::NotifyToolChanged()
{
	RefreshDetailPanel();
}

void SVelesLayersEditor::NotifyBrushChanged()
{
	RefreshDetailPanel();
}

void SVelesLayersEditor::RefreshDetailPanel()
{
	UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		// Refresh details panel
		DetailsPanel->SetObject(EdMode->UISettings, true);
	}
}

UVelesLayersEditorMode* SVelesLayersEditor::GetEditorMode() const
{
	return Cast<UVelesLayersEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(UVelesLayersEditorMode::EM_VelesLayersEditorModeId));
}


FText SVelesLayersEditor::GetErrorText() const
{
	const UVelesLayersEditorMode* EdMode = GetEditorMode();
	// ELandscapeEditingState EditState = LandscapeEdMode->GetEditingState();
	// switch (EditState)
	// {
	// case ELandscapeEditingState::SIEWorld:
	// 	{
	//
	// 		if (LandscapeEdMode->NewLandscapePreviewMode != ENewLandscapePreviewMode::None)
	// 		{
	// 			return LOCTEXT("IsSimulatingError_create", "Can't create landscape while simulating!");
	// 		}
	// 		else
	// 		{
	// 			return LOCTEXT("IsSimulatingError_edit", "Can't edit landscape while simulating!");
	// 		}
	// 		break;
	// 	}
	// case ELandscapeEditingState::PIEWorld:
	// 	{
	// 		if (LandscapeEdMode->NewLandscapePreviewMode != ENewLandscapePreviewMode::None)
	// 		{
	// 			return LOCTEXT("IsPIEError_create", "Can't create landscape in PIE!");
	// 		}
	// 		else
	// 		{
	// 			return LOCTEXT("IsPIEError_edit", "Can't edit landscape in PIE!");
	// 		}
	// 		break;
	// 	}
	// case ELandscapeEditingState::BadFeatureLevel:
	// 	{
	// 		if (LandscapeEdMode->NewLandscapePreviewMode != ENewLandscapePreviewMode::None)
	// 		{
	// 			return LOCTEXT("IsFLError_create", "Can't create landscape with a feature level less than SM4!");
	// 		}
	// 		else
	// 		{
	// 			return LOCTEXT("IsFLError_edit", "Can't edit landscape with a feature level less than SM4!");
	// 		}
	// 		break;
	// 	}
	// case ELandscapeEditingState::NoLandscape:
	// 	{
	// 		return LOCTEXT("NoLandscapeError", "No Landscape!");
	// 	}
	// case ELandscapeEditingState::Enabled:
	// 	{
	// 		return FText::GetEmpty();
	// 	}
	// default:
	// 	checkNoEntry();
	// }

	return FText::GetEmpty();
}

bool SVelesLayersEditor::GetLayersEditorIsEnabled() const
{
	UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		Error->SetError(GetErrorText());
		// return EdMode->GetEditingState() == ELandscapeEditingState::Enabled;
		// @todo: visible from World State
		return true;
	}
	return false;
}

bool SVelesLayersEditor::GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	return ParentToolkit.Pin()->GetIsPropertyVisibleFromProperty(PropertyAndParent.Property);
}

UVelesLayersEditorMode* FVelesLayersEditorModeToolkit::GetEditorModeAdv() const
{
	return Cast<UVelesLayersEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(UVelesLayersEditorMode::EM_VelesLayersEditorModeId));
}

bool FVelesLayersEditorModeToolkit::GetIsPropertyVisibleFromProperty(const FProperty& Property) const
{
	UVelesLayersEditorMode* EdMode = GetEditorModeAdv();
	
	if (EdMode != nullptr && EdMode->CurrentToolMode != EVelesLayersToolMode::None)
	{
	    // Hide all properties if the current target can't be edited. Except in New Landscape tool
		if (EdMode->CurrentToolMode != EVelesLayersToolMode::NewActor &&
			!EdMode->CanEditCurrentTarget())
		{
			return false;
		}
		
		if (Property.HasMetaData("ShowForTools"))
		{
			const FString CurrentToolName = EdMode->GetToolManager()->GetActiveToolName(EToolSide::Left);
	 
			TArray<FString> ShowForTools;
			Property.GetMetaData("ShowForTools").ParseIntoArray(ShowForTools, TEXT(","), true);
			if (!ShowForTools.Contains(CurrentToolName))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

void FVelesLayersEditorModeToolkit::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolStarted(Manager, Tool);

	NotifyToolChanged();

	if (UVelesLayersBaseTool* BaseTool = Cast<UVelesLayersBaseTool>(Tool))
	{
		UVelesLayersEditorMode* EdMode = GetEditorModeAdv();
		if (EdMode)
		{
			EdMode->SetUseBrush(BaseTool->IsBrush());
		}
	}
}

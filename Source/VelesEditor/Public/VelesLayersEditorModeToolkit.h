// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "VelesLayersEditorMode.h"
#include "Widgets/SCompoundWidget.h"

class SVelesLayersEditor;
class UVelesLayersEditorMode;

class FVelesLayersEditorModeToolkit : public FModeToolkit
{
public:
	FVelesLayersEditorModeToolkit();

	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;

	virtual UVelesLayersEditorMode* GetEditorModeAdv() const;
	virtual TSharedPtr<SWidget> GetInlineContent() const override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;

	void NotifyToolChanged();

	bool GetIsPropertyVisibleFromProperty(const FProperty& Property) const;

protected:
	virtual void OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;
	
private:
	TSharedPtr<SVelesLayersEditor> EditorWidgets;
};


class SVelesLayersEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SVelesLayersEditor ){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FVelesLayersEditorModeToolkit> InParentToolkit);
	
	void NotifyToolChanged();
	void NotifyBrushChanged();
	void RefreshDetailPanel();

protected:
	class UVelesLayersEditorMode* GetEditorMode() const;

	FText GetErrorText() const;

	bool GetLayersEditorIsEnabled() const;

	bool GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;

protected:
	TSharedPtr<SErrorText> Error;
	TSharedPtr<IDetailsView> DetailsPanel;
	TWeakPtr<FVelesLayersEditorModeToolkit> ParentToolkit;
};

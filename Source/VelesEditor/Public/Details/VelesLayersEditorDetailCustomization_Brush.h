// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayersEditorDetailCustomization_Base.h"


class FVelesLayersEditorDetailCustomization_Brush : public FVelesLayersEditorDetailCustomization_Base
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<SWidget> GetTargetListMenu(TSharedRef<IPropertyHandle> PropertyHandle);
	static void OnChangeTargetList(TSharedRef<IPropertyHandle> PropertyHandle, int32 NewSize);
	static FText GetTargetList(TSharedRef<IPropertyHandle> PropertyHandle);

	FReply ClearLayerClicked();
};

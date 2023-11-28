// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayersEditorDetailCustomization_Base.h"

enum class ENewVelesLayerMode : uint8;

class FVelesLayersEditorDetailCustomization_NewLayer : public FVelesLayersEditorDetailCustomization_Base
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static void SetScale(FVector::FReal NewValue, ETextCommit::Type, TSharedRef<IPropertyHandle> PropertyHandle);

	static TSharedRef<SWidget> GetComponentSizeMenu(TSharedRef<IPropertyHandle> PropertyHandle);
	static void OnChangeComponentSize(TSharedRef<IPropertyHandle> PropertyHandle, int32 NewSize);
	static FText GetComponentSize(TSharedRef<IPropertyHandle> PropertyHandle);
	
	// From Landscape for test
	static EVisibility GetVisibilityOnlyInNewMode();
	FText GetTotalComponentCount() const;

	//
	bool CanEditNewLayerProperty() const;
	static EVisibility NewLayerVisibility();

	//
	static void SetNewLayerName(const FText& NewValue, ETextCommit::Type CommitInfo, TSharedRef<IPropertyHandle> PropertyHandle_HeightmapFilename);

	bool IsCreateButtonEnabled() const;
	
	FReply OnCreateButtonClicked();
};

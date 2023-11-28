// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayersEditorDetailCustomization_Base.h"

class FVelesLayersEditorDetailCustomization_Setup : public FVelesLayersEditorDetailCustomization_Base
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static EVisibility GetVisibilityOnlyInNoCore();
	bool CanCreateNewCore() const;

	static TSharedRef<SWidget> GetComponentSizeMenu(TSharedRef<IPropertyHandle> PropertyHandle);
	static void OnChangeComponentSize(TSharedRef<IPropertyHandle> PropertyHandle, int32 NewSize);
	static FText GetComponentSize(TSharedRef<IPropertyHandle> PropertyHandle);

	static void SetScale(FVector::FReal NewValue, ETextCommit::Type, TSharedRef<IPropertyHandle> PropertyHandle);

	FText GetTotalComponentCount() const;

	bool IsCreateButtonEnabled() const;
	FReply OnCreateButtonClicked();
};

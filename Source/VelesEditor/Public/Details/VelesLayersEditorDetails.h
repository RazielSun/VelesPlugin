// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VelesLayersEditorDetailCustomization_Base.h"

class FVelesLayersEditorDetailCustomization_Setup;
class FVelesLayersEditorDetailCustomization_NewLayer;
class FVelesLayersEditorDetailCustomization_Brush;

class FVelesLayersEditorDetails : public FVelesLayersEditorDetailCustomization_Base
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	TSharedPtr<FVelesLayersEditorDetailCustomization_Setup> Customization_Setup;
	TSharedPtr<FVelesLayersEditorDetailCustomization_NewLayer> Customization_NewActor;
	TSharedPtr<FVelesLayersEditorDetailCustomization_Brush> Customization_Brush;
};

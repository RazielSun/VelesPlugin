// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/VelesLayersEditorDetails.h"

#include "Details/VelesLayersEditorDetailCustomization_NewLayer.h"
#include "Details/VelesLayersEditorDetailCustomization_Brush.h"
#include "Details/VelesLayersEditorDetailCustomization_Setup.h"


TSharedRef<IDetailCustomization> FVelesLayersEditorDetails::MakeInstance()
{
	return MakeShareable(new FVelesLayersEditorDetails);
}

void FVelesLayersEditorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Tools:
	Customization_Setup = MakeShareable(new FVelesLayersEditorDetailCustomization_Setup);
	Customization_Setup->CustomizeDetails(DetailBuilder);
	
	Customization_NewActor = MakeShareable(new FVelesLayersEditorDetailCustomization_NewLayer);
	Customization_NewActor->CustomizeDetails(DetailBuilder);

	Customization_Brush = MakeShareable(new FVelesLayersEditorDetailCustomization_Brush);
	Customization_Brush->CustomizeDetails(DetailBuilder);
}
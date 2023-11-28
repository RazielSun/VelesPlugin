// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesEditorStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FVelesEditorStyle::StyleInstance = nullptr;

void FVelesEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FVelesEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FVelesEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("VelesEditorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FVelesEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("VelesPlugin")->GetBaseDir() / TEXT("Resources"));
	
	Style->Set("VelesPlugin.RebuildAllAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	Style->Set("VelesLayersEditorModeCommands.SetupTool", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("VelesLayersEditorModeCommands.PaintTool", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("VelesLayersEditorModeCommands.WorldDataTool", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	
	return Style;
}

void FVelesEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FVelesEditorStyle::Get()
{
	return *StyleInstance;
}

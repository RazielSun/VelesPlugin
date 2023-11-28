// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/VelesLayersEditorDetailCustomization_Brush.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "SlateOptMacros.h"
#include "VelesCoreSubsystem.h"
#include "VelesLayerBaseActor.h"
#include "VelesLayerComponent.h"
#include "VelesLayersEditorMode.h"
#include "VelesLayersEditorObject.h"

#define LOCTEXT_NAMESPACE "LandscapeEditor.NewLandscape"

TSharedRef<IDetailCustomization> FVelesLayersEditorDetailCustomization_Brush::MakeInstance()
{
	return MakeShareable(new FVelesLayersEditorDetailCustomization_Brush);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FVelesLayersEditorDetailCustomization_Brush::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	if (!IsToolActive(TEXT("Paint")))
	{
		return;
	}
	
	IDetailCategoryBuilder& EditingCategory = DetailBuilder.EditCategory("Editing");

	TSharedRef<IPropertyHandle> PropertyHandle_TargetIndex = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVelesLayersEditorObject, CurrentTargetIndex));
	EditingCategory.AddProperty(PropertyHandle_TargetIndex)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle_TargetIndex->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SComboButton)
		.OnGetMenuContent_Static(&GetTargetListMenu, PropertyHandle_TargetIndex)
		.ContentPadding(2)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text_Static(&GetTargetList, PropertyHandle_TargetIndex)
		]
	];

	EditingCategory.AddCustomRow(FText::GetEmpty())
	.WholeRowContent()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Left)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(4, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.OnClicked(this, &FVelesLayersEditorDetailCustomization_Brush::ClearLayerClicked)
			.Text(LOCTEXT("ClearButton", "Clear Layer"))
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> FVelesLayersEditorDetailCustomization_Brush::GetTargetListMenu(
	TSharedRef<IPropertyHandle> PropertyHandle)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		int32 Index = 0;
		for (const auto& LayerInfo : EdMode->GetLayersList())
		{
			MenuBuilder.AddMenuEntry(FText::Format(LOCTEXT("LayerInfoName", "{0} ({1})"), FText::FromString(LayerInfo.LayerName), FText::FromString(LayerInfo.LayerActor->GetName())), FText::GetEmpty(),
			FSlateIcon(), FExecuteAction::CreateStatic(&OnChangeTargetList, PropertyHandle, Index));
			Index++;
		}
	}
	
	return MenuBuilder.MakeWidget();
}

void FVelesLayersEditorDetailCustomization_Brush::OnChangeTargetList(TSharedRef<IPropertyHandle> PropertyHandle,
	int32 NewSize)
{
	ensure(PropertyHandle->SetValue(NewSize) == FPropertyAccess::Success);

	UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode && EdMode->CanEditCurrentTarget() && GEditor)
	{
		GEditor->SelectActor(EdMode->GetEditCurrentTarget(), /*bSelected*/true, /*bNotify*/true, /*bSelectEvenIfHidden*/true);
	}
}

FText FVelesLayersEditorDetailCustomization_Brush::GetTargetList(TSharedRef<IPropertyHandle> PropertyHandle)
{
	int32 TargetListItem = 0;
	FPropertyAccess::Result Result = PropertyHandle->GetValue(TargetListItem);
	check(Result == FPropertyAccess::Success);

	if (Result == FPropertyAccess::MultipleValues)
	{
		return NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values");
	}

	UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		if (EdMode->GetLayersList().Num() > TargetListItem && TargetListItem >= 0)
		{
			const auto& LayerInfo = EdMode->GetLayersList()[TargetListItem];
			if (LayerInfo.LayerActor.IsValid())
				return FText::Format(LOCTEXT("LayerInfoName", "{0} ({1})"), FText::FromString(LayerInfo.LayerName), FText::FromString(LayerInfo.LayerActor->GetName()));
		}
	}

	return LOCTEXT("LayerInfoName", "None");
}

FReply FVelesLayersEditorDetailCustomization_Brush::ClearLayerClicked()
{
	UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		if (EdMode->CanEditCurrentTarget())
		{
			AVelesLayerBaseActor* LayerActor = EdMode->GetEditCurrentTarget();
			if (LayerActor)
			{
				const TArray<TObjectPtr<UVelesLayerComponent>>& Components = LayerActor->GetLayerComponents();
				for (auto& Component : Components)
				{
					UTexture2D* EditToolTexture = Component->GetComponentTexture();
					if (EditToolTexture)
					{
						Component->ClearComponentTexture();
						Component->RequestDataUpdate();
					}
				}
			}
		}

		UVelesCoreSubsystem::LayerPaintChangedEvent.Broadcast();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

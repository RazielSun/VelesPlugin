// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/VelesLayersEditorDetailCustomization_NewLayer.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "SlateOptMacros.h"
#include "SPrimaryButton.h"
#include "TutorialMetaData.h"
#include "VelesCore.h"
#include "VelesLayerPaintActor.h"
#include "VelesLayersEditorMode.h"
#include "VelesLayersEditorObject.h"
#include "VelesLayerWorldDataActor.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SSegmentedControl.h"

#define LOCTEXT_NAMESPACE "VelesLayersEditor.NewLayer"

TSharedRef<IDetailCustomization> FVelesLayersEditorDetailCustomization_NewLayer::MakeInstance()
{
	return MakeShareable(new FVelesLayersEditorDetailCustomization_NewLayer);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FVelesLayersEditorDetailCustomization_NewLayer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const bool bAllowed = IsToolActive("Paint") || IsToolActive("WorldData");
	if (!bAllowed)
	{
		return;
	}
	
	IDetailCategoryBuilder& NewActorCategory = DetailBuilder.EditCategory("Create New");

	TAttribute<EVisibility> SettingsVisibility(
			TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateStatic(NewLayerVisibility))
		);

	// NewActorCategory.AddCustomRow(FText::GetEmpty())
	// .Visibility(SettingsVisibility)
	// [
	// 	SNew(SBox)
	// 	.Padding(2.0f)
	// 	.HAlign(HAlign_Center)
	// 	[
	// 		SNew(SSegmentedControl<ENewVelesLayerMode>)
	// 		.Value_Lambda([this]()
	// 		{
	// 			UVelesLayersEditorMode* EdMode = GetEditorMode();
	// 			return EdMode ? EdMode->NewVelesLayerMode : ENewVelesLayerMode::NewLayer;
	//
	// 		})
	// 		.OnValueChanged_Lambda([this](ENewVelesLayerMode Mode)
	// 		{
	// 			UVelesLayersEditorMode* EdMode = GetEditorMode();
	// 			if (EdMode != nullptr)
	// 			{
	// 				EdMode->NewVelesLayerMode = Mode;
	// 			}
	// 		})
	// 		+SSegmentedControl<ENewVelesLayerMode>::Slot(ENewVelesLayerMode::NewLayer)
	// 		.Text(LOCTEXT("NewLayer", "New Layer"))
	// 		+ SSegmentedControl<ENewVelesLayerMode>::Slot(ENewVelesLayerMode::NewWorldData)
	// 		.Text(LOCTEXT("NewWorldData", "New WorldData"))
	// 	]
	// 	
	// ];

	TSharedRef<IPropertyHandle> PropertyHandle_LayerName = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVelesLayersEditorObject, NewActor_LayerName));
	NewActorCategory.AddProperty(PropertyHandle_LayerName)
	// .Visibility(SettingsVisibility)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle_LayerName->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	.MaxDesiredWidth(0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(SEditableTextBox)
			.Font(DetailBuilder.GetDetailFont())
			.Text_Static(&GetPropertyValueText, PropertyHandle_LayerName)
			.OnTextCommitted_Static(&SetNewLayerName, PropertyHandle_LayerName)
			.HintText(LOCTEXT("NewActor_LayerName", "(Please insert Layer Name)"))
		]
	];

	TSharedRef<IPropertyHandle> PropertyHandle_ComponentCount = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVelesLayersEditorObject, NewActor_ComponentCount));
	TSharedRef<IPropertyHandle> PropertyHandle_ComponentCount_X = PropertyHandle_ComponentCount->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_ComponentCount_Y = PropertyHandle_ComponentCount->GetChildHandle("Y").ToSharedRef();
	NewActorCategory.AddProperty(PropertyHandle_ComponentCount)
	.EditCondition(CanEditNewLayerProperty(), nullptr)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle_ComponentCount->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(SNumericEntryBox<int32>)
			.Font(DetailBuilder.GetDetailFont())
			.MinValue(1)
			.MaxValue(32)
			.MinSliderValue(1)
			.MaxSliderValue(32)
			.AllowSpin(true)
			.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
			.Value_Static(&FVelesLayersEditorDetailCustomization_Base::OnGetValue<int32>, PropertyHandle_ComponentCount_X)
			.OnValueChanged_Static(&FVelesLayersEditorDetailCustomization_Base::OnValueChanged<int32>, PropertyHandle_ComponentCount_X)
			.OnValueCommitted_Static(&FVelesLayersEditorDetailCustomization_Base::OnValueCommitted<int32>, PropertyHandle_ComponentCount_X)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(FText::FromString(FString().AppendChar(0xD7))) // Multiply sign
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(SNumericEntryBox<int32>)
			.Font(DetailBuilder.GetDetailFont())
			.MinValue(1)
			.MaxValue(32)
			.MinSliderValue(1)
			.MaxSliderValue(32)
			.AllowSpin(true)
			.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
			.Value_Static(&FVelesLayersEditorDetailCustomization_Base::OnGetValue<int32>, PropertyHandle_ComponentCount_Y)
			.OnValueChanged_Static(&FVelesLayersEditorDetailCustomization_Base::OnValueChanged<int32>, PropertyHandle_ComponentCount_Y)
			.OnValueCommitted_Static(&FVelesLayersEditorDetailCustomization_Base::OnValueCommitted<int32>, PropertyHandle_ComponentCount_Y)
		]
	];

	TSharedRef<IPropertyHandle> PropertyHandle_ComponentSize = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVelesLayersEditorObject, NewActor_ComponentSize));
	NewActorCategory.AddProperty(PropertyHandle_ComponentSize)
	.EditCondition(CanEditNewLayerProperty(), nullptr)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle_ComponentSize->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SComboButton)
		.OnGetMenuContent_Static(&GetComponentSizeMenu, PropertyHandle_ComponentSize)
		.ContentPadding(2)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text_Static(&GetComponentSize, PropertyHandle_ComponentSize)
		]
	];

	using SNumericVector2DInputBox = SNumericVectorInputBox<FVector2d::FReal, FVector2d, 2>;
	
	TSharedRef<IPropertyHandle> PropertyHandle_Location = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVelesLayersEditorObject, NewActor_WorldOffset));
	TSharedRef<IPropertyHandle> PropertyHandle_Location_X = PropertyHandle_Location->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Location_Y = PropertyHandle_Location->GetChildHandle("Y").ToSharedRef();
	NewActorCategory.AddProperty(PropertyHandle_Location)
	.EditCondition(CanEditNewLayerProperty(), nullptr)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle_Location->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(125.0f * 3.0f) // copied from FComponentTransformDetails
	.MaxDesiredWidth(125.0f * 3.0f)
	[
		SNew(SNumericVector2DInputBox)
		.bColorAxisLabels(true)
		.Font(DetailBuilder.GetDetailFont())
		.X_Static(&GetOptionalPropertyValue<FVector2D::FReal>, PropertyHandle_Location_X)
		.Y_Static(&GetOptionalPropertyValue<FVector2D::FReal>, PropertyHandle_Location_Y)
		.OnXCommitted_Static(&SetPropertyValue<FVector2D::FReal>, PropertyHandle_Location_X)
		.OnYCommitted_Static(&SetPropertyValue<FVector2D::FReal>, PropertyHandle_Location_Y)
		.OnXChanged_Lambda([=](FVector2D::FReal NewValue) { ensure(PropertyHandle_Location_X->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange) == FPropertyAccess::Success); })
		.OnYChanged_Lambda([=](FVector2D::FReal NewValue) { ensure(PropertyHandle_Location_Y->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange) == FPropertyAccess::Success); })
		.AllowSpin(true)
	];

	TSharedRef<IPropertyHandle> PropertyHandle_Scale = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVelesLayersEditorObject, NewActor_WorldScale));
	TSharedRef<IPropertyHandle> PropertyHandle_Scale_X = PropertyHandle_Scale->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_Scale_Y = PropertyHandle_Scale->GetChildHandle("Y").ToSharedRef();
	NewActorCategory.AddProperty(PropertyHandle_Scale)
	.EditCondition(CanEditNewLayerProperty(), nullptr)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle_Scale->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(125.0f * 3.0f) // copied from FComponentTransformDetails
	.MaxDesiredWidth(125.0f * 3.0f)
	[
		SNew(SNumericVector2DInputBox)
		.bColorAxisLabels(true)
		.Font(DetailBuilder.GetDetailFont())
		.X_Static(&GetOptionalPropertyValue<FVector2D::FReal>, PropertyHandle_Scale_X)
		.Y_Static(&GetOptionalPropertyValue<FVector2D::FReal>, PropertyHandle_Scale_Y)
		.OnXCommitted_Static(&SetScale, PropertyHandle_Scale_X)
		.OnYCommitted_Static(&SetScale, PropertyHandle_Scale_Y)
		.OnXChanged_Lambda([=](FVector2D::FReal NewValue) { ensure(PropertyHandle_Scale_X->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange) == FPropertyAccess::Success); })
		.OnYChanged_Lambda([=](FVector2D::FReal NewValue) { ensure(PropertyHandle_Scale_Y->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange) == FPropertyAccess::Success); })
		.AllowSpin(true)
	];

	NewActorCategory.AddCustomRow(LOCTEXT("TotalComponents", "Total Components"))
	// .Visibility(SettingsVisibility)
	.RowTag("LandscapeEditor.TotalComponents")
	.NameContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		.Padding(FMargin(2))
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("TotalComponents", "Total Components"))
			.ToolTipText(LOCTEXT("NewLandscape_TotalComponents", "The total number of components that will be created for this landscape."))
		]
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	[
		SNew(SEditableTextBox)
		.IsReadOnly(true)
		.Font(DetailBuilder.GetDetailFont())
		.Text(this, &FVelesLayersEditorDetailCustomization_NewLayer::GetTotalComponentCount)
	];

	NewActorCategory.AddCustomRow(FText::GetEmpty())
	// .Visibility(SettingsVisibility)
	.WholeRowContent()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Right)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(4, 0)
		.AutoWidth()
		[
			SNew(SPrimaryButton)
			.Visibility_Static(&GetVisibilityOnlyInNewMode)
			.Text(LOCTEXT("Create", "Create"))
			.AddMetaData<FTutorialMetaData>(FTutorialMetaData(TEXT("CreateButton"), TEXT("LevelEditorToolBox")))
			.OnClicked(this, &FVelesLayersEditorDetailCustomization_NewLayer::OnCreateButtonClicked)
			.IsEnabled(this, &FVelesLayersEditorDetailCustomization_NewLayer::IsCreateButtonEnabled)
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


void FVelesLayersEditorDetailCustomization_NewLayer::SetScale(FVector::FReal NewValue, ETextCommit::Type, TSharedRef<IPropertyHandle> PropertyHandle)
{
	FVector::FReal OldValue = 0;
	PropertyHandle->GetValue(OldValue);

	if (NewValue == 0)
	{
		if (OldValue < 0)
		{
			NewValue = -1;
		}
		else
		{
			NewValue = 1;
		}
	}

	ensure(PropertyHandle->SetValue(NewValue) == FPropertyAccess::Success);

	// Make X and Y scale match
	FName PropertyName = PropertyHandle->GetProperty()->GetFName();
	if (PropertyName == "X")
	{
		TSharedRef<IPropertyHandle> PropertyHandle_Y = PropertyHandle->GetParentHandle()->GetChildHandle("Y").ToSharedRef();
		ensure(PropertyHandle_Y->SetValue(NewValue) == FPropertyAccess::Success);
	}
	else if (PropertyName == "Y")
	{
		TSharedRef<IPropertyHandle> PropertyHandle_X = PropertyHandle->GetParentHandle()->GetChildHandle("X").ToSharedRef();
		ensure(PropertyHandle_X->SetValue(NewValue) == FPropertyAccess::Success);
	}
}

TSharedRef<SWidget> FVelesLayersEditorDetailCustomization_NewLayer::GetComponentSizeMenu(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	int32 ComponentSizeQuadsValues[5] = { 32, 64, 128, 256, 512 };

	for (int32 i = 0; i < UE_ARRAY_COUNT(ComponentSizeQuadsValues); i++)
	{
		MenuBuilder.AddMenuEntry(FText::Format(LOCTEXT("NxNQuads", "{0}\u00D7{0}"), FText::AsNumber(ComponentSizeQuadsValues[i])), FText::GetEmpty(),
			FSlateIcon(), FExecuteAction::CreateStatic(&OnChangeComponentSize, PropertyHandle, ComponentSizeQuadsValues[i]));
	}

	return MenuBuilder.MakeWidget();
}

void FVelesLayersEditorDetailCustomization_NewLayer::OnChangeComponentSize(TSharedRef<IPropertyHandle> PropertyHandle, int32 NewSize)
{
	ensure(PropertyHandle->SetValue(NewSize) == FPropertyAccess::Success);
}

FText FVelesLayersEditorDetailCustomization_NewLayer::GetComponentSize(TSharedRef<IPropertyHandle> PropertyHandle)
{
	int32 QuadsPerSection = 0;
	FPropertyAccess::Result Result = PropertyHandle->GetValue(QuadsPerSection);
	check(Result == FPropertyAccess::Success);

	if (Result == FPropertyAccess::MultipleValues)
	{
		return NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values");
	}

	return FText::Format(LOCTEXT("NxNQuads", "{0}\u00D7{0}"), FText::AsNumber(QuadsPerSection));
}

EVisibility FVelesLayersEditorDetailCustomization_NewLayer::GetVisibilityOnlyInNewMode()
{
	return EVisibility::Visible;
}

FText FVelesLayersEditorDetailCustomization_NewLayer::GetTotalComponentCount() const
{
	const UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode != nullptr)
	{
		return FText::AsNumber(EdMode->UISettings->NewActor_ComponentCount.X * EdMode->UISettings->NewActor_ComponentCount.Y);
	}

	return FText::FromString(TEXT("---"));
}

bool FVelesLayersEditorDetailCustomization_NewLayer::CanEditNewLayerProperty() const
{
	// UVelesLayersEditorMode* EdMode = GetEditorMode();
	// if (EdMode)
	// {
	// 	return !EdMode->CanEditCurrentTarget();
	// }
	return false;
}

EVisibility FVelesLayersEditorDetailCustomization_NewLayer::NewLayerVisibility()
{
	return (IsToolActive("Paint") || IsToolActive("WorldData")) ? EVisibility::Visible : EVisibility::Hidden;
}

FReply FVelesLayersEditorDetailCustomization_NewLayer::OnCreateButtonClicked()
{
	UVelesLayersEditorMode* EdMode = GetEditorMode();

	bool bCreated = false;
	AActor* CreatedActor = nullptr;
	
	if (EdMode != nullptr && 
		EdMode->GetWorld() != nullptr && 
		EdMode->GetWorld()->GetCurrentLevel()->bIsVisible &&
		EdMode->HasCoreActor())
	{
		AVelesCore* CoreActor = EdMode->GetCoreActor();
		UVelesLayersEditorObject* UISettings = EdMode->UISettings;
		// const int32 ComponentCountX = UISettings->NewActor_ComponentCount.X;
		// const int32 ComponentCountY = UISettings->NewActor_ComponentCount.Y;
		// const int32 ComponentSize = UISettings->NewActor_ComponentSize;
		//
		const FVector WorldScale3D = FVector(UISettings->NewActor_WorldScale, 1.0);
		// const FIntPoint ComponentsSize = FIntPoint(ComponentCountX, ComponentCountY) * ComponentSize;
		// const FVector Offset = FTransform(FQuat::Identity, FVector::ZeroVector, WorldScale3D).TransformVector(FVector(-ComponentsSize.X / 2, -ComponentsSize.Y / 2, 0));
		//
		const FVector WorldLocation = CoreActor->WorldLocation;
		const FTransform WorldTransform = FTransform(FQuat::Identity, WorldLocation, WorldScale3D);
		
		{
			FScopedTransaction Transaction(LOCTEXT("Undo", "Creating New Layer"));
			
			AVelesLayerBaseActor* NewActor = nullptr;
			FString NewActorName;

			if (IsToolActive("Paint"))
			{
				AVelesLayerPaintActor* NewLayerActor = EdMode->GetWorld()->SpawnActor<AVelesLayerPaintActor>(AVelesLayerPaintActor::StaticClass(), WorldTransform);
				NewActor = Cast<AVelesLayerBaseActor>(NewLayerActor);
				// NewActorName = AVelesLayerPaintActor::StaticClass()->GetName();
				NewActorName = *FString::Printf(TEXT("VLPaint_%s"), *UISettings->NewActor_LayerName.ToString());
			}
			else if (IsToolActive("WorldData"))
			{
				AVelesLayerWorldDataActor* NewWorldDataActor = EdMode->GetWorld()->SpawnActor<AVelesLayerWorldDataActor>(AVelesLayerWorldDataActor::StaticClass(), WorldTransform);
				NewActor = Cast<AVelesLayerBaseActor>(NewWorldDataActor);
				// NewActorName = AVelesLayerWorldDataActor::StaticClass()->GetName();
				NewActorName = *FString::Printf(TEXT("VLWorldData_%s"), *UISettings->NewActor_LayerName.ToString());
			}

			if (NewActor)
			{
				NewActor->Setup(FGuid::NewGuid(), CoreActor, UISettings->NewActor_LayerName);
		
				FActorLabelUtilities::SetActorLabelUnique(NewActor, NewActorName);
			}
		
			bCreated = true;
			CreatedActor = NewActor;
		}
	}

	if (bCreated)
	{
		EdMode->UpdateSettings();
		EdMode->UpdateLayerPaintList();

		if (GEditor && CreatedActor)
		{
			// Select the actor
			GEditor->SelectNone(false, true);
			GEditor->SelectActor(CreatedActor, /*InSelected=*/true, /*bNotify=*/false, /*bSelectEvenIfHidden=*/true);
			GEditor->NoteSelectionChange();
		}
	}

	return FReply::Handled();
}

void FVelesLayersEditorDetailCustomization_NewLayer::SetNewLayerName(const FText& NewValue, ETextCommit::Type CommitInfo, TSharedRef<IPropertyHandle> PropertyHandle_LayerName)
{
	const FName LayerName = *NewValue.ToString();
	ensure(PropertyHandle_LayerName->SetValue(LayerName) == FPropertyAccess::Success);
}

bool FVelesLayersEditorDetailCustomization_NewLayer::IsCreateButtonEnabled() const
{
	const UVelesLayersEditorMode* EdMode = GetEditorMode();
	
	if (EdMode != nullptr)
	{
		// return EdMode->IsLandscapeResolutionCompliant();
		return true;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
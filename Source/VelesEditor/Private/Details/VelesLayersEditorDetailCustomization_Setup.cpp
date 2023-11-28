// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/VelesLayersEditorDetailCustomization_Setup.h"

#include "SlateOptMacros.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#include "VelesLayersEditorMode.h"
#include "VelesLayersEditorObject.h"

#include "SPrimaryButton.h"
#include "VelesCore.h"
#include "Widgets/Input/SVectorInputBox.h"

#define LOCTEXT_NAMESPACE "VelesLayersEditor.Setup"

TSharedRef<IDetailCustomization> FVelesLayersEditorDetailCustomization_Setup::MakeInstance()
{
	return MakeShareable(new FVelesLayersEditorDetailCustomization_Setup);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FVelesLayersEditorDetailCustomization_Setup::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	if (!IsToolActive(TEXT("Setup")))
	{
		// Create default text that everything is initialized
		return;
	}
	
	IDetailCategoryBuilder& NewActorCategory = DetailBuilder.EditCategory("Setup");

	TSharedRef<IPropertyHandle> PropertyHandle_ComponentCount = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVelesLayersEditorObject, NewActor_ComponentCount));
	TSharedRef<IPropertyHandle> PropertyHandle_ComponentCount_X = PropertyHandle_ComponentCount->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_ComponentCount_Y = PropertyHandle_ComponentCount->GetChildHandle("Y").ToSharedRef();
	NewActorCategory.AddProperty(PropertyHandle_ComponentCount)
	.EditCondition(CanCreateNewCore(), nullptr)
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
	.EditCondition(CanCreateNewCore(), nullptr)
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
	.EditCondition(CanCreateNewCore(), nullptr)
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
	.EditCondition(CanCreateNewCore(), nullptr)
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
	.RowTag("VelesLayersEditor.TotalComponents")
	.NameContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		.Padding(FMargin(2))
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("TotalComponents", "Total Components"))
			.ToolTipText(LOCTEXT("NewActor_TotalComponents", "The total number of components that will be created for this Veles Core."))
		]
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	[
		SNew(SEditableTextBox)
		.IsReadOnly(true)
		.Font(DetailBuilder.GetDetailFont())
		.Text(this, &FVelesLayersEditorDetailCustomization_Setup::GetTotalComponentCount)
	];

	NewActorCategory.AddCustomRow(FText::GetEmpty()).WholeRowContent()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Right)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(4, 0)
		.AutoWidth()
		[
			SNew(SPrimaryButton)
			.Visibility_Static(&GetVisibilityOnlyInNoCore)
			.Text(LOCTEXT("Create", "Create"))
			// .AddMetaData<FTutorialMetaData>(FTutorialMetaData(TEXT("CreateButton"), TEXT("LevelEditorToolBox")))
			.OnClicked(this, &FVelesLayersEditorDetailCustomization_Setup::OnCreateButtonClicked)
			.IsEnabled(this, &FVelesLayersEditorDetailCustomization_Setup::IsCreateButtonEnabled)
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

EVisibility FVelesLayersEditorDetailCustomization_Setup::GetVisibilityOnlyInNoCore()
{
	const UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		return EdMode->HasCoreActor() ? EVisibility::Collapsed : EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

bool FVelesLayersEditorDetailCustomization_Setup::CanCreateNewCore() const
{
	const UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode)
	{
		return !EdMode->HasCoreActor();
	}
	return false;
}

TSharedRef<SWidget> FVelesLayersEditorDetailCustomization_Setup::GetComponentSizeMenu(TSharedRef<IPropertyHandle> PropertyHandle)
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

void FVelesLayersEditorDetailCustomization_Setup::OnChangeComponentSize(TSharedRef<IPropertyHandle> PropertyHandle, int32 NewSize)
{
	ensure(PropertyHandle->SetValue(NewSize) == FPropertyAccess::Success);
}

FText FVelesLayersEditorDetailCustomization_Setup::GetComponentSize(TSharedRef<IPropertyHandle> PropertyHandle)
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

void FVelesLayersEditorDetailCustomization_Setup::SetScale(FVector::FReal NewValue, ETextCommit::Type, TSharedRef<IPropertyHandle> PropertyHandle)
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

FText FVelesLayersEditorDetailCustomization_Setup::GetTotalComponentCount() const
{
	const UVelesLayersEditorMode* EdMode = GetEditorMode();
	if (EdMode != nullptr)
	{
		return FText::AsNumber(EdMode->UISettings->NewActor_ComponentCount.X * EdMode->UISettings->NewActor_ComponentCount.Y);
	}

	return FText::FromString(TEXT("---"));
}

bool FVelesLayersEditorDetailCustomization_Setup::IsCreateButtonEnabled() const
{
	const UVelesLayersEditorMode* EdMode = GetEditorMode();
	
	if (EdMode != nullptr)
	{
		// return EdMode->IsLandscapeResolutionCompliant();
		return true;
	}

	return true;
}

FReply FVelesLayersEditorDetailCustomization_Setup::OnCreateButtonClicked()
{
	UVelesLayersEditorMode* EdMode = GetEditorMode();

	bool bCreated = false;
	AActor* CreatedActor = nullptr;
	
	if (EdMode != nullptr && 
		EdMode->GetWorld() != nullptr && 
		EdMode->GetWorld()->GetCurrentLevel()->bIsVisible &&
		!EdMode->HasCoreActor())
	{
		UVelesLayersEditorObject* UISettings = EdMode->UISettings;
		const int32 ComponentCountX = UISettings->NewActor_ComponentCount.X;
		const int32 ComponentCountY = UISettings->NewActor_ComponentCount.Y;
		const int32 ComponentSize = UISettings->NewActor_ComponentSize;

		const FVector WorldScale3D = FVector(UISettings->NewActor_WorldScale, 1.0);
		const FIntPoint ComponentsSize = FIntPoint(ComponentCountX, ComponentCountY) * ComponentSize;
		const FVector ComponentsOffset = FTransform(FQuat::Identity, FVector::ZeroVector, WorldScale3D).TransformVector(FVector(-ComponentsSize.X / 2, -ComponentsSize.Y / 2, 0));

		const FVector WorldOffset = FVector(UISettings->NewActor_WorldOffset, 0.0);
		const FTransform WorldTransform = FTransform(FQuat::Identity, WorldOffset + ComponentsOffset, WorldScale3D);
		const FVector WorldLocation = WorldTransform.GetLocation();
		
		{
			FScopedTransaction Transaction(LOCTEXT("Undo", "Creating New Core"));
			
			AVelesCore* NewCoreActor = EdMode->GetWorld()->SpawnActor<AVelesCore>(AVelesCore::StaticClass(), FTransform::Identity);
			NewCoreActor->Setup(FGuid::NewGuid(), ComponentCountX, ComponentCountY, ComponentSize, UISettings->NewActor_WorldOffset, WorldLocation, UISettings->NewActor_WorldScale);

			bCreated = true;
			CreatedActor = NewCoreActor;
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

#undef LOCTEXT_NAMESPACE

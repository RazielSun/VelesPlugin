// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UVelesLayersEditorMode;
enum class EVelesLayersToolMode;

class FVelesLayersEditorDetailCustomization_Base : public IDetailCustomization
{
public:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override = 0;

	static bool IsToolActive(const FString& ToolName);

protected:
	static UVelesLayersEditorMode* GetEditorMode();

	template<typename type>
	static TOptional<type> OnGetValue(TSharedRef<IPropertyHandle> PropertyHandle);
	
	template<typename type>
	static void OnValueChanged(type NewValue, TSharedRef<IPropertyHandle> PropertyHandle);
	
	template<typename type>
	static void OnValueCommitted(type NewValue, ETextCommit::Type CommitType, TSharedRef<IPropertyHandle> PropertyHandle);

	template<typename type>
	static type GetPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle);

	template<typename type>
	static TOptional<type> GetOptionalPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle);

	template<typename type>
	static type* GetObjectPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle);

	static FText GetPropertyValueText(TSharedRef<IPropertyHandle> PropertyHandle);

	template<typename type>
	static void SetPropertyValue(type NewValue, ETextCommit::Type CommitInfo, TSharedRef<IPropertyHandle> PropertyHandle);
};


template<typename type>
TOptional<type> FVelesLayersEditorDetailCustomization_Base::OnGetValue(TSharedRef<IPropertyHandle> PropertyHandle)
{
	type Value;
	if (ensure(PropertyHandle->GetValue(Value) == FPropertyAccess::Success))
	{
		return TOptional<type>(Value);
	}

	// Value couldn't be accessed. Return an unset value
	return TOptional<type>();
}

template<typename type>
void FVelesLayersEditorDetailCustomization_Base::OnValueChanged(type NewValue, TSharedRef<IPropertyHandle> PropertyHandle)
{
	const EPropertyValueSetFlags::Type Flags = EPropertyValueSetFlags::InteractiveChange;
	ensure(PropertyHandle->SetValue(NewValue, Flags) == FPropertyAccess::Success);
}

template<typename type>
void FVelesLayersEditorDetailCustomization_Base::OnValueCommitted(type NewValue, ETextCommit::Type CommitType, TSharedRef<IPropertyHandle> PropertyHandle)
{
	ensure(PropertyHandle->SetValue(NewValue) == FPropertyAccess::Success);
}

template<typename type>
type FVelesLayersEditorDetailCustomization_Base::GetPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle)
{
	type Value;
	if (PropertyHandle->GetValue(Value) == FPropertyAccess::Success)
	{
		return Value;
	}

	// Couldn't get, return null / 0
	return type{};
}

template<typename type>
TOptional<type> FVelesLayersEditorDetailCustomization_Base::GetOptionalPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle)
{
	type Value;
	if (PropertyHandle->GetValue(Value) == FPropertyAccess::Success)
	{
		return Value;
	}

	// Couldn't get, return unset optional
	return TOptional<type>();
}

template<typename type>
type* FVelesLayersEditorDetailCustomization_Base::GetObjectPropertyValue(TSharedRef<IPropertyHandle> PropertyHandle)
{
	UObject* Value;
	if (PropertyHandle->GetValue(Value) == FPropertyAccess::Success)
	{
		return Cast<type>(Value);
	}

	// Couldn't get, return null
	return nullptr;
}

inline FText FVelesLayersEditorDetailCustomization_Base::GetPropertyValueText(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FString Value;
	if (PropertyHandle->GetValueAsFormattedString(Value) == FPropertyAccess::Success)
	{
		return FText::FromString(Value);
	}

	return FText();
}

template<typename type>
void FVelesLayersEditorDetailCustomization_Base::SetPropertyValue(type NewValue, ETextCommit::Type CommitInfo, TSharedRef<IPropertyHandle> PropertyHandle)
{
	ensure(PropertyHandle->SetValue(NewValue) == FPropertyAccess::Success);
}
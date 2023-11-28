// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "VelesLayersEditorObject.generated.h"

class UVelesLayersEditorMode;

UCLASS(MinimalAPI)
class UVelesLayersEditorObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	TWeakObjectPtr<UVelesLayersEditorMode> ParentMode;

	// New Actor Tool
	UPROPERTY(Category="New Actor", EditAnywhere, meta=(DisplayName="Layer Name", ShowForTools="Paint,WorldData"))
	FName NewActor_LayerName;

	UPROPERTY(Category="New Actor", EditAnywhere, meta=(DisplayName="Number of Component", ShowForTools="Setup,Paint,WorldData"))
	FIntPoint NewActor_ComponentCount;

	UPROPERTY(Category="New Actor", EditAnywhere, meta=(DisplayName="Component Size", ShowForTools="Setup,Paint,WorldData"))
	int32 NewActor_ComponentSize;

	UPROPERTY(Category="New Actor", EditAnywhere, meta=(DisplayName="World Offset", ShowForTools="Setup,Paint,WorldData"))
	FVector2D NewActor_WorldOffset;

	UPROPERTY(Category="New Actor", EditAnywhere, meta=(DisplayName="World Scale", ShowForTools="Setup,Paint,WorldData"))
	FVector2D NewActor_WorldScale;

	// Target
	UPROPERTY(Category="Target", EditAnywhere, meta=(DisplayName="Target Name", ShowForTools="Paint"))
	int32 CurrentTargetIndex;

	// Brush / Erase
	UPROPERTY(Category="Brush", EditAnywhere, meta=(DisplayName="Radius", ShowForTools="Paint"))
	float BrushRadius;

	UPROPERTY(Category="Brush", EditAnywhere, meta=(DisplayName="Opacity", ShowForTools="Paint", ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
	float BrushOpacity;

	UPROPERTY(Category="Brush", EditAnywhere, meta=(DisplayName="Falloff", ShowForTools="Paint", ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
	float BrushFalloff;

	//
	void SetParent(UVelesLayersEditorMode* InParentMode)
	{
		ParentMode = InParentMode;
	}
};

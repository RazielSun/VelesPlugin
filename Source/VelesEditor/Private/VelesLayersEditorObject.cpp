// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayersEditorObject.h"

UVelesLayersEditorObject::UVelesLayersEditorObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
NewActor_LayerName(NAME_None),
NewActor_ComponentCount(8,8),
NewActor_ComponentSize(128),
NewActor_WorldOffset(0, 0),
NewActor_WorldScale(100, 100),
CurrentTargetIndex(INDEX_NONE),
BrushRadius(512.f),
BrushOpacity(0.5f),
BrushFalloff(0.5f),
bUseTargetValue(false),
TargetValue(1.0f)
{
	//
}
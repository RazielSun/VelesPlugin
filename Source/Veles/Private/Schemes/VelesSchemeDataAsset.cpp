// Fill out your copyright notice in the Description page of Project Settings.


#include "Schemes/VelesSchemeDataAsset.h"

FVelesSchemeMaskParam::FVelesSchemeMaskParam(): LayerName(TEXT("Layer"))
, LayerType(FVelesLayerType::Mask)
, ClampRange(0, 1)
, PowerFactor(1)
, Multiplier(1)
, bDebugDraw(false)
{
}

FVelesSchemeAssetParam::FVelesSchemeAssetParam() : Probability(1.0f)
{
}

UVelesSubBiomeSchemeDataAsset::UVelesSubBiomeSchemeDataAsset() : RandomYawRotation(-180.0f, 180.0f)
                                                                 , RandomScale(0.9f, 1.1f)
                                                                 , RandomSeed(3253453)
, bDebugDraw(false)
{
}

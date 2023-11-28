// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ComponentVisualizer.h"

class FVelesBaseComponentVisualizer : public FComponentVisualizer
{
public:
	FVelesBaseComponentVisualizer();
	virtual ~FVelesBaseComponentVisualizer();
	
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};

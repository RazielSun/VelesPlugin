// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesBaseComponentVisualizer.h"

#include "VelesLayerComponent.h"
#include "VelesLayerBaseActor.h"

FVelesBaseComponentVisualizer::FVelesBaseComponentVisualizer()
{
}

FVelesBaseComponentVisualizer::~FVelesBaseComponentVisualizer()
{
}

void FVelesBaseComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
                                                       FPrimitiveDrawInterface* PDI)
{
	const UVelesLayerComponent* LayerComp = Cast<UVelesLayerComponent>(Component);
	if (!LayerComp)
		return;

	const AVelesLayerBaseActor* ProxyActor = Cast<AVelesLayerBaseActor>(LayerComp->GetOwner());
	if (!ProxyActor)
		return;

	const double CoordZ = ProxyActor->GetActorLocation().Z;

	const FVector LocalMin = FVector(LayerComp->GetComponentBase(), 0);
	const FVector LocalMax = LocalMin + FVector(LayerComp->ComponentSizeQuads, LayerComp->ComponentSizeQuads, 0);
	const FVector Min = ProxyActor->ActorToWorld().TransformPosition(LocalMin);
	const FVector Max = ProxyActor->ActorToWorld().TransformPosition(LocalMax);

	FBox Bounds(Min, Max);
	Bounds = Bounds.ExpandBy(10000.0); // @todo: Component?
	
	const FVector ViewOrigin = View->ViewMatrices.GetViewOrigin();
	if (Bounds.IsInside(ViewOrigin))
	{
		const int32 TileSize = 8; // @todo: Component?
		const float ThicknessLight = 0.8f;
		const float ThicknessBold = 2.0f;
		
		const int32 CountX = (LayerComp->ComponentSizeQuads / TileSize);
		const int32 CountY = (LayerComp->ComponentSizeQuads / TileSize);

		const double StepX = (Max.X - Min.X) / CountX;
		const double StepY = (Max.Y - Min.Y) / CountY;

		const FLinearColor OutsideColor = FLinearColor::Green;
		const FLinearColor InsideColor = FLinearColor::Blue;
		
		for (int32 X = 0; X <= CountX; X++)
		{
			const double PosX = Min.X + StepX * X;
			const bool bIsBorder = (X==0||X==CountX);
			const FVector StartPos = FVector(PosX, Min.Y, CoordZ);
			const FVector EndPos = FVector(PosX, Max.Y, CoordZ);
			PDI->DrawLine( StartPos, EndPos, (bIsBorder)?OutsideColor:InsideColor, SDPG_Foreground, (bIsBorder)?ThicknessBold:ThicknessLight );
		}

		for (int32 Y = 0; Y <= CountY; Y++)
		{
			const double PosY = Min.Y + StepY * Y;
			const bool bIsBorder = (Y==0||Y==CountY);
			const FVector StartPos = FVector(Min.X, PosY, CoordZ);
			const FVector EndPos = FVector(Max.X, PosY, CoordZ);
			PDI->DrawLine( StartPos, EndPos, (bIsBorder)?OutsideColor:InsideColor, SDPG_Foreground, (bIsBorder)?ThicknessBold:ThicknessLight );
		}
	}
}

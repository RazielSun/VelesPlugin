// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayersPaintTool.h"

#include "InteractiveToolManager.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "VelesCoreSubsystem.h"
#include "VelesLayersEditorMode.h"
#include "VelesLayerBaseActor.h"
#include "VelesLayersEditorObject.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseHoverBehavior.h"

static const int UVelesLayersBaseBrushTool_ShiftModifier = 1;

bool UVelesLayersPaintToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	if (const auto* EdMode = GetEditorMode())
		return EdMode->HasCoreActor();
	return Super::CanBuildTool(SceneState);
}

UVelesLayersBaseTool* UVelesLayersPaintToolBuilder::CreateToolInstance(const FToolBuilderState& SceneState) const
{
	return NewObject<UVelesLayersPaintTool>(SceneState.ToolManager);
}

UVelesLayersPaintBrushProperties::UVelesLayersPaintBrushProperties()
{
	bHeightfield = true;
	bWorldStatic = false;
	bWorldDynamic = false;
	bHasVelesTag = true;
}

UVelesLayersPaintTool::UVelesLayersPaintTool()
{
}

void UVelesLayersPaintTool::Setup()
{
	UInteractiveTool::Setup();

	bShiftToggle = false;

	Properties = NewObject<UVelesLayersPaintBrushProperties>(this);
	AddToolPropertySource(Properties);

	UClickDragInputBehavior* DragBehavior = NewObject<UClickDragInputBehavior>();
	DragBehavior->Modifiers.RegisterModifier(UVelesLayersBaseBrushTool_ShiftModifier, FInputDeviceState::IsShiftKeyDown);
	DragBehavior->Initialize(this);
	AddInputBehavior(DragBehavior);

	UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>();
	HoverBehavior->Modifiers.RegisterModifier(UVelesLayersBaseBrushTool_ShiftModifier, FInputDeviceState::IsShiftKeyDown);
	HoverBehavior->Initialize(this);
	AddInputBehavior(HoverBehavior);

	// bShiftToggle = false;
	// bCtrlToggle = false;
	//
	// // add input behaviors
	// UClickDragInputBehavior* DragBehavior = NewObject<UClickDragInputBehavior>();
	// DragBehavior->Modifiers.RegisterModifier(UMeshSurfacePointTool_ShiftModifier, FInputDeviceState::IsShiftKeyDown);
	// DragBehavior->Modifiers.RegisterModifier(UMeshSurfacePointTool_CtrlModifier, FInputDeviceState::IsCtrlKeyDown);
	// DragBehavior->Initialize(this);
	// AddInputBehavior(DragBehavior);
	//
	// UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>();
	// HoverBehavior->Modifiers.RegisterModifier(UMeshSurfacePointTool_ShiftModifier, FInputDeviceState::IsShiftKeyDown);
	// HoverBehavior->Modifiers.RegisterModifier(UMeshSurfacePointTool_CtrlModifier, FInputDeviceState::IsCtrlKeyDown);
	// HoverBehavior->Initialize(this);
	// AddInputBehavior(HoverBehavior);
}


FInputRayHit UVelesLayersPaintTool::CanBeginClickDragSequence(const FInputDeviceRay& ClickPos)
{
	return FInputRayHit(0.0f);
}


void UVelesLayersPaintTool::OnClickPress(const FInputDeviceRay& ClickPos)
{
	UpdateBrushLocation(ClickPos);

	check(!bTransactionStarted);
	if (!bTransactionStarted)
	{
		ToolStroke.Emplace( EdMode.Get(), EdMode->GetCoreActor(), EdMode.IsValid() ? EdMode->GetEditCurrentTarget() : nullptr );
		// @todo: start Transaction
		bTransactionStarted = true;
	}
	
	if (bValidHitTrace)
	{
		ToolStroke->Apply(ValidHitTrace, bShiftToggle);
	}
}

void UVelesLayersPaintTool::OnClickDrag(const FInputDeviceRay& ClickPos)
{
	UpdateBrushLocation(ClickPos);

	if (bValidHitTrace)
	{
		ToolStroke->Apply(ValidHitTrace, bShiftToggle);
	}
}


void UVelesLayersPaintTool::OnClickRelease(const FInputDeviceRay& ClickPos)
{
	UpdateBrushLocation(ClickPos);

	if (bValidHitTrace)
	{
		ToolStroke->Apply(ValidHitTrace, bShiftToggle);
	}

	ToolStroke.Reset();
	// @todo: end Transaction
	bTransactionStarted = false;

	UVelesCoreSubsystem::LayerPaintChangedEvent.Broadcast();
}


void UVelesLayersPaintTool::OnTerminateDragSequence()
{

}

void UVelesLayersPaintTool::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	if (ModifierID == UVelesLayersBaseBrushTool_ShiftModifier)
	{
		bShiftToggle = bIsOn;
	}
}

FInputRayHit UVelesLayersPaintTool::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos)
{
	return FInputRayHit(0.0f);
}

bool UVelesLayersPaintTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	UpdateBrushLocation(DevicePos);
	// UE_LOG(LogTemp, Warning, TEXT("OnUpdateHover %s %s SP %s"), *DevicePos.WorldRay.Origin.ToString(), *DevicePos.WorldRay.Direction.ToString(), *DevicePos.ScreenPosition.ToString());
	return true;
}

void UVelesLayersPaintTool::UpdateBrushLocation(const FInputDeviceRay& DevicePos)
{
	if (!IsEqual(DevicePos,LastDeviceRay))
	{
		LastDeviceRay = DevicePos;

		if (!TargetWorld)
			return;

		// SCOPE_CYCLE_COUNTER(STAT_FoliageTrace);

		// FCollisionQueryParams QueryParams(InTraceTag, SCENE_QUERY_STAT_ONLY(IFA_FoliageTrace), true);
		// QueryParams.bReturnFaceIndex = InbReturnFaceIndex;

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BrushToolTrace), true);
		QueryParams.bReturnFaceIndex = false;

		const FVector TraceStart(DevicePos.WorldRay.Origin);
		const FVector TraceEnd(DevicePos.WorldRay.Origin + DevicePos.WorldRay.Direction * HALF_WORLD_MAX);
		const float TraceRadius = 0.f; //

		TArray<FHitResult> Hits;
		FCollisionShape SphereShape;
		SphereShape.SetSphere(TraceRadius);
		TargetWorld->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects), SphereShape, QueryParams);

		bValidHitTrace = false;
		ValidHitTrace = FHitResult();
		for(const auto& Hit : Hits)
		{
			// @todo: clean here some invisible walls and etc
			if (Hit.bBlockingHit && Hit.GetComponent())
			{
				UPrimitiveComponent* Component = Hit.GetComponent();
				if (!Component)
					continue;
				
				const FCollisionResponseContainer& Container = Component->GetCollisionResponseToChannels();
				
				if ((Properties->bHeightfield && Component->IsA(ULandscapeHeightfieldCollisionComponent::StaticClass()))
					|| (Properties->bHasVelesTag && Hit.GetActor() && Hit.GetActor()->Tags.Contains(VELES_SURFACE_TAG))
					|| (Properties->bWorldStatic && Container.GetResponse(ECC_WorldStatic) == ECR_Block)
					|| (Properties->bWorldDynamic && Container.GetResponse(ECC_WorldDynamic) == ECR_Block))
				{
					bValidHitTrace = true;
					ValidHitTrace = Hit;
					break;
				}
			}
		}

		if (EdMode.IsValid())
		{
			EdMode->SetBrushLocation(bValidHitTrace, ValidHitTrace.ImpactPoint);
		}
	}
}

bool UVelesLayersPaintTool::IsEqual(const FInputDeviceRay& InDevicePosA, const FInputDeviceRay& InDevicePosB) const
{
	return InDevicePosA.ScreenPosition == InDevicePosB.ScreenPosition && InDevicePosA.WorldRay.Origin == InDevicePosB.WorldRay.Origin && InDevicePosA.WorldRay.Direction == InDevicePosB.WorldRay.Direction;
}

void FVelesLayersToolStrokePaint::Apply(const FHitResult& InHit, bool bInvert)
{
	if (!EdMode || !EdMode->UISettings || !Target)
		return;
	
	// Tablet pressure
	// const float Pressure = (ViewportClient && ViewportClient->Viewport->IsPenActive()) ? ViewportClient->Viewport->GetTabletPressure() : 1.0f;
	const float Pressure = 1.0f;

	const FVector MousePosition = InHit.ImpactPoint;
	const FSphere BrushSphere(MousePosition, EdMode->UISettings->BrushRadius);
	const FBoxSphereBounds BrushBounds(BrushSphere);

	// @todo: from target?
	const FTransform& ToWorld = Target->ActorToWorld();
	const FVector LocalMin = ToWorld.InverseTransformPosition(BrushBounds.GetBox().Min);
	const FVector LocalMax = ToWorld.InverseTransformPosition(BrushBounds.GetBox().Max);
	const FBox BrushBoundsLocal = FBox(LocalMin, LocalMax);

	// Bounds
	int32 X1, Y1, X2, Y2;
	X1 = FMath::FloorToInt(LocalMin.X);
	Y1 = FMath::FloorToInt(LocalMin.Y);
	X2 = FMath::FloorToInt(LocalMax.X);
	Y2 = FMath::FloorToInt(LocalMax.Y);

	const int32 SizeX = (X2 - X1 + 1);
	const int32 SizeY = (Y2 - Y1 + 1);

	const float TotalRadius = EdMode->UISettings->BrushRadius;
	const float Radius = (1.0f - EdMode->UISettings->BrushFalloff) * TotalRadius;
	const float Falloff = EdMode->UISettings->BrushFalloff * TotalRadius;

	TArray<float> BrushAlpha;
	BrushAlpha.SetNumUninitialized(SizeX*SizeY);
	for (int32 Y = Y1; Y <= Y2; Y++)
	{
		float* BrushScanline = BrushAlpha.GetData() + (Y - Y1) * SizeX + (0 - X1);
		for (int32 X = X1; X <= X2; X++)
		{
			float CurX = (((float)(X - X1 + 0.5) / SizeX) * BrushBounds.GetBox().GetSize().X + BrushBounds.GetBox().Min.X);
			float CurY = (((float)(Y - Y1 + 0.5) / SizeY) * BrushBounds.GetBox().GetSize().Y + BrushBounds.GetBox().Min.Y);
			float MouseDist = FMath::Sqrt(FMath::Square(MousePosition.X - CurX) + FMath::Square(MousePosition.Y - CurY));
			float PaintAmount = CalculateFalloff(MouseDist, Radius, Falloff);
			BrushScanline[X] = PaintAmount;
		}
	}

	this->Cache.CacheData(X1, Y1, X2, Y2);

	// The data we'll be writing to
	TArray<ToolTarget::CacheClass::DataType> Data;
	this->Cache.GetCachedData(X1, Y1, X2, Y2, Data);

	// The source data we use for editing. 
	TArray<ToolTarget::CacheClass::DataType>* SourceDataArrayPtr = &Data;
	TArray<ToolTarget::CacheClass::DataType> OriginalData;

	this->Cache.GetOriginalData(X1, Y1, X2, Y2, OriginalData);
	SourceDataArrayPtr = &OriginalData;

	const float AdjustedStrength = 1.0f; // @todo: need?
	
	float PaintStrength = EdMode->UISettings->BrushOpacity * Pressure * AdjustedStrength;
	if (PaintStrength <= 0.0f)
	{
		return;
	}

	// PaintStrength = FMath::Max(PaintStrength, 1.0f);
	
	// Apply the brush
	for (int32 Y = Y1; Y <= Y2; Y++)
	{
		const float* BrushScanline = BrushAlpha.GetData() + (Y - Y1) * SizeX + (0 - X1);
		auto* DataScanline = Data.GetData() + (Y - Y1) * SizeX + (0 - X1);
		auto* SourceDataScanline = SourceDataArrayPtr->GetData() + (Y - Y1) * SizeX + (0 - X1);
	
		for (int32 X = X1; X <= X2; X++)
		{
			const FIntPoint Key = FIntPoint(X, Y);
			const float BrushValue = BrushScanline[X];
	
			const float PaintAmount = BrushValue * PaintStrength * 255;
			auto& CurrentValue = DataScanline[X];
			const auto& SourceValue = SourceDataScanline[X];
	
			if (bInvert)
			{
				CurrentValue = ToolTarget::CacheClass::ClampValue(FMath::Min<int32>(SourceValue - FMath::RoundToInt(PaintAmount), CurrentValue));
			}
			else
			{
				CurrentValue = ToolTarget::CacheClass::ClampValue(FMath::Max<int32>(SourceValue + FMath::RoundToInt(PaintAmount), CurrentValue));
			}
		}
	}
	
	this->Cache.SetCachedData(X1, Y1, X2, Y2, Data);
	this->Cache.Flush();
}

float FVelesLayersToolStrokePaint::CalculateFalloff(float Distance, float Radius, float Falloff)
{
	return Distance < Radius ? 1.0f :
		Falloff > 0.0f ? FMath::Max<float>(0.0f, 1.0f - (Distance - Radius) / Falloff) :
		0.0f;
}
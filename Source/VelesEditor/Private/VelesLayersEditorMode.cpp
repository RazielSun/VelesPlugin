// Copyright Epic Games, Inc. All Rights Reserved.

#include "VelesLayersEditorMode.h"
#include "VelesLayersEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "VelesLayersEditorModeCommands.h"
#include "VelesLayersEditorObject.h"
#include "VelesLayerPaintActor.h"
#include "EngineUtils.h"
#include "VelesCore.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Toolkits/ToolkitManager.h"

////////////////////////////////////////////////////////////////////////// 

#include "Tools/VelesLayersBaseTool.h"
#include "Tools/VelesLayersPaintTool.h"
#include "Tools/VelesLayersWorldDataTool.h"

////////////////////////////////////////////////////////////////////////// 

namespace
{
	static FName BrushHighlightColorParamName("HighlightColor");
	static FName OpacityParamName("OpacityAmount");
}

#define LOCTEXT_NAMESPACE "VelesLayersEditorMode"

const FEditorModeID UVelesLayersEditorMode::EM_VelesLayersEditorModeId = TEXT("EM_VelesLayersEditorMode");

UVelesLayersEditorMode::UVelesLayersEditorMode() :
DefaultBrushOpacity(1.0f)
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UVelesLayersEditorMode::EM_VelesLayersEditorModeId,
		LOCTEXT("ModeName", "VelesLayers"),
		FSlateIcon(),
		true);

	UISettings = NewObject<UVelesLayersEditorObject>(GetTransientPackage(), TEXT("VelesLayersUISettings"), RF_Transactional);
	UISettings->SetParent(this);

	CurrentToolMode = EVelesLayersToolMode::None;
}

UVelesLayersEditorMode::~UVelesLayersEditorMode()
{
}


void UVelesLayersEditorMode::ActorSelectionChangeNotify()
{
}

void UVelesLayersEditorMode::Enter()
{
	UEdMode::Enter();

	// Register Tools
	// UpdateToolModes();

	InitializeBrush();
	UpdateSettings();
	UpdateLayerPaintList();
}

void UVelesLayersEditorMode::ModeTick(float DeltaTime)
{
	Super::ModeTick(DeltaTime);

	if (CurrentToolMode == EVelesLayersToolMode::Modify)
	{
		if (bUseBrush && bBrushTraceValid)
		{
			// Scale adjustment is due to default sphere SM size.
			constexpr float SphereRatio = 1.0f / 160.0f; // 0.00625f - 160 is Sphere Mesh Radius
			const float ActualRadius = UISettings->BrushRadius * SphereRatio;
			const FTransform BrushTransform = FTransform(FQuat::Identity, BrushLocation, FVector(ActualRadius));

			if (BrushComponent)
				BrushComponent->SetRelativeTransform(BrushTransform);

			if (BrushMID)
			{
				BrushMID->SetScalarParameterValue(TEXT("MaskRadiusRate"), ActualRadius);
				BrushMID->SetScalarParameterValue(TEXT("MaskFalloffRate"), UISettings->BrushFalloff);
			}
				
			
			if (BrushComponent && !BrushComponent->IsRegistered())
			{
				BrushComponent->RegisterComponentWithWorld(GetWorld());
			}
		}
		else
		{
			if (BrushComponent->IsRegistered())
			{
				BrushComponent->UnregisterComponent();
			}
		}
	}
}

void UVelesLayersEditorMode::Exit()
{
	LayersList.Empty();

	FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
	Toolkit.Reset();

	SetUseBrush(false);
	
	// Remove the brush
	if (BrushComponent)
		BrushComponent->UnregisterComponent();
	
	Super::Exit();
}

void UVelesLayersEditorMode::BindCommands()
{
	Super::BindCommands();

	const FVelesLayersEditorModeCommands& Commands = FVelesLayersEditorModeCommands::Get();

	RegisterTool(Commands.SetupTool, TEXT("Setup"), NewObject<UVelesLayersSetupToolBuilder>(this));
	RegisterTool(Commands.PaintTool, TEXT("Paint"), NewObject<UVelesLayersPaintToolBuilder>(this));
	RegisterTool(Commands.WorldDataTool, TEXT("WorldData"), NewObject<UVelesLayersWorldDataToolBuilder>(this));
}

void UVelesLayersEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FVelesLayersEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UVelesLayersEditorMode::GetModeCommands() const
{
	return FVelesLayersEditorModeCommands::Get().GetCommands();
}

void UVelesLayersEditorMode::SetCurrentToolMode(EVelesLayersToolMode InNewToolMode)
{
	CurrentToolMode = InNewToolMode;

	RedrawWidgets();

	GEditor->RedrawLevelEditingViewports();
}

bool UVelesLayersEditorMode::HasCoreActor() const
{
	return CoreActor.IsValid();
}

AVelesCore* UVelesLayersEditorMode::GetCoreActor()
{
	return CoreActor.Get();
}

bool UVelesLayersEditorMode::CanEditCurrentTarget() const
{
	return UISettings->CurrentTargetIndex != INDEX_NONE;
}

AVelesLayerBaseActor* UVelesLayersEditorMode::GetEditCurrentTarget() const
{
	if (CanEditCurrentTarget() && LayersList.Num() > UISettings->CurrentTargetIndex)
	{
		return LayersList[UISettings->CurrentTargetIndex].LayerActor.Get();
	}
	return nullptr;
}

void UVelesLayersEditorMode::UpdateSettings()
{
	if (UWorld* World = GetWorld())
	{
		const TActorIterator<AVelesCore> It(World);
		CoreActor = It ? *It : nullptr;

		if (CoreActor.IsValid())
		{
			UISettings->NewActor_ComponentCount = FIntPoint(CoreActor->NumComponentsX, CoreActor->NumComponentsY);
			UISettings->NewActor_ComponentSize = CoreActor->ComponentsSize;
			UISettings->NewActor_WorldOffset = CoreActor->WorldOffset;
			UISettings->NewActor_WorldScale = CoreActor->WorldScale;
			// const FIntPoint ComponentsSize = UISettings->NewActor_ComponentCount * UISettings->NewActor_ComponentSize;
			// const FVector WorldScale3D = FVector(UISettings->NewActor_WorldScale, 1.0);
			// const FVector Offset = FTransform(FQuat::Identity, FVector::ZeroVector, WorldScale3D).TransformVector(FVector(ComponentsSize.X / 2, ComponentsSize.Y / 2, 0));
			// UISettings->NewActor_WorldLocation = CoreActor->WorldOffset + FVector2d(Offset.X, Offset.Y);
		}
	}
}

void UVelesLayersEditorMode::UpdateLayerPaintList()
{
	UISettings->CurrentTargetIndex = INDEX_NONE;
	
	LayersList.Reset();
	
	UWorld* World = GetWorld();
	
	if (World)
	{
		int32 Index = 0;

		for (TActorIterator<AVelesLayerPaintActor> It(World); It; ++It)
		{
			if (It)
			{
				FVelesLayerInfo LayerInfo;
				LayerInfo.LayerName = It->LayerName.ToString();
				LayerInfo.LayerActor = *It;
				LayersList.Add(LayerInfo);
				Index++;
			}
		}
	}

	if (LayersList.Num() > 0)
	{
		UISettings->CurrentTargetIndex = 0;
		SetCurrentToolMode(EVelesLayersToolMode::Modify);
	}
	else
	{
		SetCurrentToolMode(EVelesLayersToolMode::NewActor);
	}
}

void UVelesLayersEditorMode::RedrawWidgets()
{
	if (Toolkit.IsValid())
	{
		StaticCastSharedPtr<FVelesLayersEditorModeToolkit>(Toolkit)->NotifyToolChanged();
	}
}

const TArray<FVelesLayerInfo>& UVelesLayersEditorMode::GetLayersList() const
{
	return LayersList;
}

void UVelesLayersEditorMode::SetUseBrush(bool bInUseBrush)
{
	bUseBrush = bInUseBrush;
	
	if (BrushComponent)
		BrushComponent->SetVisibility(bUseBrush);
}

void UVelesLayersEditorMode::SetBrushLocation(bool bInBrushTraceValid, const FVector& InBrushLocation)
{
	bBrushTraceValid = bInBrushTraceValid;
	BrushLocation = InBrushLocation;
}

void UVelesLayersEditorMode::InitializeBrush()
{
	// Load resources and construct brush component
	UStaticMesh* StaticMesh = nullptr;
	BrushDefaultHighlightColor = FColor::White;
	if (!IsRunningCommandlet())
	{
		UMaterial* BrushMaterial = LoadObject<UMaterial>(nullptr, TEXT("/VelesPlugin/Materials/PaintBrushSphereMaterial.PaintBrushSphereMaterial"), nullptr, LOAD_None, nullptr);
		BrushMID = UMaterialInstanceDynamic::Create(BrushMaterial, GetTransientPackage());
		check(BrushMID != nullptr);
		FLinearColor DefaultColor;
		BrushMID->GetVectorParameterDefaultValue(BrushHighlightColorParamName, DefaultColor);
		BrushDefaultHighlightColor = DefaultColor.ToFColor(false);
		StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/VelesPlugin/Meshes/Sphere.Sphere"), nullptr, LOAD_None, nullptr);
	}
	BrushCurrentHighlightColor = BrushDefaultHighlightColor;
	
	BrushComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("VelesLayersBrushComponent"));
	BrushComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BrushComponent->SetCollisionObjectType(ECC_WorldDynamic);
	BrushComponent->SetStaticMesh(StaticMesh);
	BrushComponent->SetMaterial(0, BrushMID);
	BrushComponent->SetAbsolute(true, true, true);
	BrushComponent->CastShadow = false;
        
	bBrushTraceValid = false;
	BrushLocation = FVector::ZeroVector;
        	
	// Get the default opacity from the material.
	BrushMID->GetScalarParameterValue(OpacityParamName, DefaultBrushOpacity);
}

#undef LOCTEXT_NAMESPACE

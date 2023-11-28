// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayerBaseActor.h"

#include "VelesCore.h"
#include "VelesCoreSubsystem.h"
#include "VelesLayerComponent.h"

AVelesLayerBaseActor::AVelesLayerBaseActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootScene->Mobility = EComponentMobility::Static;
	SetRootComponent(RootScene);
}

void AVelesLayerBaseActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	RegisterToSubsystem();
}

void AVelesLayerBaseActor::UnregisterAllComponents(bool bForReregister)
{
	UnregisterFromSubsystem();
	
	Super::UnregisterAllComponents(bForReregister);
}

void AVelesLayerBaseActor::RegisterToSubsystem()
{
	if (!IsPendingKillPending() && IsInitialized())
	{
		if (UWorld* OwningWorld = GetWorld())
		{
			if (UVelesCoreSubsystem* CoreSubsystem = OwningWorld->GetSubsystem<UVelesCoreSubsystem>())
			{
				CoreSubsystem->RegisterLayer(this);
			}
		}
	}
}

void AVelesLayerBaseActor::UnregisterFromSubsystem()
{
	// On shutdown the world will be unreachable
	if (GetWorld() && IsValidChecked(GetWorld()) && !GetWorld()->IsUnreachable())
	{
		if (UVelesCoreSubsystem* CoreSubsystem = GetWorld()->GetSubsystem<UVelesCoreSubsystem>())
		{
			CoreSubsystem->UnregisterLayer(this);
		}
	}
}

void AVelesLayerBaseActor::Initialize()
{
	bInitialized = true;

	RegisterToSubsystem();
}

#if WITH_EDITOR
void AVelesLayerBaseActor::Setup(const FGuid& InGuid, AVelesCore* InCore, FName InName)
{
	this->SetupInternal(InGuid, InCore, InName);

	Initialize();
}

void AVelesLayerBaseActor::SetupInternal(const FGuid& InGuid, AVelesCore* InCore, FName InName)
{
	check(InGuid.IsValid());
	check(InCore);
	
	VelesGuid = InGuid;
	LayerName = InName;
	CurrentCore = InCore;

	const int32 NumComponentsX = CurrentCore->NumComponentsX;
	const int32 NumComponentsY = CurrentCore->NumComponentsY;
	const int32 ComponentsSize = CurrentCore->ComponentsSize;
	
	check(LayersComponents.Num() == 0);
	LayersComponents.Empty(NumComponentsX * NumComponentsY);

	for (int32 Y = 0; Y < NumComponentsY; Y++)
	{
		for (int32 X = 0; X < NumComponentsX; X++)
		{
			const int32 BaseX = X * ComponentsSize;
			const int32 BaseY = Y * ComponentsSize;

			UVelesLayerComponent* LayersComponent = NewObject<UVelesLayerComponent>(this, NAME_None, RF_Transactional);
			LayersComponent->Init(BaseX, BaseY, ComponentsSize);
		}
	}
}
#endif

bool AVelesLayerBaseActor::GetExtent(FIntRect& OutExtent) const
{
	return GetExtent(OutExtent.Min.X, OutExtent.Min.Y, OutExtent.Max.X, OutExtent.Max.Y);
}

bool AVelesLayerBaseActor::GetExtent(int32& MinX, int32& MinY, int32& MaxX, int32& MaxY) const
{
	MinX = MAX_int32;
	MinY = MAX_int32;
	MaxX = MIN_int32;
	MaxY = MIN_int32;

	// Find range of entire landscape
	for (const auto& Component : LayersComponents)
	{
		Component->GetComponentExtent(MinX, MinY, MaxX, MaxY);
	}
	return (MinX != MAX_int32);
}

TObjectPtr<UVelesLayerComponent> AVelesLayerBaseActor::GetLayerComponent(FIntPoint InKey) const
{
	if (!CurrentCore.IsValid())
		return nullptr;

	const int32 ComponentSize = CurrentCore->ComponentsSize;

	const TObjectPtr<UVelesLayerComponent>* ComponentPtr = LayersComponents.FindByPredicate(
				[Key = InKey, ComponentSize](const TObjectPtr<UVelesLayerComponent>& InComponent)
				{
					return InComponent->GetComponentBase() == Key * ComponentSize;
				});
	if (ComponentPtr)
	{
		return (*ComponentPtr);
	}
	return nullptr;
}

const TArray<TObjectPtr<UVelesLayerComponent>>& AVelesLayerBaseActor::GetLayerComponents() const
{
	return LayersComponents;
}

bool AVelesLayerBaseActor::GetComponentsToUpdate(TArray<FIntPoint>& OutKeys)
{
	if (!CurrentCore.IsValid())
		return false;

	OutKeys = UpdatedComponentKeys;
	UpdatedComponentKeys.Reset();

	return !OutKeys.IsEmpty();
}

AVelesCore* AVelesLayerBaseActor::GetCore() const
{
	return CurrentCore.Get();
}

void AVelesLayerBaseActor::RequestContentUpdate(const UVelesLayerComponent* InComponent)
{
	if (!CurrentCore.IsValid())
		return;

	const int32 ComponentSize = CurrentCore->ComponentsSize;
	
	if (InComponent)
	{
		const FIntPoint Key = InComponent->GetComponentBase() / ComponentSize;
		UpdatedComponentKeys.AddUnique(Key);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesInstancedMeshActor.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Schemes/VelesSchemeDataAsset.h"


AVelesInstancedMeshActor::AVelesInstancedMeshActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
                                                                                                  , ActorBaseX(0)
                                                                                                  , ActorBaseY(0)
{
	PrimaryActorTick.bCanEverTick = false;

	Parent = CreateDefaultSubobject<USceneComponent>(TEXT("Parent"));
	Parent->SetMobility(EComponentMobility::Static);
	SetRootComponent(Parent);
}

void AVelesInstancedMeshActor::Init(int32 InBaseX, int32 InBaseY)
{
	SetActorBase(FIntPoint(InBaseX, InBaseY));

#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("VISMActor_%d_%d"), InBaseX, InBaseY));
#endif
}

FVelesInstancedMeshTypeInfo* AVelesInstancedMeshActor::GetOrCreateInfo(int32 InAssetIndex, UVelesSubBiomeSchemeDataAsset* InSubBiome)
{
	auto* Found = Infos.FindByPredicate([Index = InAssetIndex, SubBiome = InSubBiome](const FVelesInstancedMeshTypeInfo& InInfo)
	{
		return InInfo.AssetIndex == Index && InInfo.SubBiome == SubBiome;
	});
	
	if (!Found)
	{
		FVelesInstancedMeshTypeInfo NewInfo;
		NewInfo.AssetIndex = InAssetIndex;
		NewInfo.SubBiome = InSubBiome;
		NewInfo.Component = CreateComponent();
		const int32 Index = Infos.Add(NewInfo);
		Found = &Infos[Index];
	}
	
	if (Found && Found->Component.IsValid())
	{
		if (!InSubBiome->Assets.IsEmpty() && InAssetIndex >= 0 && InAssetIndex < InSubBiome->Assets.Num())
		{
			Found->Component->SetStaticMesh(InSubBiome->Assets[InAssetIndex].Asset.LoadSynchronous());
		}

		// @todo: InPrescription INSTANCE parameters here
	}
	return Found;
}

UHierarchicalInstancedStaticMeshComponent* AVelesInstancedMeshActor::CreateComponent()
{
	UHierarchicalInstancedStaticMeshComponent* HISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, NAME_None, RF_Transactional);

	// @todo: InPrescription SETUP parameters here

	HISMC->SetMobility(Parent->Mobility);
	HISMC->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	HISMC->RegisterComponent();
	
	AddInstanceComponent(HISMC);
	
	return HISMC;
}

bool AVelesInstancedMeshActor::RemoveComponent(UHierarchicalInstancedStaticMeshComponent* InComponent)
{
	if (IsValid(InComponent))
	{
		InComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		RemoveInstanceComponent(InComponent);
		InComponent->UnregisterComponent();
		InComponent->DestroyComponent();
		return true;
	}
	return false;
}

void AVelesInstancedMeshActor::ClearAllInstances()
{
	for (auto& Info : Infos)
	{
		if (auto* Component = Info.Component.Get())
		{
			Component->ClearInstances();
		}
	}
}

void AVelesInstancedMeshActor::ClearInstances(int32 InAssetIndex)
{
	if (const auto* Info = GetOrCreateInfo(InAssetIndex, nullptr))
	{
		if (auto* Component = Info->Component.Get())
		{
			Component->ClearInstances();
		}
	}
}

void AVelesInstancedMeshActor::UpdateInstances(int32 InAssetIndex, UVelesSubBiomeSchemeDataAsset* InSubBiome, const TArray<FTransform>& InInstanceTransforms)
{
	if (const auto* Info = GetOrCreateInfo(InAssetIndex, InSubBiome))
	{
		if (auto* Component = Info->Component.Get())
		{
			Component->ClearInstances();
			Component->AddInstances(InInstanceTransforms, false, true);
		}
	}
}

void AVelesInstancedMeshActor::UpdateInstances(UVelesSubBiomeSchemeDataAsset* InSubBiome,
	const TMap<int32, TArray<FTransform>>& InInstanceTransforms)
{
	TArray<int32> AssetIndices;
	InInstanceTransforms.GetKeys(AssetIndices);

	for (const auto& Pair : InInstanceTransforms)
	{
		UpdateInstances(Pair.Key, InSubBiome, Pair.Value);
	}

	for (int32 Index = Infos.Num() - 1; Index >= 0; --Index)
	{
		if (Infos[Index].SubBiome == InSubBiome && !AssetIndices.Contains(Infos[Index].AssetIndex))
		{
			if (auto* Component = Infos[Index].Component.Get())
			{
				RemoveComponent(Component);
			}
			Infos.RemoveAt(Index);
		}
	}
}

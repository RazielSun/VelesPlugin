// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesCoreSubsystem.h"

#include "EngineUtils.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "VelesComputeTileRun.h"
#include "VelesTileManager.h"

#include "VelesLayerBaseActor.h"
#include "VelesCore.h"
#include "VelesLayerComponent.h"
#include "Schemes/VelesSchemeDataAsset.h"
#include "Pattern/VelesPlacementPatternDataAsset.h"
#include "LandscapeComponent.h"
#include "VelesInstancedMeshActor.h"

UVelesCoreSubsystem::UVelesCoreSubsystem() : TileManager(nullptr)
{
}

UVelesCoreSubsystem::~UVelesCoreSubsystem()
{
}

UVelesCoreSubsystem::FOnLayerPaintChanged UVelesCoreSubsystem::LayerPaintChangedEvent;

void UVelesCoreSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TileManager)
	{
		TileManager->Update(DeltaTime);
	}
}

ETickableTickType UVelesCoreSubsystem::GetTickableTickType() const
{
	return HasAnyFlags(RF_ClassDefaultObject) || !GetWorld() || GetWorld()->IsNetMode(NM_DedicatedServer) ? ETickableTickType::Never : ETickableTickType::Always;
}

bool UVelesCoreSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// we also support inactive worlds -- they are used when the world is already saved, but SaveAs renames it:
	// then it duplicates the world (producing an inactive world), which we then need to update Landscapes in during OnPreSave()
	return Super::DoesSupportWorldType(WorldType) || WorldType == EWorldType::Inactive;
}

TStatId UVelesCoreSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVelesCoreSubsystem, STATGROUP_Tickables);
}

void UVelesCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TileManager = MakeUnique<FVelesTileManager>(this);
	BeginInitResource(TileManager.Get());

	LayerPaintHandle = UVelesCoreSubsystem::LayerPaintChangedEvent.AddUObject(this, &UVelesCoreSubsystem::OnLayerPaintChanged);
}

void UVelesCoreSubsystem::Deinitialize()
{
	BeginReleaseResource(TileManager.Get());

	UVelesCoreSubsystem::LayerPaintChangedEvent.Remove(LayerPaintHandle);

	Super::Deinitialize();
}

void UVelesCoreSubsystem::RegisterCoreActor(AVelesCore* InCore)
{
	CurrentCore = InCore;
}

void UVelesCoreSubsystem::UnregisterCoreActor(AVelesCore* InCore)
{
	if (CurrentCore == InCore)
	{
		CurrentCore = nullptr;
	}
}

void UVelesCoreSubsystem::RegisterLayer(AVelesLayerBaseActor* InActor)
{
	if (InActor)
	{
		check(!LayersMap.Contains(InActor->LayerName));
		LayersMap.Add(InActor->LayerName, InActor);
	}
}

void UVelesCoreSubsystem::UnregisterLayer(AVelesLayerBaseActor* InActor)
{
	if (InActor && LayersMap.Contains(InActor->LayerName))
	{
		LayersMap.Remove(InActor->LayerName);
	}
}

const UTexture2D* UVelesCoreSubsystem::GetTextureFromActor(const FName& InName, const FIntPoint& InKey) const
{
	if (LayersMap.Contains(InName))
	{
		TWeakObjectPtr<AVelesLayerBaseActor> Actor = LayersMap[InName];
		if (Actor.IsValid())
		{
			auto Component = Actor->GetLayerComponent(InKey);
			return Component ? Component->GetComponentTexture() : nullptr;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("VelesSubsystem doesn't have actor with name %s"), *InName.ToString());
	return nullptr;
}

void UVelesCoreSubsystem::RebuildAll()
{
	for (int32 X = 0; X < CurrentCore->NumComponentsX; ++X)
	{
		for (int32 Y = 0; Y < CurrentCore->NumComponentsY; ++Y)
		{
			AVelesInstancedMeshActor* InstancedActor = GetOrCreateInstancedMeshActor(FIntPoint(X, Y));
			if (InstancedActor)
			{
				InstancedActor->ClearAllInstances();
			}

			for (const auto& Biome : CurrentCore->BiomesSchemes)
			{
				for (const auto& SubBiome : Biome->SubBiomes)
				{
					SendTileToCompute(X, Y, SubBiome);
				}
			}
		}
	}
}

void UVelesCoreSubsystem::OnLayerPaintChanged()
{
	for (TTuple<FName, TWeakObjectPtr<AVelesLayerBaseActor>> LayerPaints : LayersMap)
	{
		if (LayerPaints.Value.IsValid() && LayerPaints.Value->HasContentToUpdate())
		{
			TArray<FIntPoint> KeysToUpdate;
			if (LayerPaints.Value->GetComponentsToUpdate(KeysToUpdate))
			{
				for (const auto& Biome : CurrentCore->BiomesSchemes)
				{
					for (const auto& SubBiome : Biome->SubBiomes)
					{
						for (const auto& Key : KeysToUpdate)
						{
							SendTileToCompute(Key.X, Key.Y, SubBiome);
						}
					}
				}
			}
		}
	}
}

AVelesInstancedMeshActor* UVelesCoreSubsystem::GetOrCreateInstancedMeshActor(const FIntPoint& InBase)
{
	if (!CurrentCore.IsValid())
		return nullptr;

	CollectInstancedActors();
	
	if (InstancedActors.Contains(InBase))
	{
		auto* InstancedActor = InstancedActors[InBase].Get();
		if (!InstancedActor)
		{
			InstancedActors.Remove(InBase);
		}
		else
		{
			return InstancedActor;
		}
	}

	const FVector Location = CurrentCore->WorldLocation + FVector(InBase, 0) * FVector(CurrentCore->WorldScale, 0) * CurrentCore->ComponentsSize;

	FActorSpawnParameters SpawnParams;
	AVelesInstancedMeshActor* NewActor = GetWorld()->SpawnActor<AVelesInstancedMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
	if (NewActor)
	{
		NewActor->Init(InBase.X, InBase.Y);
		InstancedActors.Add(InBase, NewActor);
		return NewActor;
	}

	return nullptr;
}

void UVelesCoreSubsystem::SendTileToCompute(int32 X, int32 Y, const TObjectPtr<UVelesSubBiomeSchemeDataAsset>& SubBiome)
{
	if (!SubBiome)
		return;
	
	// @todo: find VelesCore when start to  compute
	if (!CurrentCore.IsValid())
	{
		return;
	}
	
	if (!CurrentLandscape.IsValid() && !bLandscapeNotFound)
	{
		const TActorIterator<ALandscape> It(GetWorld());
		CurrentLandscape = It ? *It : nullptr;
		bLandscapeNotFound = CurrentLandscape == nullptr;
	}

	const FIntPoint Key(X, Y);
	const bool bHasReadyTile = ReadyTiles.Contains(Key);
	TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe> Tile = bHasReadyTile ? ReadyTiles[Key] : MakeShared<FVelesComputeTileRun, ESPMode::ThreadSafe>(X, Y);
	
	if (bHasReadyTile)
	{
		ReadyTiles.Remove(Key);
	}
	else
	{
		Tiles.Emplace(Tile);
	}
	
	Tile->WorldBounds = CurrentCore->GetTileBounds(X, Y, CurrentCore->ComponentsSize);

	Tile->SubBiome = SubBiome.Get();

	Tile->Seed = SubBiome->RandomSeed;
	Tile->RandomScale = SubBiome->RandomScale;
	Tile->RandomYawRotation = SubBiome->RandomYawRotation;
	Tile->LocationOffset = SubBiome->LocationOffset;
	
	Tile->Pattern = SubBiome->Pattern.Get();
	if (Tile->Pattern.IsValid())
	{
		auto PatternMin = FVector2D(Tile->WorldBounds.Min) / SubBiome->PatternScaleSize;
		auto PatternMax = FVector2D(Tile->WorldBounds.Max) / SubBiome->PatternScaleSize;
		FIntPoint IntMin = FIntPoint(FMath::FloorToInt(PatternMin.X), FMath::FloorToInt(PatternMin.Y));
		FIntPoint IntMax = FIntPoint(FMath::CeilToInt(PatternMax.X), FMath::CeilToInt(PatternMax.Y));
		Tile->PatternNumXY = IntMax - IntMin;
		const FVector2f Offset = FVector2f(SubBiome->PatternScaleSize) * IntMin;
		Tile->PatternOffsetAndScale = FVector4f(Offset.X, Offset.Y, SubBiome->PatternScaleSize.X, SubBiome->PatternScaleSize.Y);
	}

	// @todo: Landscape fixup find component
	if (CurrentLandscape.IsValid())
	{
		const FTransform& Transform = CurrentLandscape->GetActorTransform();
		const FVector2D LocalOffset = FVector2D(Tile->WorldBounds.Min) - FVector2D(Transform.GetLocation());
		if (const ULandscapeInfo* LandscapeInfo = CurrentLandscape->GetLandscapeInfo())
		{
			const FVector2D FloatLocalSectionBase = LocalOffset / (FVector2D(Transform.GetScale3D()) * LandscapeInfo->ComponentSizeQuads);
			// @todo: 512?
			const FIntPoint LocalSectionBase = FIntPoint(FMath::Clamp(FMath::Floor(FloatLocalSectionBase.X), 0, 512), FMath::Clamp(FMath::Floor(FloatLocalSectionBase.Y), 0, 512)) * LandscapeInfo->ComponentSizeQuads;
			// @todo: For loop?
			LandscapeInfo->ForAllLandscapeComponents([&Tile, LocalSectionBase](ULandscapeComponent* LandscapeComponent)
			{
				if (LandscapeComponent->GetSectionBase() == LocalSectionBase)
				{
					Tile->LandscapeComponent = LandscapeComponent;
				}
			});
		}

		if (Tile->LandscapeComponent)
		{
			Tile->HeightmapTexture = Tile->LandscapeComponent->GetHeightmap();
			Tile->HeightmapScaleBias = FVector4f(Tile->LandscapeComponent->HeightmapScaleBias);

			const FVector LocalLocation = FVector(Tile->LandscapeComponent->GetSectionBase().X, Tile->LandscapeComponent->GetSectionBase().Y, 0);
			Tile->LandscapeLocation = FVector3f(Transform.TransformPosition(LocalLocation));
			Tile->LandscapeScale = FVector3f(Transform.GetScale3D());
		}
	}
	else
	{
		Tile->LandscapeComponent = nullptr;
		Tile->HeightmapTexture = nullptr;
	}
	
	Tile->TextureSize = CurrentCore->ComponentsSize;
	TArray<FVelesMaskTileInfo> Masks;
	TArray<FVelesWeightmapTileInfo> WeightmapInfos;
	for (const auto& MaskInfo : SubBiome->Masks)
	{
		if (MaskInfo.LayerType == FVelesLayerType::Mask)
		{
			if (LayersMap.Contains(*MaskInfo.LayerName))
			{
				const auto Component = LayersMap[*MaskInfo.LayerName]->GetLayerComponent(Key);
				if (Component && Component->GetComponentTexture())
				{
					FVelesMaskTileInfo TileInfo;
					TileInfo.Texture = Component->GetComponentTexture();
					TileInfo.ClampPowMul = FVector4f(MaskInfo.ClampRange.X, MaskInfo.ClampRange.Y, MaskInfo.PowerFactor, MaskInfo.Multiplier);
					TileInfo.bDebugDraw = MaskInfo.bDebugDraw;
					Masks.Add(TileInfo);
				}
			}
		}
		else if (MaskInfo.LayerType == FVelesLayerType::Weightmap)
		{
			if (Tile->LandscapeComponent)
			{
				const auto& Infos = Tile->LandscapeComponent->GetWeightmapLayerAllocations();
				for (const FWeightmapLayerAllocationInfo& Info : Infos)
				{
					const FString LayerName = Info.LayerInfo ? Info.LayerInfo.GetName() : FString();
					if (LayerName.Contains(*MaskInfo.LayerName))
					{
						FVelesWeightmapTileInfo TileInfo;
						TileInfo.Texture = Tile->LandscapeComponent->GetWeightmapTextures()[Info.WeightmapTextureIndex];
						TileInfo.ClampPowMul = FVector4f(MaskInfo.ClampRange.X, MaskInfo.ClampRange.Y, MaskInfo.PowerFactor, MaskInfo.Multiplier);
						TileInfo.bDebugDraw = MaskInfo.bDebugDraw;
						TileInfo.ScaleBias = FVector4f(Tile->LandscapeComponent->WeightmapScaleBias);
						TileInfo.Channel = Info.WeightmapTextureChannel;
						WeightmapInfos.Add(TileInfo);
						break;
					}
				}
			}
		}
	}
	
	Tile->MasksInfos = Masks;
	Tile->WeightmapInfos = WeightmapInfos;
	Tile->DebugTexture = CurrentCore->GetOrCreateCompositeTexture(Key);
	Tile->bDebugDraw = SubBiome->bDebugDraw;

	TArray<float> AssetsProbabilities;
	float TotalProbability = 0;
	for (const auto& Info : SubBiome->Assets)
	{
		TotalProbability += Info.Probability;
		AssetsProbabilities.Add(Info.Probability);
	}
	Tile->NumAssets = SubBiome->Assets.Num();
	Tile->TotalAssetProbability = TotalProbability;
	Tile->AssetsProbabilities = AssetsProbabilities;

	const int32 FoundMasks = Masks.Num() + WeightmapInfos.Num();
	if (SubBiome->Masks.Num() == FoundMasks)
	{
		if (TileManager)
		{
			TileManager->ComputeTile(Tile);
		}
	}
	else
	{
		Tile->InstancedObjectTransforms.Reset();
		GameThread_HandleTile(Tile);
	}
}

void UVelesCoreSubsystem::GameThread_HandleTile(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& Tile)
{
	const FBox Bounds = Tile->WorldBounds.ExpandBy(FVector(0,0,10000));

	AVelesInstancedMeshActor* InstancedActor = GetOrCreateInstancedMeshActor(Tile->Coord);
	if (InstancedActor)
	{
		if (Tile->SubBiome.IsValid())
		{
			// UE_LOG(LogTemp, Display, TEXT("[VELES] GameThread_HandleTile:: Tile: %d %d has %d items for %d asset [Bounds: %s]"), Tile->Coord.X, Tile->Coord.Y, Pair.Value.Num(), Pair.Key, *Bounds.ToString());
			InstancedActor->UpdateInstances(Tile->SubBiome.Get(), Tile->InstancedObjectTransforms);
		}
	}

	ReadyTiles.Add(Tile->Coord, Tile);
}

void UVelesCoreSubsystem::CollectInstancedActors()
{
	if (!bInstancedActorsCollected)
	{
		for (TActorIterator<AVelesInstancedMeshActor> It(GetWorld()); It; ++It)
		{
			InstancedActors.Add(It->GetActorBase(), *It);
		}
		bInstancedActorsCollected = true;
	}
}

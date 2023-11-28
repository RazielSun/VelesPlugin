// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesTileManager.h"

#include "VelesCoreSubsystem.h"
#include "DensityMapComputeShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "VelesComputeTileRun.h"
#include "Pattern/VelesPlacementPatternDataAsset.h"
#include "Engine/TextureRenderTarget2D.h"

FVelesTileManager::FVelesTileManager(UVelesCoreSubsystem* InCoreSubsystem) : FRenderResource(), CoreSubsystem(InCoreSubsystem)
{
}

void FVelesTileManager::InitRHI()
{
	FRenderResource::InitRHI();
}

void FVelesTileManager::ReleaseRHI()
{
	FRenderResource::ReleaseRHI();
}

void FVelesTileManager::Update(float DeltaTime)
{
	ENQUEUE_RENDER_COMMAND(UVelesCoreSubsystem_Tick)(
		[&](FRHICommandListImmediate& RHICmdList)
		{
			for (int32 Index = ReadbackBuffersInUse.Num()-1; Index >= 0; --Index)
			{
				FRHIGPUBufferReadback* GPUReadback = ReadbackBuffersInUse[Index];
				if (GPUReadback && GPUReadback->IsReady())
				{
					if (RenderThread_ReadbackPositions(GPUReadback))
					{
						ReadbackBuffersReady.Add(GPUReadback);
						ReadbackBuffersInUse.Remove(GPUReadback);
						OnReadbackPositionDone();
					}
				}
			}
		});
}

void FVelesTileManager::ComputeTile(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& InTile)
{
	// @todo: Logs
	if (!InTile->SubBiome.IsValid())
		return;
	
	if (!InTile->Pattern.IsValid())
		return;

	if (ReadbackBuffersReady.Num() > 0 || MaxReadbackBuffersNum > 0)
	{
		if (MaxReadbackBuffersNum > 0)
			MaxReadbackBuffersNum--;

		ComputeTileInternal(InTile);
	}
	else
	{
		PostponeTile(InTile);
	}
}

void FVelesTileManager::ComputeTileInternal(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& Tile)
{
	ActiveTiles.Add(Tile);

	UE_LOG(LogTemp, Display, TEXT("[VELES] ComputeTileInternal Tile %s"), *Tile->Coord.ToString());
	
	ENQUEUE_RENDER_COMMAND(FComputeShaderRunner) (
			[this, Tile](FRHICommandListImmediate& RHICmdList)
		{
				FRDGBuilder GraphBuilder(RHICmdList);

				{
					int32 TextureSize = Tile->TextureSize;
					FClearValueBinding ClearColor = Tile->MasksInfos.IsEmpty() && Tile->WeightmapInfos.IsEmpty()
						                                ? FClearValueBinding::Black
						                                : FClearValueBinding::White;
					
					// Prepare
					FRDGTextureDesc OutDesc = FRDGTextureDesc::Create2D(FIntPoint(TextureSize, TextureSize), PF_G8, ClearColor, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
					FRDGTextureRef RDGOutTexture = GraphBuilder.CreateTexture(OutDesc, TEXT("Veles::OutCompositeTexture"));
					FRDGTextureUAVRef RDGOutTextureRef = GraphBuilder.CreateUAV(RDGOutTexture);
					FRDGTextureSRVRef RDGTextureRef = GraphBuilder.CreateSRV(RDGOutTexture);

					FRDGTextureClearInfo ClearInfo;
					ClearInfo.NumMips = 1;
					AddClearRenderTargetPass(GraphBuilder, RDGOutTexture, ClearInfo);

					// Debug Texture
					FRHITexture* DebugTextureRHI = Tile->DebugTexture->GetResource()->GetTexture2DRHI();
					FRDGTextureRef RDGDebugTexture = RegisterExternalTexture(GraphBuilder, DebugTextureRHI, *Tile->DebugTexture->GetName());

					FVelesDensityMapCSDispatchRDGParams DensityBaseParams;
					DensityBaseParams.OutputTexture = RDGOutTextureRef;
					DensityBaseParams.TextureSize = TextureSize;

					bool bHasDebugDraw = false;
					
					for (const auto& Info : Tile->MasksInfos)
					{
						FRHITexture* MaskTextureRHI = Info.Texture->GetResource()->GetTexture2DRHI();
						FRDGTextureRef RDGMaskTexture = RegisterExternalTexture(GraphBuilder, MaskTextureRHI, *Info.Texture->GetName());
						
						FVelesDensityMapCSDispatchRDGParams MaskParams = DensityBaseParams;
						MaskParams.Texture = GraphBuilder.CreateSRV(RDGMaskTexture);
						MaskParams.ClampPowMul = Info.ClampPowMul;
						MaskParams.bDebug = Info.bDebugDraw;
						
						if (!bHasDebugDraw && Info.bDebugDraw)
						{
							bHasDebugDraw = true;
							MaskParams.DebugTexture = GraphBuilder.CreateUAV(RDGDebugTexture);
						}
						
						VelesShaderUtils::AddComputeDensityMap(GraphBuilder, MaskParams);
					}

					for (const auto& Info : Tile->WeightmapInfos)
					{
						FRHITexture* WTextureRHI = Info.Texture->GetResource()->GetTexture2DRHI();
						FRDGTextureRef RDGWTexture = RegisterExternalTexture(GraphBuilder, WTextureRHI, *Info.Texture->GetName());
						
						FVelesDensityMapCSDispatchRDGParams WeightmapParams = DensityBaseParams;
						WeightmapParams.Texture = GraphBuilder.CreateSRV(RDGWTexture);
						WeightmapParams.ClampPowMul = Info.ClampPowMul;
						WeightmapParams.bDebug = Info.bDebugDraw;
						WeightmapParams.WeightmapScaleBias = Info.ScaleBias;
						WeightmapParams.WeightmapChannel = Info.Channel;

						if (!bHasDebugDraw && Info.bDebugDraw)
						{
							bHasDebugDraw = true;
							WeightmapParams.DebugTexture = GraphBuilder.CreateUAV(RDGDebugTexture);
						}

						VelesShaderUtils::AddComputeDensityMap(GraphBuilder, WeightmapParams);
					}

					// Debug Copy
					if (!bHasDebugDraw && Tile->bDebugDraw)
					{
						bHasDebugDraw = true;
						FRHICopyTextureInfo CopyInfo;
						CopyInfo.Size = FIntVector(TextureSize, TextureSize, 1);
						AddCopyTexturePass(GraphBuilder, RDGOutTexture, RDGDebugTexture, CopyInfo);
					}

					// @todo: transitions?
					
					const int32 ElementCount = Tile->Pattern->Pattern.Num();
					const int32 PatternCount = Tile->PatternNumXY.X * Tile->PatternNumXY.Y;

					// Two Vector4f - Pos.xyz Valid.w Normal.xy Rot.z Scl.w
					FRDGBufferRef PointCloudBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateBufferDesc(sizeof(FVector4f), PatternCount * ElementCount * 2), TEXT("PointCloudBufferData"));
					FRDGBufferUAVRef PointCloudUAV = GraphBuilder.CreateUAV(PointCloudBuffer, PF_A32B32G32R32F);
					FRDGBufferSRVRef PointCloudSRV = GraphBuilder.CreateSRV(PointCloudBuffer, PF_A32B32G32R32F);

					AddClearUAVPass(GraphBuilder, PointCloudUAV, 0u);

					// FRDGBufferDesc PatternBufferDesc = FRDGBufferDesc::CreateBufferDesc(sizeof(FVector4f), ElementCount);
					FRDGBufferRef PatternBuffer = CreateStructuredBuffer(GraphBuilder,TEXT("PatternBufferData"),sizeof(FVector4f), Tile->Pattern->Pattern.Num(),Tile->Pattern->Pattern.GetData(),sizeof(FVector4f) * ElementCount);
					FRDGBufferSRVRef PatternBufferSRV = GraphBuilder.CreateSRV(PatternBuffer);

					// Landscape Texture
					FRDGTextureSRVRef RDGLandscapeTextureSRV = nullptr;
					if (Tile->HeightmapTexture)
					{
						FRHITexture* LandscapeTextureRHI = Tile->HeightmapTexture->GetResource()->GetTexture2DRHI();
						FRDGTextureRef RDGLandscapeTexture = RegisterExternalTexture(GraphBuilder, LandscapeTextureRHI, *Tile->HeightmapTexture->GetName());
						RDGLandscapeTextureSRV = GraphBuilder.CreateSRV(RDGLandscapeTexture);
					}

					{
						FVelesGenerateCSDispatchRDGParams Params;
						Params.DensityTexture = RDGTextureRef;
						Params.WorldBounds = Tile->GetBounds();
						Params.PointCloudBuffer = PointCloudUAV;
						Params.PatternBuffer = PatternBufferSRV;
						Params.PatternElements = ElementCount;
						Params.PatternNumX = Tile->PatternNumXY.X;
						Params.PatternNumY = Tile->PatternNumXY.Y;
						Params.PatternOffsetAndScale = Tile->PatternOffsetAndScale;
						Params.HeightfieldTexture = RDGLandscapeTextureSRV;
						// Params.HeightfieldBounds = Tile->GetLandscapeBounds();
						Params.HeightmapScaleBias = Tile->HeightmapScaleBias;
						Params.HeightfieldLocation = Tile->LandscapeLocation;
						Params.HeightfieldScale = Tile->LandscapeScale;

						VelesShaderUtils::AddComputeGenerate(GraphBuilder, Params);
					}

					// @todo: Transforms
					FRDGBufferRef OutInstancesBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FInstanceTransformPayload), ElementCount * PatternCount), TEXT("InstancesBufferData"));
					FRDGBufferUAVRef OutInstancesBufferUAV = GraphBuilder.CreateUAV(OutInstancesBuffer);

					FRDGBufferRef AssetsProbabilityBuffer = CreateStructuredBuffer(GraphBuilder,TEXT("ProbabilityBufferData"),sizeof(float), Tile->AssetsProbabilities.Num(),Tile->AssetsProbabilities.GetData(),sizeof(float) * Tile->AssetsProbabilities.Num());
					FRDGBufferSRVRef AssetsProbabilityBufferSRV = GraphBuilder.CreateSRV(AssetsProbabilityBuffer);

					{
						FVelesPlacementCSDispatchRDGParams Params;
						Params.PointCloudBuffer = PointCloudSRV;
						Params.OutInstancesBuffer = OutInstancesBufferUAV;
						Params.NumElements = ElementCount;
						Params.NumX = Tile->PatternNumXY.X;
						Params.NumY = Tile->PatternNumXY.Y;
						Params.Seed = Tile->Seed;
						Params.RandomScale = Tile->RandomScale;
						Params.RandomYawRotation = Tile->RandomYawRotation;
						Params.LocationOffset = Tile->LocationOffset;
						Params.AssetsProbabilityBuffer = AssetsProbabilityBufferSRV;
						Params.NumAssets = Tile->NumAssets;
						Params.TotalAssetsProbability = Tile->TotalAssetProbability;
						
						VelesShaderUtils::AddComputePlacement(GraphBuilder, Params);
					}

					FRHIGPUBufferReadback* GPUBufferReadback = nullptr;
					if (!ReadbackBuffersReady.IsEmpty())
					{
						GPUBufferReadback = ReadbackBuffersReady.Pop();
					}
					else
					{
						GPUBufferReadback = new FRHIGPUBufferReadback(TEXT("ExecuteVelesComputeShaderOutput"));
					}
					Tile->GPUReadback = GPUBufferReadback;

					AddEnqueueCopyPass(GraphBuilder, GPUBufferReadback, OutInstancesBuffer, ElementCount * PatternCount * sizeof(FInstanceTransformPayload));
					ReadbackBuffersInUse.Add(GPUBufferReadback);

					UE_LOG(LogTemp, Display, TEXT("[VELES] ComputeTileInternal Tile %s Readback %p"), *Tile->Coord.ToString(), GPUBufferReadback);
				}

				// Execute the graph.
				GraphBuilder.Execute();
		});
}

void FVelesTileManager::PostponeTile(const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& InTile)
{
	PostponedTiles.Add(InTile);
}

bool FVelesTileManager::RenderThread_ReadbackPositions(FRHIGPUBufferReadback* GPUReadback)
{
	int32 FoundIndex = INDEX_NONE;
	for (int32 Index = 0; Index < ActiveTiles.Num(); Index++)
	{
		if (ActiveTiles[Index]->GPUReadback == GPUReadback)
		{
			FoundIndex = Index;
			break;
		}
	}

	if (FoundIndex == INDEX_NONE)
	{
		// @todo: return Tile
		return false;
	}

	const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe> Tile = ActiveTiles[FoundIndex];

	UE_LOG(LogTemp, Display, TEXT("[VELES] RenderThread_ReadbackPositions Index %d Readback %p Tile %s"), FoundIndex, GPUReadback, *Tile->Coord.ToString());
	
	//
	const int32 ElementCount = Tile->Pattern->Pattern.Num();
	const int32 PatternCount = Tile->PatternNumXY.X * Tile->PatternNumXY.Y;
	const int32 TotalCount = ElementCount * PatternCount;
	FInstanceTransformPayload* Buffer = (FInstanceTransformPayload*)GPUReadback->Lock(sizeof(FInstanceTransformPayload) * TotalCount);

	
	TMap<int32, TArray<FTransform>> MapTransforms;
	for (int32 Index = 0; Index < TotalCount; Index++)
	{
		const auto& Data = Buffer[Index];
		if (Data.Location.W > 0)
		{
			FTransform Instance;
			Instance.SetLocation(FVector(Data.Location.X, Data.Location.Y, Data.Location.Z));
			Instance.SetRotation(FQuat(FRotator(Data.Rotation.X, Data.Rotation.Z, Data.Rotation.Y)));
			Instance.SetScale3D(FVector(Data.Scale.X, Data.Scale.Y, Data.Scale.Z));

			const int32 AssetIndex = FMath::RoundToInt(Data.Location.W - 1);
			if (!MapTransforms.Contains(AssetIndex))
			{
				TArray<FTransform> Temp;
				MapTransforms.Add(AssetIndex, Temp);
			}
			MapTransforms[AssetIndex].Add(Instance);
		}
	}
	Tile->InstancedObjectTransforms = MapTransforms;
	
	GPUReadback->Unlock();
	Tile->GPUReadback = nullptr;

	// @todo: Change Array And ?
	// ActiveTiles.RemoveSwap(Tile);
	ActiveTiles.RemoveAt(FoundIndex);

	AsyncTask(ENamedThreads::GameThread, [LambdaSubsystem = CoreSubsystem, Tile]() {
		if (LambdaSubsystem.IsValid())
			LambdaSubsystem->GameThread_HandleTile(Tile);
	});

	return true;
}

void FVelesTileManager::OnReadbackPositionDone()
{
	if (PostponedTiles.Num() > 0)
	{
		const TSharedRef<FVelesComputeTileRun, ESPMode::ThreadSafe>& Tile = PostponedTiles.Pop();
		ComputeTile(Tile);
	}
}

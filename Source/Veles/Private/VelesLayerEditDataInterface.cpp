// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayerEditDataInterface.h"

#include "TextureCompiler.h"
#include "VelesCore.h"
#include "VelesLayerBaseActor.h"
#include "VelesLayerComponent.h"

FVelesTextureDataInfo::FVelesTextureDataInfo(UTexture2D* InTexture) : Texture(InTexture)
{
	MipInfo.AddZeroed(Texture->Source.GetNumMips());
	Texture->SetFlags(RF_Transactional);
	Texture->TemporarilyDisableStreaming();
}

FVelesTextureDataInfo::~FVelesTextureDataInfo()
{
	// Unlock any mips still locked.
	for( int32 i=0;i<MipInfo.Num();i++ )
	{
		if( MipInfo[i].MipData )
		{
			Texture->Source.UnlockMip(i);
			MipInfo[i].MipData = NULL;
		}
	}
	Texture->ClearFlags(RF_Transactional);
}

bool FVelesTextureDataInfo::UpdateTextureData()
{
	// EditToolTexture->UpdateResource();

	// Only wait once
	bool bNeedToFinishCompilation = true;

	if (bNeedToFinishCompilation)
	{
		// Need to make sure we have a valid Resource
		FTextureCompilingManager::Get().FinishCompilation({ Texture });
		bNeedToFinishCompilation = false;
	}

	int32 DataSize = sizeof(uint8);
				
	const uint32 SrcSizeX = (Texture->Source.GetSizeX()) >> 0;
	const uint32 SrcSizeY = (Texture->Source.GetSizeY()) >> 0;
	const uint32 SrcPitch = (SrcSizeX * DataSize);
	const uint32 BufferSize = SrcSizeX * SrcSizeY * DataSize;

	for (int32 i = 0; i < MipInfo.Num(); i++)
	{
		if (MipInfo[i].MipData && MipInfo[i].MipUpdateRegions.Num() > 0)
		{
			// Copy Mip update data so we can avoid waiting for Render thread in calling method
			FMipInfo* CopyMipInfo = new FMipInfo();
			CopyMipInfo->MipUpdateRegions = MipInfo[i].MipUpdateRegions;
			CopyMipInfo->MipData = FMemory::Malloc(BufferSize);
			FMemory::Memcpy(CopyMipInfo->MipData, MipInfo[i].MipData, BufferSize);
				
			Texture->UpdateTextureRegions(0, CopyMipInfo->MipUpdateRegions.Num(), &CopyMipInfo->MipUpdateRegions[0], SrcPitch, DataSize, (uint8*)CopyMipInfo->MipData, 
				[CopyMipInfo](uint8* SrcData, const FUpdateTextureRegion2D*) 
				{ 
					FMemory::Free(CopyMipInfo->MipData);
					delete CopyMipInfo; 
				});
		}
	}

	return true;
}

FVelesLayerEditDataInterface::FVelesLayerEditDataInterface(AVelesCore* InCore, AVelesLayerBaseActor* InLayer) : Core(InCore), Layer(InLayer)
{
}

FVelesLayerEditDataInterface::~FVelesLayerEditDataInterface()
{
	Flush();
}

FVelesTextureDataInfo* FVelesLayerEditDataInterface::GetTextureDataInfo(UTexture2D* Texture)
{
	FVelesTextureDataInfo* Result = TextureDataMap.FindRef(Texture);
	if( !Result )
	{
		Result = TextureDataMap.Add(Texture, new FVelesTextureDataInfo(Texture));
	}
	return Result;
}

void FVelesLayerEditDataInterface::GetEditToolTextureData(const int32 X1, const int32 Y1, const int32 X2, const int32 Y2,
	TMap<FIntPoint, uint8>& Data)
{
	const int32 ComponentSizeQuads = Core->ComponentsSize;

	// @todo: refactor from Landscape code
	// Find component range for this block of data
	int32 ComponentIndexX1 = (X1 >= 0) ? X1 / ComponentSizeQuads : (X1+1) / ComponentSizeQuads;
	int32 ComponentIndexY1 = (Y1 >= 0) ? Y1 / ComponentSizeQuads : (Y1+1) / ComponentSizeQuads;
	int32 ComponentIndexX2 = (X2-1 >= 0) ? (X2-1) / ComponentSizeQuads : (X2) / ComponentSizeQuads;
	int32 ComponentIndexY2 = (Y2-1 >= 0) ? (Y2-1) / ComponentSizeQuads : (Y2) / ComponentSizeQuads;
	
	// Shrink indices for shared values
	if ( ComponentIndexX2 < ComponentIndexX1)
	{
		ComponentIndexX2 = ComponentIndexX1;
	}
	if ( ComponentIndexY2 < ComponentIndexY1)
	{
		ComponentIndexY2 = ComponentIndexY1;
	}

	for (int32 ComponentIndexY = ComponentIndexY1; ComponentIndexY <= ComponentIndexY2; ComponentIndexY++)
	{
		for (int32 ComponentIndexX = ComponentIndexX1; ComponentIndexX <= ComponentIndexX2; ComponentIndexX++)
		{
			FIntPoint ComponentKey (ComponentIndexX,ComponentIndexY);
			
			TObjectPtr<UVelesLayerComponent> Component = Layer->GetLayerComponent(ComponentKey);

			if (!Component)
				continue;
			
			FVelesTextureDataInfo* TexDataInfo = nullptr;
			uint8* SelectTextureData = nullptr;
			UTexture2D* EditToolTexture = nullptr;
			
			if (Component)
			{
				EditToolTexture = Component->GetOrCreateComponentTexture();
			}
			
			if (EditToolTexture)
			{
				TexDataInfo = GetTextureDataInfo(EditToolTexture);
				SelectTextureData = (uint8*)TexDataInfo->GetMipData(0);
			}

			// Find coordinates of box that lies inside component
			int32 ComponentX1 = FMath::Clamp<int32>(X1 - ComponentIndexX * ComponentSizeQuads, 0, ComponentSizeQuads-1);
			int32 ComponentY1 = FMath::Clamp<int32>(Y1 - ComponentIndexY * ComponentSizeQuads, 0, ComponentSizeQuads-1);
			int32 ComponentX2 = FMath::Clamp<int32>(X2 - ComponentIndexX * ComponentSizeQuads, 0, ComponentSizeQuads-1);
			int32 ComponentY2 = FMath::Clamp<int32>(Y2 - ComponentIndexY * ComponentSizeQuads, 0, ComponentSizeQuads-1);

			for (int32 ComponentY = ComponentY1; ComponentY <= ComponentY2; ComponentY++)
			{
				for (int32 ComponentX = ComponentX1; ComponentX <= ComponentX2; ComponentX++)
				{
					const int32 TileX = ComponentX + ComponentIndexX * ComponentSizeQuads;
					const int32 TileY = ComponentY + ComponentIndexY * ComponentSizeQuads;
					
					// Find the input data corresponding to this vertex
					if (Component && SelectTextureData)
					{
						// Find the texture data corresponding to this vertex
						int32 SizeU = EditToolTexture->Source.GetSizeX();
						int32 SizeV = EditToolTexture->Source.GetSizeY();

						const int32 TexIndex = ComponentX + ComponentY * SizeU;
						uint8& TexData = SelectTextureData[TexIndex];

						Data.Add(FIntPoint(TileX, TileY), TexData);
					}
					else
					{
						Data.Add(FIntPoint(TileX, TileY), 0);
					}
				}
			}

			Component->RequestDataUpdate();
		}
	}
}

void FVelesLayerEditDataInterface::SetEditToolTextureData(const int32 X1, const int32 Y1, const int32 X2, const int32 Y2,
	const uint8* Data)
{
	// TRACE_CPUPROFILER_EVENT_SCOPE(LandscapeEditDataInterface_SetAlphaData);
	
	check(Data != nullptr);
	
	const int32 Stride = (1+X2-X1);
	const int32 ComponentSizeQuads = Core->ComponentsSize;

	check(ComponentSizeQuads > 0);
	// Find component range for this block of data
	int32 ComponentIndexX1 = (X1 >= 0) ? X1 / ComponentSizeQuads : (X1+1) / ComponentSizeQuads;
	int32 ComponentIndexY1 = (Y1 >= 0) ? Y1 / ComponentSizeQuads : (Y1+1) / ComponentSizeQuads;
	int32 ComponentIndexX2 = (X2-1 >= 0) ? (X2-1) / ComponentSizeQuads : (X2) / ComponentSizeQuads;
	int32 ComponentIndexY2 = (Y2-1 >= 0) ? (Y2-1) / ComponentSizeQuads : (Y2) / ComponentSizeQuads;
	
	// Shrink indices for shared values
	if ( ComponentIndexX2 < ComponentIndexX1)
	{
		ComponentIndexX2 = ComponentIndexX1;
	}
	if ( ComponentIndexY2 < ComponentIndexY1)
	{
		ComponentIndexY2 = ComponentIndexY1;
	}

	for (int32 ComponentIndexY = ComponentIndexY1; ComponentIndexY <= ComponentIndexY2; ComponentIndexY++)
	{
		for (int32 ComponentIndexX = ComponentIndexX1; ComponentIndexX <= ComponentIndexX2; ComponentIndexX++)
		{
			FIntPoint ComponentKey (ComponentIndexX, ComponentIndexY);

			TObjectPtr<UVelesLayerComponent> Component = Layer->GetLayerComponent(ComponentKey);

			if (!Component)
				continue;

			FVelesTextureDataInfo* TexDataInfo = nullptr;
			uint8* SelectTextureData = nullptr;
			UTexture2D* EditToolTexture = nullptr;
			
			if (Component)
			{
				EditToolTexture = Component->GetOrCreateComponentTexture();
			}
			
			if (EditToolTexture)
			{
				TexDataInfo = GetTextureDataInfo(EditToolTexture);
				SelectTextureData = (uint8*)TexDataInfo->GetMipData(0);
			}

			// Find coordinates of box that lies inside component
			int32 ComponentX1 = FMath::Clamp<int32>(X1 - ComponentIndexX * ComponentSizeQuads, 0, ComponentSizeQuads-1);
			int32 ComponentY1 = FMath::Clamp<int32>(Y1 - ComponentIndexY * ComponentSizeQuads, 0, ComponentSizeQuads-1);
			int32 ComponentX2 = FMath::Clamp<int32>(X2 - ComponentIndexX * ComponentSizeQuads, 0, ComponentSizeQuads-1);
			int32 ComponentY2 = FMath::Clamp<int32>(Y2 - ComponentIndexY * ComponentSizeQuads, 0, ComponentSizeQuads-1);

			for (int32 ComponentY = ComponentY1; ComponentY <= ComponentY2; ComponentY++)
			{
				for (int32 ComponentX = ComponentX1; ComponentX <= ComponentX2; ComponentX++)
				{
					// Find the input data corresponding to this vertex
					if (Component && SelectTextureData)
					{
						// Find the texture data corresponding to this vertex
						int32 SizeU = EditToolTexture->Source.GetSizeX();
						int32 SizeV = EditToolTexture->Source.GetSizeY();

						const int32 TileX = ComponentX + ComponentIndexX * ComponentSizeQuads;
						const int32 TileY = ComponentY + ComponentIndexY * ComponentSizeQuads;
						
						const int32 DataIndex = (TileX-X1) + Stride * (TileY-Y1);
						const uint8 NewData = Data[DataIndex];

						const int32 TexIndex = ComponentX + ComponentY * SizeU;
						uint8& TexData = SelectTextureData[TexIndex];

						TexData = NewData;
					}
				}
			}

			TexDataInfo->AddMipUpdateRegion(0,ComponentX1,ComponentY1,ComponentX2,ComponentY2);
		}
	}
}

void FVelesLayerEditDataInterface::Flush()
{
	// bool bNeedToWaitForUpdate = false;
	//
	// if (bUploadTextureChangesToGPU)
	{
		// Update all textures
		for (TMap<UTexture2D*, FVelesTextureDataInfo*>::TIterator It(TextureDataMap); It; ++It)
		{
			if (It.Value()->UpdateTextureData())
			{
				// bNeedToWaitForUpdate = true;
			}
		}
	}

	// if( bNeedToWaitForUpdate )
	// {
	// 	FlushRenderingCommands();
	// }

	// delete all the TextureDataInfo allocations
	for (TMap<UTexture2D*, FVelesTextureDataInfo*>::TIterator It(TextureDataMap); It;  ++It )
	{
		delete It.Value();	// TextureDataInfo destructors will unlock any texture data
	}

	TextureDataMap.Empty();
}

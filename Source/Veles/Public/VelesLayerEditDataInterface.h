// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AVelesCore;
class AVelesLayerBaseActor;

struct FVelesTextureDataInfo
{
	struct FMipInfo
	{
		void* MipData;
		TArray<FUpdateTextureRegion2D> MipUpdateRegions;
		bool bFull;
	};

	FVelesTextureDataInfo(UTexture2D* InTexture);
	~FVelesTextureDataInfo();

	bool UpdateTextureData();

	void* GetMipData(int32 MipNum)
	{
		check( MipNum < MipInfo.Num() );
		if( !MipInfo[MipNum].MipData )
		{
			MipInfo[MipNum].MipData = Texture->Source.LockMip(MipNum);
		}
		return MipInfo[MipNum].MipData;
	}

	void AddMipUpdateRegion(int32 MipNum, int32 InX1, int32 InY1, int32 InX2, int32 InY2)
	{
		if (MipInfo[MipNum].bFull)
		{
			return;
		}

		check(MipNum < MipInfo.Num());
		uint32 Width = 1 + InX2 - InX1;
		uint32 Height = 1 + InY2 - InY1;
		// Catch situation where we are updating the whole texture to avoid adding redundant regions once the whole region as been included.
		if (Width == GetMipSizeX(MipNum) && Height == GetMipSizeY(MipNum))
		{
			MipInfo[MipNum].bFull = true;
			MipInfo[MipNum].MipUpdateRegions.Reset();
			// Push a full region for UpdateTextureData() to process later
			new(MipInfo[MipNum].MipUpdateRegions) FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
			return;
		}

		new(MipInfo[MipNum].MipUpdateRegions) FUpdateTextureRegion2D(InX1, InY1, InX1, InY1, Width, Height);
	}

	int32 GetMipSizeX(int32 MipNum) const
	{
		return FMath::Max(Texture->Source.GetSizeX() >> MipNum, 1);
	}

	int32 GetMipSizeY(int32 MipNum) const
	{
		return FMath::Max(Texture->Source.GetSizeY() >> MipNum, 1);
	}
	
private:
	UTexture2D* Texture;
	TArray<FMipInfo> MipInfo;
};

struct VELES_API FVelesLayerEditDataInterface
{
	FVelesLayerEditDataInterface(AVelesCore* InCore, AVelesLayerBaseActor* InLayer);
	virtual ~FVelesLayerEditDataInterface();

	FVelesTextureDataInfo* GetTextureDataInfo(UTexture2D* Texture);

	void GetEditToolTextureData(const int32 X1, const int32 Y1, const int32 X2, const int32 Y2, TMap<FIntPoint, uint8>& Data);
	void SetEditToolTextureData(const int32 X1, const int32 Y1, const int32 X2, const int32 Y2, const uint8* Data);

	void Flush();

private:
	AVelesCore* Core;
	AVelesLayerBaseActor* Layer;

	TMap<UTexture2D*, FVelesTextureDataInfo*> TextureDataMap;
};

//
template<class Accessor, typename AccessorType>
struct TVelesLayerEditCache
{
public:
	typedef AccessorType DataType;
	Accessor DataAccess;

	TVelesLayerEditCache(AVelesCore* InCore, AVelesLayerBaseActor* InTarget)
		: DataAccess(InCore, InTarget)
		, EditInterface(InCore, InTarget)
	{
		// check(TargetActor.IsValid());
	}

	void CacheData(int32 X1, int32 Y1, int32 X2, int32 Y2)
	{
		if (!bIsValid)
		{
			CachedX1 = X1;
			CachedY1 = Y1;
			CachedX2 = X2;
			CachedY2 = Y2;

			DataAccess.GetDataFast(CachedX1, CachedY1, CachedX2, CachedY2, CachedData);

			// Drop a visual log to indicate the area covered by this cache region extension :
			VisualizeLandscapeRegion(CachedX1, CachedY1, CachedX2, CachedY2, FColor::Red, TEXT("Cache Data"));

			// bCacheOriginalData
			OriginalData = CachedData;

			bIsValid = true;
		}
		else
		{
			bool bCacheExtended = false;

			if (X1 < CachedX1)
			{
				DataAccess.GetDataFast(X1, CachedY1, CachedX1 - 1, CachedY2, CachedData);
				
				CacheOriginalData(X1, CachedY1, CachedX1 - 1, CachedY2);

				CachedX1 = X1;

				bCacheExtended = true;
			}

			if (X2 > CachedX2)
			{
				DataAccess.GetDataFast(CachedX2 + 1, CachedY1, X2, CachedY2, CachedData);
				
				CacheOriginalData(CachedX2 + 1, CachedY1, X2, CachedY2);

				CachedX2 = X2;

				bCacheExtended = true;
			}

			if (Y1 < CachedY1)
			{
				DataAccess.GetDataFast(CachedX1, Y1, CachedX2, CachedY1 - 1, CachedData);
				
				CacheOriginalData(CachedX1, Y1, CachedX2, CachedY1 - 1);

				CachedY1 = Y1;

				bCacheExtended = true;
			}

			if (Y2 > CachedY2)
			{
				DataAccess.GetDataFast(CachedX1, CachedY2 + 1, CachedX2, Y2, CachedData);
				
				CacheOriginalData(CachedX1, CachedY2 + 1, CachedX2, Y2);

				CachedY2 = Y2;

				bCacheExtended = true;
			}

			if (bCacheExtended)
			{
				VisualizeLandscapeRegion(CachedX1, CachedY1, CachedX2, CachedY2, FColor::Red, TEXT("Cache Data"));
			}
		}
	}

	AccessorType* GetValueRef(int32 LandscapeX, int32 LandscapeY)
	{
		return CachedData.Find(FIntPoint(LandscapeX, LandscapeY));
	}

	// X2/Y2 Coordinates are "inclusive" max values
	bool GetCachedData(int32 X1, int32 Y1, int32 X2, int32 Y2, TArray<AccessorType>& OutData)
	{
		const int32 XSize = (1 + X2 - X1);
		const int32 YSize = (1 + Y2 - Y1);
		const int32 NumSamples = XSize * YSize;
		OutData.Empty(NumSamples);
		OutData.AddUninitialized(NumSamples);
		bool bHasNonZero = false;

		for (int32 Y = Y1; Y <= Y2; Y++)
		{
			const int32 YOffset = (Y - Y1) * XSize;
			for (int32 X = X1; X <= X2; X++)
			{
				const int32 XYOffset = YOffset + (X - X1);
				AccessorType* Ptr = GetValueRef(X, Y);
				if (Ptr)
				{
					OutData[XYOffset] = *Ptr;
					if (!IsZeroValue(*Ptr))
					{
						bHasNonZero = true;
					}
				}
				else
				{
					OutData[XYOffset] = (AccessorType)0;
				}
			}
		}

		return bHasNonZero;
	}

	void SetValue(int32 LandscapeX, int32 LandscapeY, AccessorType Value)
	{
		CachedData.Add(FIntPoint(LandscapeX, LandscapeY), Forward<AccessorType>(Value));
	}

	bool IsZeroValue(const uint8& Value)
	{
		return Value == 0;
	}

	// X2/Y2 Coordinates are "inclusive" max values
	void SetCachedData(int32 X1, int32 Y1, int32 X2, int32 Y2, TArray<AccessorType>& Data, bool bUpdateData = true)
	{
		const int32 XSize = (1 + X2 - X1);
		const int32 YSize = (1 + Y2 - Y1);
		checkSlow(Data.Num() == XSize * YSize);

		// Update cache
		for (int32 Y = Y1; Y <= Y2; Y++)
		{
			for (int32 X = X1; X <= X2; X++)
			{
				SetValue(X, Y, Data[(X - X1) + (Y - Y1)*XSize]);
			}
		}

		if (bUpdateData)
		{
			// Update real data
			DataAccess.SetData(X1, Y1, X2, Y2, Data.GetData());
		}
	}

	// Get the original data before we made any changes with the SetCachedData interface.
	// X2/Y2 Coordinates are "inclusive" max values
	void GetOriginalData(int32 X1, int32 Y1, int32 X2, int32 Y2, TArray<AccessorType>& OutOriginalData)
	{
		const int32 XSize = (1 + X2 - X1);
		const int32 YSize = (1 + Y2 - Y1);
		const int32 NumSamples = XSize * YSize;
		OutOriginalData.Empty(NumSamples);
		OutOriginalData.AddUninitialized(NumSamples);

		for (int32 Y = Y1; Y <= Y2; Y++)
		{
			for (int32 X = X1; X <= X2; X++)
			{
				AccessorType* Ptr = OriginalData.Find(FIntPoint(X, Y));
				if (Ptr)
				{
					OutOriginalData[(X - X1) + (Y - Y1) * XSize] = *Ptr;
				}
			}
		}
	}

	void Flush()
	{
		DataAccess.Flush();
	}
	
private:
	// X2/Y2 Coordinates are "inclusive" max values
	void CacheOriginalData(int32 X1, int32 Y1, int32 X2, int32 Y2)
	{
		for (int32 Y = Y1; Y <= Y2; Y++)
		{
			for (int32 X = X1; X <= X2; X++)
			{
				FIntPoint Key = FIntPoint(X, Y);
				AccessorType* Ptr = CachedData.Find(Key);
				if (Ptr)
				{
					check(OriginalData.Find(Key) == NULL);
					OriginalData.Add(Key, *Ptr);
				}
			}
		}
	}

	void VisualizeLandscapeRegion(int32 InX1, int32 InY1, int32 InX2, int32 InY2, const FColor& InColor, const FString& InDescription)
	{
		// check(TargetActor.IsValid());
		// const FTransform& TargetTransform = TargetActor->GetTransform();
		// const FVector Min = TargetTransform.TransformPosition(FVector(InX1, InY1, 0));
		// const FVector Max = TargetTransform.TransformPosition(FVector(InX2, InY2, 0));
		// UE_VLOG_BOX(TargetActor.Get(), LogVelesLayersEditor, Log, FBox(Min, Max), InColor, TEXT("%s"), *InDescription);
	}
	
	TMap<FIntPoint, AccessorType> CachedData;
	TMap<FIntPoint, AccessorType> OriginalData;
	FVelesLayerEditDataInterface EditInterface;

	bool bIsValid = false;

	int32 CachedX1 = INDEX_NONE;
	int32 CachedY1 = INDEX_NONE;
	int32 CachedX2 = INDEX_NONE;
	int32 CachedY2 = INDEX_NONE;
};

struct FVelesLayerDataAccessor
{
	FVelesLayerDataAccessor(AVelesCore* InCore, AVelesLayerBaseActor* InTarget)
		: EditInterface(InCore, InTarget)
	{
	}
	
	void GetDataFast(const int32 X1, const int32 Y1, const int32 X2, const int32 Y2, TMap<FIntPoint, uint8>& Data)
	{
		EditInterface.GetEditToolTextureData(X1, Y1, X2, Y2, Data);
	}
	
	void SetData(const int32 X1, const int32 Y1, const int32 X2, const int32 Y2, const uint8* Data)
	{
		EditInterface.SetEditToolTextureData(X1, Y1, X2, Y2, Data);
	}

	void Flush()
	{
		EditInterface.Flush();
	}
	
private:
	FVelesLayerEditDataInterface EditInterface;
};

struct FLandscapeAlphaCache : public TVelesLayerEditCache<FVelesLayerDataAccessor, uint8>
{
	static uint8 ClampValue(int32 Value) { return FMath::Clamp(Value, 0, 255); }

	FLandscapeAlphaCache(AVelesCore* InCore, AVelesLayerBaseActor* InTarget)
		: TVelesLayerEditCache<FVelesLayerDataAccessor, uint8>(InCore, InTarget)
	{
	}
};


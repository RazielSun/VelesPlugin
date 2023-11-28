// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesLayerComponent.h"

#include "VelesLayerBaseActor.h"


UVelesLayerComponent::UVelesLayerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	, ComponentBaseX(0)
	, ComponentBaseY(0)
	, ComponentSizeQuads(0)
	, LayerUpdateFlag(0)
{
	PrimaryComponentTick.bCanEverTick = false;
}

UTexture2D* UVelesLayerComponent::GetOrCreateComponentTexture()
{
	if (ComponentTexture == nullptr)
	{
		InitComponentTexture();
	}
	return GetComponentTexture();
}

void UVelesLayerComponent::ClearComponentTexture()
{
	if (ComponentTexture)
	{
		ComponentTexture = nullptr;
	}
}

AVelesLayerBaseActor* UVelesLayerComponent::GetProxyActor()
{
	return Cast<AVelesLayerBaseActor>(GetOuter());
}

const AVelesLayerBaseActor* UVelesLayerComponent::GetProxyActor() const
{
	return Cast<AVelesLayerBaseActor>(GetOuter());
}

void UVelesLayerComponent::RequestDataUpdate(bool bUpdateAll)
{
	if (bUpdateAll)
	{
		LayerUpdateFlag = 1; // @todo:
	}
	else
	{
		LayerUpdateFlag = 0;
	}

	if (AVelesLayerBaseActor* ProxyActor = GetProxyActor())
	{
		if (bUpdateAll)
		{
			ProxyActor->RequestContentUpdate(this);
		}
	}
}

void UVelesLayerComponent::InitComponentTexture()
{
	UTexture2D* NewTexture = NewObject<UTexture2D>(GetOuter(), MakeUniqueObjectName(GetOuter(), UTexture2D::StaticClass(), GetComponentTextureName()), RF_Standalone | RF_Public);
	NewTexture->Source.Init(ComponentSizeQuads, ComponentSizeQuads, 1, 1, TSF_G8);
	NewTexture->SRGB = false;
	NewTexture->CompressionNone = true;
	NewTexture->MipGenSettings = TMGS_NoMipmaps;
	NewTexture->AddressX = TA_Clamp;
	NewTexture->AddressY = TA_Clamp;
	ComponentTexture = NewTexture;

	uint8* TexData = ComponentTexture->Source.LockMip(0);
	FMemory::Memzero(TexData, ComponentSizeQuads*ComponentSizeQuads*sizeof(uint8));
	ComponentTexture->Source.UnlockMip(0);

	ComponentTexture->UpdateResource();
	ComponentTexture->PostEditChange();

	//Flush RHI thread after creating texture render target to make sure that RHIUpdateTextureReference is executed before doing any rendering with it
	//This makes sure that Value->TextureReference.TextureReferenceRHI->GetReferencedTexture() is valid so that FUniformExpressionSet::FillUniformBuffer properly uses the texture for rendering, instead of using a fallback texture
	ENQUEUE_RENDER_COMMAND(FlushRHIThreadToUpdateTextureRenderTargetReference)(
		[](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
		});
}

FName UVelesLayerComponent::GetComponentTextureName()
{
	return TEXT("Layer");
}

void UVelesLayerComponent::GetComponentExtent(int32& MinX, int32& MinY, int32& MaxX, int32& MaxY) const
{
	MinX = FMath::Min(ComponentBaseX, MinX);
	MinY = FMath::Min(ComponentBaseY, MinY);
	MaxX = FMath::Max(ComponentBaseX + ComponentSizeQuads, MaxX);
	MaxY = FMath::Max(ComponentBaseY + ComponentSizeQuads, MaxY);
}

void UVelesLayerComponent::Init(int32 InBaseX, int32 InBaseY, int32 InComponentSizeQuads)
{
	AVelesLayerBaseActor* ProxyActor = GetProxyActor();
	check(ProxyActor && !ProxyActor->LayersComponents.Contains(this));
	ProxyActor->LayersComponents.Add(this);

	//
	ProxyActor->AddInstanceComponent(this);

	SetComponentBase(FIntPoint(InBaseX, InBaseY));
	SetRelativeLocation(FVector(GetComponentBase(), 0));
	ComponentSizeQuads = InComponentSizeQuads;

	AttachToComponent(ProxyActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

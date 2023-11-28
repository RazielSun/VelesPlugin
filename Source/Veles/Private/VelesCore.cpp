// Fill out your copyright notice in the Description page of Project Settings.


#include "VelesCore.h"

#include "VelesCoreSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"

AVelesCore::AVelesCore(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	, NumComponentsX(0)
	, NumComponentsY(0)
	, ComponentsSize(0)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVelesCore::Clear()
{
	for (const auto& Pair : CompositeTextures)
	{
		if (!Pair.Value)
			continue;
		
		ENQUEUE_RENDER_COMMAND(FComputeShaderRunner)(
			[CompositeResource = Pair.Value->GetResource()](FRHICommandListImmediate& RHICommands)
		{
				check(IsInRenderingThread());
		
				RHICommands.Transition(FRHITransitionInfo(CompositeResource->GetTexture2DRHI(), ERHIAccess::SRVMask, ERHIAccess::RTV));
				FRHIRenderPassInfo RPInfo(CompositeResource->GetTexture2DRHI(), ERenderTargetActions::Clear_Store);
				RHICommands.BeginRenderPass(RPInfo, TEXT("Clear"));
				RHICommands.EndRenderPass();
				RHICommands.Transition(FRHITransitionInfo(CompositeResource->GetTexture2DRHI(), ERHIAccess::RTV, ERHIAccess::SRVMask));
		});
	}
}

void AVelesCore::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (!IsPendingKillPending())
	{
		if (UWorld* OwningWorld = GetWorld())
		{
			if (UVelesCoreSubsystem* CoreSubsystem = OwningWorld->GetSubsystem<UVelesCoreSubsystem>())
			{
				CoreSubsystem->RegisterCoreActor(this);
			}
		}
	}
}

void AVelesCore::UnregisterAllComponents(bool bForReregister)
{
	// On shutdown the world will be unreachable
	if (GetWorld() && IsValidChecked(GetWorld()) && !GetWorld()->IsUnreachable())
	{
		if (UVelesCoreSubsystem* CoreSubsystem = GetWorld()->GetSubsystem<UVelesCoreSubsystem>())
		{
			CoreSubsystem->UnregisterCoreActor(this);
		}
	}
	
	Super::UnregisterAllComponents(bForReregister);
}

TObjectPtr<UTextureRenderTarget2D> AVelesCore::GetOrCreateCompositeTexture(const FIntPoint& InKey)
{
	if (!CompositeTextures.Contains(InKey))
		CompositeTextures.Add(InKey, CreateRenderTargetTexture());
	if (const TObjectPtr<UTextureRenderTarget2D>* CompositeTexture = CompositeTextures.Find(InKey))
	{
		return *CompositeTexture;
	}
	return nullptr;
}

FBox AVelesCore::GetTileBounds(int32 X, int32 Y, int32 Size)
{
	if (!CachedTransform.IsSet())
	{
		CachedTransform = FTransform(FQuat::Identity, WorldLocation, FVector(WorldScale, 1));
	}
	const FVector LocalMin = FVector(X * Size, Y * Size, 0);
	const FVector LocalMax = FVector((X + 1)* Size, (Y + 1) * Size, 0);
	return FBox(CachedTransform->TransformPosition(LocalMin), CachedTransform->TransformPosition(LocalMax));
}

#if WITH_EDITOR
void AVelesCore::Setup(const FGuid& InGuid, int32 InNumComponentsX, int32 InNumComponentsY, int32 InComponentsSizeQuads,
	const FVector2D& InWorldOffset, const FVector& InWorldLocation, const FVector2D& InWorldScale)
{
	check(InGuid.IsValid());
	VelesGuid = InGuid;

	NumComponentsX = InNumComponentsX;
	NumComponentsY = InNumComponentsY;
	ComponentsSize = InComponentsSizeQuads;

	WorldOffset = InWorldOffset;
	WorldLocation = InWorldLocation;
	WorldScale = InWorldScale;
}
#endif

UTextureRenderTarget2D* AVelesCore::CreateRenderTargetTexture() const
{
	UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>(GetOuter(), MakeUniqueObjectName(GetOuter(), UTextureRenderTarget2D::StaticClass(), TEXT("TempComputeRT")), RF_Public);
	NewRenderTarget2D->RenderTargetFormat = ETextureRenderTargetFormat::RTF_R8;
	NewRenderTarget2D->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	NewRenderTarget2D->AddressX = TA_Clamp;
	NewRenderTarget2D->AddressY = TA_Clamp;
	NewRenderTarget2D->bAutoGenerateMips = false;
	NewRenderTarget2D->bCanCreateUAV = true;
	NewRenderTarget2D->InitAutoFormat(ComponentsSize, ComponentsSize);
	NewRenderTarget2D->UpdateResourceImmediate(true);

	//Flush RHI thread after creating texture render target to make sure that RHIUpdateTextureReference is executed before doing any rendering with it
	//This makes sure that Value->TextureReference.TextureReferenceRHI->GetReferencedTexture() is valid so that FUniformExpressionSet::FillUniformBuffer properly uses the texture for rendering, instead of using a fallback texture
	ENQUEUE_RENDER_COMMAND(FlushRHIThreadToUpdateTextureRenderTargetReference)(
		[](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
		});

	return NewRenderTarget2D;
}

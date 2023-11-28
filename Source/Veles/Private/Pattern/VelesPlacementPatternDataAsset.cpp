// Fill out your copyright notice in the Description page of Project Settings.

#include "Pattern/VelesPlacementPatternDataAsset.h"

#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Pattern/VelesPlacementPatternGenerator.h"

UVelesPlacementPatternDataAsset::UVelesPlacementPatternDataAsset(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
                                                                                                                , Seed(643533)
{
	//
}

#if WITH_EDITOR
void UVelesPlacementPatternDataAsset::Generate()
{
	if (Generator)
	{
		Pattern = Generator->Generate(Seed);
	}
}

void UVelesPlacementPatternDataAsset::GetNewSeed()
{
	Seed = FMath::RandRange(0, INT_MAX - 1);
}

void UVelesPlacementPatternDataAsset::Draw()
{
	InitDebugTexture();

	if (DebugTexture)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, DebugTexture, FLinearColor::Black);

		UCanvas* Canvas;
		FVector2D CanvasToRenderTargetSize;
		FDrawToRenderTargetContext RenderTargetContext;
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GWorld, DebugTexture, /*out*/ Canvas, /*out*/ CanvasToRenderTargetSize, /*out*/ RenderTargetContext);
		check(::IsValid(Canvas));

		const float Size = DebugTextureSize * 0.8f;
		const float Offset = (DebugTextureSize - Size) * 0.5f;
		const FVector2D ScreenSize(Size, Size);
		const FVector2D ScreenOffset(Offset, Offset);
		Canvas->UCanvas::K2_DrawBox(ScreenOffset, ScreenSize, 1.0f);

		for (const auto& Point : Pattern)
		{
			// @todo: Canvas->UCanvas::K2_DrawMaterial() for Circle
			FLinearColor Color = FLinearColor::MakeRandomColor();
			const FVector2D PointSize = FVector2D(2,2);
			Canvas->UCanvas::K2_DrawBox(ScreenOffset + FVector2D(Point.X, Point.Y) * ScreenSize + PointSize * 0.5, PointSize, 1.0f, Color);
		}

		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GWorld, RenderTargetContext);
	}
}

void UVelesPlacementPatternDataAsset::InitDebugTexture()
{
	if (DebugTexture == nullptr)
	{
		UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>(this, MakeUniqueObjectName(GetOuter(), UTextureRenderTarget2D::StaticClass(), TEXT("DebugPatternComputeRT")), RF_Public);
		NewRenderTarget2D->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		NewRenderTarget2D->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		NewRenderTarget2D->AddressX = TA_Clamp;
		NewRenderTarget2D->AddressY = TA_Clamp;
		NewRenderTarget2D->bAutoGenerateMips = false;
		// NewRenderTarget2D->bCanCreateUAV = true;
		NewRenderTarget2D->InitAutoFormat(DebugTextureSize, DebugTextureSize);
		NewRenderTarget2D->UpdateResourceImmediate(true);
		DebugTexture = NewRenderTarget2D;

		//Flush RHI thread after creating texture render target to make sure that RHIUpdateTextureReference is executed before doing any rendering with it
		//This makes sure that Value->TextureReference.TextureReferenceRHI->GetReferencedTexture() is valid so that FUniformExpressionSet::FillUniformBuffer properly uses the texture for rendering, instead of using a fallback texture
		ENQUEUE_RENDER_COMMAND(FlushRHIThreadToUpdateTextureRenderTargetReference)(
			[](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
			});
	}
}
#endif


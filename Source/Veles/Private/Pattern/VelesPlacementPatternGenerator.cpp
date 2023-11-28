// Fill out your copyright notice in the Description page of Project Settings.


#include "Pattern/VelesPlacementPatternGenerator.h"

constexpr double FOOTPRINT_NRM = 1000.0;

TArray<FVector4f> UVelesPlacementPatternGenerator::Generate(int32 InSeed) const
{
	checkf(false, TEXT("Abstract function, pleas override"));
	return TArray<FVector4f>();
}

bool UVelesPlacementPatternGenerator::IsIntersected(TArray<FSphere3d>& InSpheres, const FVector4& InPoint, float MaxFootprint, bool bAsPoint) const
{
	const FSphere3d Sphere(FVector(InPoint), InPoint.W);
	return IsIntersected(InSpheres, Sphere, MaxFootprint);
}

bool UVelesPlacementPatternGenerator::IsIntersected(TArray<FSphere3d>& InSpheres, const FSphere3d& InSphere,
	float MaxFootprint, bool bAsPoint) const
{
	auto CheckFunc = [bAsPoint](const FSphere3d& A, const FSphere3d& B) -> bool
	{
		if (bAsPoint)
		{
			if (A.IsInside(B.Center))
			{
				return true;
			}
		}
		else
		{
			if (A.Intersects(B))
			{
				return true;
			}
		}
		return false;
	};
	
	for (const auto& OtherSphere : InSpheres)
	{
		if (CheckFunc(OtherSphere, InSphere))
			return true;
	}
	
	if (InSphere.Center.X < MaxFootprint)
	{
		FSphere3d NewSphere(InSphere);
		NewSphere.Center.X = 1.0 + InSphere.Center.X;
		for (const auto& OtherSphere : InSpheres)
		{
			if (CheckFunc(OtherSphere, NewSphere))
				return true;
		}
	}
	else if (FMath::Abs(InSphere.Center.X - 1.0) < MaxFootprint)
	{
		FSphere3d NewSphere(InSphere);
		NewSphere.Center.X = -FMath::Abs(InSphere.Center.X - 1.0);
		for (const auto& OtherSphere : InSpheres)
		{
			if (CheckFunc(OtherSphere, NewSphere))
				return true;
		}
	}
	if (InSphere.Center.Y < MaxFootprint)
	{
		FSphere3d NewSphere(InSphere);
		NewSphere.Center.Y = 1.0 + InSphere.Center.Y;
		for (const auto& OtherSphere : InSpheres)
		{
			if (CheckFunc(OtherSphere, NewSphere))
				return true;
		}
	}
	else if (FMath::Abs(InSphere.Center.Y - 1.0) < MaxFootprint)
	{
		FSphere3d NewSphere(InSphere);
		NewSphere.Center.Y = -FMath::Abs(InSphere.Center.Y - 1.0);
		for (const auto& OtherSphere : InSpheres)
		{
			if (CheckFunc(OtherSphere, NewSphere))
				return true;
		}
	}
	return false;
}

UVelesPlacementPatternGridGenerator::UVelesPlacementPatternGridGenerator() : GridSize(10,10)
{
}

TArray<FVector4f> UVelesPlacementPatternGridGenerator::Generate(int32 InSeed) const
{
	TArray<FVector4f> Pattern;
		
	Pattern.SetNumUninitialized(GridSize.X * GridSize.Y);

	const FVector2f Step = FVector2f::One() / static_cast<float>(GridSize.X);
	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			Pattern[Y * GridSize.X + X] = FVector4f(Step.X * (X + 0.5f), Step.Y * (Y + 0.5f), 0, 1);
		}
	}
		
	return Pattern;
}

UVelesPlacementPatternRandomGenerator::UVelesPlacementPatternRandomGenerator() : ItemCount(100)
{
}

TArray<FVector4f> UVelesPlacementPatternRandomGenerator::Generate(int32 InSeed) const
{
	const FRandomStream RandomStream(InSeed);
		
	TArray<FVector4f> Pattern;
		
	Pattern.SetNumUninitialized(ItemCount);
		
	for (int32 Index = 0; Index < ItemCount; ++Index)
	{
		const double RndX = RandomStream.GetFraction();
		const double RndY = RandomStream.GetFraction();
		const double Z = 0;
		const double W = 1;

		Pattern[Index] = FVector4f(RndX, RndY, Z, W);
	}
		
	return Pattern;
}

UVelesPlacementPatternBlueNoiseGenerator::UVelesPlacementPatternBlueNoiseGenerator() : ItemCount(100), Footprint(100, 100)
{
	//
}
	
TArray<FVector4f> UVelesPlacementPatternBlueNoiseGenerator::Generate(int32 InSeed) const
{
	const FRandomStream RandomStream(InSeed);
		
	TArray<FVector4f> Pattern;
		
	int32 MaxAttempts = ItemCount * 100;
	int32 Count = ItemCount;
	const float FootprintMax = Footprint.Y / FOOTPRINT_NRM;

	TArray<FSphere3d> Spheres;
		
	while (MaxAttempts > 0 && Count > 0)
	{
		const double RndX = RandomStream.GetFraction();
		const double RndY = RandomStream.GetFraction();
		const double RndF = RandomStream.GetFraction() * (Footprint.Y - Footprint.X) + Footprint.X;

		const FVector4 Point = FVector4(RndX, RndY, 0, RndF / FOOTPRINT_NRM);
		const bool bIntersected = IsIntersected(Spheres, Point, FootprintMax);

		if (!bIntersected)
		{
			Pattern.Add(FVector4f(Point));
			Spheres.Add(FSphere3d(FVector(Point), Point.W));

			Count--;
		}
			
		MaxAttempts--;
	}
		
	return Pattern;
}

UVelesPlacementPatternHexGenerator::UVelesPlacementPatternHexGenerator() : Footprint(100, 100), JitterXY(0.1, 0.1), JitterRadius(0.3), ExtendBorder(0.1)
	{
	}

TArray<FVector4f> UVelesPlacementPatternHexGenerator::Generate(int32 InSeed) const
	{
		const FRandomStream RandomStream(InSeed);
		
		TArray<FVector4f> Pattern;
		
		const float FootprintMax = Footprint.Y / FOOTPRINT_NRM;
		const float FpBorderZero = -FootprintMax * ExtendBorder;
		const float FpBorderZeroOne = 1.0 - FpBorderZero;
		
		const double StartRndX = RandomStream.GetFraction();
		const double StartRndY = RandomStream.GetFraction();
		const double StartRndF = RandomStream.GetFraction() * (Footprint.Y - Footprint.X) + Footprint.X;

		TArray<FSphere3d> ToProcess;
		TArray<FSphere3d> Spheres;

		TArray<FVector2d> Around = { {0,1}, {3.0/4.0, 0.5}, {3.0/4.0, -0.5}, {0,-1}, {-3.0/4.0, -0.5}, {-3.0/4.0, 0.5} };

		ToProcess.Add(FSphere3d(FVector(StartRndX, StartRndY, 0), StartRndF / FOOTPRINT_NRM));

		while (ToProcess.Num() > 0)
		{
			TArray<FSphere3d> NextProcess;
			for (int32 i = 0; i < ToProcess.Num(); ++i)
			{
				const auto& Sphere = ToProcess[i];

				const bool bIntersected = IsIntersected(Spheres, Sphere, FootprintMax, true);
				if (!bIntersected)
				{
					Spheres.Add(Sphere);

					for (const FVector2d& V : Around)
					{
						const double RndF = RandomStream.GetFraction() * (Footprint.Y - Footprint.X) + Footprint.X;
						const double Size = 2 - JitterRadius * 0.5 + JitterRadius * RandomStream.GetFraction();
						const double Diam = Sphere.W * Size;
						FSphere3d NewSphere(FVector(V.X * Diam + Sphere.Center.X, V.Y * Diam + Sphere.Center.Y, Sphere.Center.Z), RndF / FOOTPRINT_NRM);
						
						const double JitterX = RandomStream.GetFraction();
						const double JitterY = RandomStream.GetFraction();

						NewSphere.Center.X += NewSphere.W * JitterXY.X * (JitterX - 0.5);
						NewSphere.Center.Y += NewSphere.W * JitterXY.Y * (JitterY - 0.5);

						if (NewSphere.Center.X >= FpBorderZero && NewSphere.Center.Y >= FpBorderZero && NewSphere.Center.X <= FpBorderZeroOne && NewSphere.Center.Y <= FpBorderZeroOne)
							NextProcess.Add(NewSphere);
					}
				}
			}
			ToProcess = NextProcess;
		}

		for (const auto& Sphere : Spheres)
		{
			Pattern.Add(FVector4f(Sphere.Center.X, Sphere.Center.Y, Sphere.Center.Z, Sphere.W));
		}
		
		return Pattern;
	}
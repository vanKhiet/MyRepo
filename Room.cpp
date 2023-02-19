// Fill out your copyright notice in the Description page of Project Settings.


#include "Procedural/Room.h"

#include "Procedural/Dot.h"
#include "Procedural/EmMath.h"



ARoom::ARoom()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARoom::BeginPlay()
{
	Super::BeginPlay();

	DotArray.Reserve(10);

	GetActorBounds(false, RoomOrigin, RoomBoxExtent, false);
	RoomLengthX = RoomBoxExtent.X * 2;
	RoomLengthY = RoomBoxExtent.Y * 2;
	RoomLengthZ = RoomBoxExtent.Z * 2;

	for (int i = 0; i < 4; i++)
	{
		SpawnItem(Item, DotArray);
	}

	// ConnectDots(DotArray);
	TArray<FemTriangle> Triangulation = {};
	BowyerWatson(DotArray, Triangulation);
	ConnectBowyerWatsonTriangles(Triangulation);
}

void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoom::SpawnItem(UClass* ActorToSpawn, TArray<ADot*>& DotArrayPara)
{
	if (GetWorld())
	{
		float ItemLocationX = RoomOrigin.X + FMath::FRandRange(-RoomLengthX / 2, RoomLengthX / 2);
		float ItemLocationY = RoomOrigin.Y + FMath::FRandRange(-RoomLengthY / 2, RoomLengthY / 2);
		ItemLocationZ = RoomOrigin.Z + RoomLengthZ/2;

		ADot* MyDot = NewObject<ADot>();
		MyDot->Init(FVector(ItemLocationX, ItemLocationY, ItemLocationZ), FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));
		DotArrayPara.Add(MyDot);

		GetWorld()->SpawnActor<AActor>(ActorToSpawn, MyDot->DotLocation, MyDot->DotRotator);
	}
}



void ARoom::ConnectDots(const TArray<ADot*>& DotArrayPara)
{
	if (!GetWorld())
	{
		return;
	}

	for (int i = 0; i < DotArrayPara.Num(); i++)
	{
		for (int j = 0; j < DotArrayPara.Num(); j++)
		{
			DrawDebugLine(GetWorld(), DotArrayPara[i]->DotLocation, DotArrayPara[j]->DotLocation, FColor::Black, true, -1.0f, (uint8)0U, 10);
		}
	}
}


void ARoom::BowyerWatson(const TArray<ADot*>& DotArrayPara, TArray<FemTriangle>& TriangulationPara)
{	
	// Triangulation = empty triangle mesh data structure

	// Add super-triangle to triangulation
	FemTriangle SuperTriangle = CreateSuperTriangle(DotArrayPara);
	TriangulationPara.Add(SuperTriangle);

	// For each point in pointList do
	for (int PointIndex = 0; PointIndex < DotArrayPara.Num(); PointIndex++)
	{
		// BadTriangles = empty set
		TArray<FemTriangle> BadTriangles = {};
		// For each triangle in triangulation do
		for (int TriangleIndex = 0; TriangleIndex < TriangulationPara.Num(); TriangleIndex++)
		{
			// If point is inside circumcircle of triangle
			if (FVector::Distance(DotArrayPara[PointIndex]->DotLocation, TriangulationPara[TriangleIndex].CircumCentroid) < TriangulationPara[TriangleIndex].CircumRadius)
			{
				// Add triangle to badTriangles
				BadTriangles.Add(TriangulationPara[TriangleIndex]);
			}
		}

		// Polygon = empty set
		TArray<FemEdge> Polygon = {};
		// If there is only 1 triangle in badTriangles
		if (BadTriangles.Num() == 1)
		{
			for (int EdgeIndex = 0; EdgeIndex < 3; EdgeIndex++)
			{
				Polygon.Add(BadTriangles[0].Edges[EdgeIndex]);
			}
		}
		else
		{
			// For each triangle in badTriangles do
			for (int TriangleIndex = 0; TriangleIndex < BadTriangles.Num(); TriangleIndex++)
			{
				// For each edge in triangle do
				for (int EdgeIndex = 0; EdgeIndex < 3; EdgeIndex++)
				{
					// If edge is not shared by any other triangles in badTriangles
					for (int OtherTriangleIndex = 0; OtherTriangleIndex < BadTriangles.Num(); OtherTriangleIndex++)
					{
						// If other triangle is not current triangle
						if (TriangleIndex != OtherTriangleIndex)
							GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Line executed"));
						{
							bool IfEdgeIsNotShared = FemTriangle::IFemEdgeIsNotShared(BadTriangles[TriangleIndex].Edges[EdgeIndex], BadTriangles[OtherTriangleIndex]);
							if (IfEdgeIsNotShared)
							{
								// Add edge to polygon
								Polygon.Add(BadTriangles[TriangleIndex].Edges[EdgeIndex]);
							}
						}
					}
				}
			}
		}

		// For each triangle in badTriangles do
		for (int TriangleIndex = 0; TriangleIndex < BadTriangles.Num(); TriangleIndex++)
		{
			for (int TriangulationIndex = 0; TriangulationIndex < TriangulationPara.Num(); TriangulationIndex++)
			{
				if (FemTriangle::IsEqual(BadTriangles[TriangleIndex], TriangulationPara[TriangulationIndex]))
				{
					TriangulationPara.RemoveAt(TriangleIndex);
					break;
				}
			}
		}

		/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Triangulation element count: ") + FString::FromInt(Triangulation.Num()));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Bad triangles element count: ") + FString::FromInt(BadTriangles.Num()));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Polygon element count: ") + FString::FromInt(Polygon.Num()));*/

		// For each edge in polygon do
		for (int EdgeIndex = 0; EdgeIndex < Polygon.Num(); EdgeIndex++)
		{
			// NewTri = form a triangle from edge to point
			FemTriangle NewTri = FemTriangle(Polygon[EdgeIndex].Vertices[0], Polygon[EdgeIndex].Vertices[1], DotArrayPara[PointIndex]->DotLocation);
			// Add newTri to triangulation
			TriangulationPara.Add(NewTri);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::FromInt(TriangulationPara.Num()));
		}
	}

	 // Version with contain

	// For each triangle in triangulation
	for (int TriangleIndex = 0; TriangleIndex < TriangulationPara.Num(); TriangleIndex++)
	{
		// If triangle contains a vertex from original super-triangle
		for(int VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
			
			if ( SuperTriangle.Vertices.Contains(TriangulationPara[TriangleIndex].Vertices[VertexIndex]) )
			{
				// Remove triangle from triangulation
				TriangulationPara.RemoveAt(TriangleIndex);
			}
		}
	}

	// Without contain

	// For each triangle in triangulation
	//for (int TriangleIndex = 0; TriangleIndex < Triangulation.Num(); TriangleIndex++)
	//{
	//	// If triangle contains a vertex from original super-triangle
	//	for (int VertexIndex = 0; VertexIndex < Triangulation[TriangleIndex].Vertices.Num(); VertexIndex++)
	//	{
	//		for (int SuperVertexIndex = 0; SuperVertexIndex < 3; SuperVertexIndex++)
	//		{
	//			if (SuperTriangle.Vertices[SuperVertexIndex] == Triangulation[TriangleIndex].Vertices[VertexIndex])
	//			{
	//				Triangulation.RemoveAt(TriangleIndex);
	//			}
	//		}
	//	}
	//}
}


FemTriangle ARoom::CreateSuperTriangle(const TArray<ADot*>& DotArrayPara)
{
	double MinX = DotArrayPara[0]->DotLocation.X;
	for (int i = 1; i < DotArrayPara.Num(); i++)
	{
		if (DotArrayPara[i]->DotLocation.X < MinX)
		{
			MinX = DotArrayPara[i]->DotLocation.X;
		}
	}
	double MaxX = DotArrayPara[0]->DotLocation.X;
	for (int i = 1; i < DotArrayPara.Num(); i++)
	{
		if (DotArrayPara[i]->DotLocation.X > MaxX)
		{
			MaxX = DotArrayPara[i]->DotLocation.X;
		}
	}
	double MinY = DotArrayPara[0]->DotLocation.Y;
	for (int i = 1; i < DotArrayPara.Num(); i++)
	{
		if (DotArrayPara[i]->DotLocation.Y < MinY)
		{
			MinY = DotArrayPara[i]->DotLocation.Y;
		}
	}
	double MaxY = DotArrayPara[0]->DotLocation.Y;
	for (int i = 1; i < DotArrayPara.Num(); i++)
	{
		if (DotArrayPara[i]->DotLocation.Y > MaxY)
		{
			MaxY = DotArrayPara[i]->DotLocation.Y;
		}
	}
	double CircleRadius = FVector::Distance(FVector(MinX, MinY, ItemLocationZ), FVector(MaxX, MaxY, ItemLocationZ)) / 2;
	FVector CircleCenter = FVector(MinX + (MaxX - MinX) / 2, MinY + (MaxY - MinY) / 2, ItemLocationZ);

	double BigCircleRatio = 2.5;
	FVector PointA = FVector(CircleCenter.X, CircleCenter.Y + CircleRadius * BigCircleRatio, ItemLocationZ);
	FVector PointB = FVector(CircleCenter.X - (FMath::Sin(FMath::DegreesToRadians(60.0)) * CircleRadius * BigCircleRatio), CircleCenter.Y - (FMath::Cos(FMath::DegreesToRadians(60.0)) * CircleRadius * BigCircleRatio), ItemLocationZ);
	FVector PointC = FVector(CircleCenter.X + (FMath::Sin(FMath::DegreesToRadians(60.0)) * CircleRadius * BigCircleRatio), CircleCenter.Y - (FMath::Cos(FMath::DegreesToRadians(60.0)) * CircleRadius * BigCircleRatio), ItemLocationZ);

	FemTriangle SuperTriangle = FemTriangle(PointA, PointB, PointC);

	if (GetWorld())
	{
		DrawDebugLine(GetWorld(), PointA, PointB, FColor::Black, true, -1.0f, (uint8)0U, 10);
		DrawDebugLine(GetWorld(), PointB, PointC, FColor::Black, true, -1.0f, (uint8)0U, 10);
		DrawDebugLine(GetWorld(), PointC, PointA, FColor::Black, true, -1.0f, (uint8)0U, 10);
	}

	DrawDebugSphere(GetWorld(), PointA, 20, 32, FColor::Red, true);
	DrawDebugSphere(GetWorld(), PointB, 20, 32, FColor::Red, true);
	DrawDebugSphere(GetWorld(), PointC, 20, 32, FColor::Red, true);

	return SuperTriangle;
}

void ARoom::ConnectBowyerWatsonTriangles(const TArray<FemTriangle>& TriangulationPara)
{
	if (!GetWorld())
	{
		return;
	}

	for (int TriangleIndex = 0; TriangleIndex < TriangulationPara.Num(); TriangleIndex++)
	{
		for (int EdgeIndex = 0; EdgeIndex < 3; EdgeIndex++)
		{
			FVector Vector1 = TriangulationPara[TriangleIndex].Edges[EdgeIndex].Vertices[0];
			FVector Vector2 = TriangulationPara[TriangleIndex].Edges[EdgeIndex].Vertices[1];
			DrawDebugLine(GetWorld(), Vector1, Vector2, FColor::Black, true, -1.0f, (uint8)0U, 10);
		}
	}
}

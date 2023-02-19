// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Room.generated.h"

class ADot;
struct FemTriangle;

UCLASS()
class ENDLESSMAZE_API ARoom : public AActor
{
	GENERATED_BODY()
	
	ARoom();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	class UStaticMeshComponent* Floor;

	UPROPERTY(EditAnywhere, Category = "CPP Input")
	class TSubclassOf<AActor> Item;

	UPROPERTY(VisibleAnywhere, Category = "CPP Information")
	FVector RoomOrigin;
	FVector RoomBoxExtent;

	double RoomLengthX;
	double RoomLengthY;
	double RoomLengthZ;

	double ItemLocationZ;

	TArray<ADot*> DotArray;

	void SpawnItem(UClass* ActorToSpawn, TArray<ADot*>& DotArrayPara);
	void ConnectDots(const TArray<ADot*>& DotArrayPara);
	void BowyerWatson(const TArray<ADot*>& DotArrayPara, TArray<FemTriangle>& TriangulationPara);

	FemTriangle CreateSuperTriangle(const TArray<ADot*>& DotArrayPara);
	void ConnectBowyerWatsonTriangles(const TArray<FemTriangle>& TriangulationPara);
};

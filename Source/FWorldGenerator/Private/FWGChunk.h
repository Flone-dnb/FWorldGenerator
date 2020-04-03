// This file is part of the FWorldGenenerator.
// Copyright Aleksandr "Flone" Tretyakov (github.com/Flone-dnb).
// Licensed under the ZLib license.
// Refer to the LICENSE file included.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// UE
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"

// STL
#include <vector>

#include "FWGChunk.generated.h"

class FWGenChunkMap;

UCLASS()
class AFWGChunk : public AActor
{
	GENERATED_BODY()
	
public:

	AFWGChunk();

	void setInit(long long iX, long long iY, int32 iSectionIndex, bool bAroundCenter);
	void setChunkSize(int32 iXCount, int32 iYCount);
	void setChunkMap(FWGenChunkMap* pChunkMap);
	void setOverlapToActors(std::vector<FString> vClasses);
	void clearChunk();
	void setMeshSection(FProcMeshSection* pMeshSection);

	~AFWGChunk();


	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	FProcMeshSection* pMeshSection;

	UPROPERTY()
	UBoxComponent* pTriggerBox;


	TArray<FProcMeshTangent>  vTangents;
	TArray<FLinearColor>      vVertexColors;
	TArray<FVector>           vVertices;
	TArray<int32>             vTriangles;
	TArray<FVector>           vNormals;
	TArray<FVector2D>         vUV0;


	std::vector<int32>        vLayerIndex;
	std::vector<std::vector<bool>> vChunkCells;


	size_t                    iMaxZVertexIndex;


	long long                 iX;
	long long                 iY;


	int32                     iSectionIndex;

protected:

	virtual void BeginPlay() override;


	std::vector<FString>      vClassesToOverlap;

	FWGenChunkMap*            pChunkMap;

	bool                      bAroundCenter;
};

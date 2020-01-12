// This file is part of the FWorldGenenerator.
// Copyright Aleksandr "Flone" Tretyakov (github.com/Flone-dnb).
// Licensed under the ZLib license.
// Refer to the LICENSE file included.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include <vector>

#include "FWGen.generated.h"


class UStaticMeshComponent;

#if WITH_EDITOR
class UBoxComponent;
#endif // WITH_EDITOR


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

class FWGenChunkMap;
class FWGenChunk;


UCLASS()
class FWORLDGENERATOR_API AFWGen : public AActor
{
	GENERATED_BODY()
	
public:	
	AFWGen();

	~AFWGen();


	UFUNCTION(BlueprintCallable, Category = "Procedural Generation")
		void          GenerateWorld();

#if WITH_EDITOR
	virtual void  PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void  PostEditMove          (bool bFinished);
#endif // WITH_EDITOR



#if WITH_EDITOR
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
		bool  ComplexPreview = false;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ChunkPieceRowCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ChunkPieceColumnCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		float ChunkPieceSizeX = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		float ChunkPieceSizeY = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ViewDistance = 1;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		float GenerationFrequency = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationOctaves = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationMaxZFromActorZ = 15000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		bool  InvertWorld = false;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
		int32 WorldSize = 0;
	// 0   ---  infinite
	// -1  ---  only 1 chunk
	// 1   ---  only 1 ViewDistance size
	// 2   ---  only 2 ViewDistance sizes




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
		UMaterialInterface* GroundMaterial;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		bool  CreateWater = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float ZWaterLevelInWorld = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float WaterSize = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		UMaterialInterface* WaterMaterial;

protected:

	virtual void BeginPlay() override;



	// --------------------------------------------------------------



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
		USceneComponent* pRootNode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
		UStaticMeshComponent* WaterPlane;

#if WITH_EDITOR
	UPROPERTY()
		UBoxComponent* PreviewPlane;
#endif // WITH_EDITOR


private:

	void generateChunk(FWGenChunk* pChunk);
	void generateSeed();

#if WITH_EDITOR
	void refreshPreview();
#endif // WITH_EDITOR


	FWGenChunkMap*           pChunkMap;


	int32                    iGeneratedSeed;


	bool                     bWorldCreated;
};


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------



class FWGenChunk
{
public:
	FWGenChunk(UProceduralMeshComponent* pMeshComponent, long long iX, long long iY)
	{
		this->pMeshComponent  = pMeshComponent;

		this->iX = iX;
		this->iY = iY;
	}

	void clearChunk()
	{
		vVertices     .Empty();
		vTriangles    .Empty();
		vNormals      .Empty();
		vUV0          .Empty();
		vVertexColors .Empty();
		vTangents     .Empty();

		pMeshComponent->ClearAllMeshSections();
	}

	int32 getX() const
	{
		return iX;
	}

	int32 getY() const
	{
		return iY;
	}

	~FWGenChunk()
	{
		pMeshComponent->DestroyComponent();
	}


	UPROPERTY()
	UProceduralMeshComponent* pMeshComponent;


	TArray<FProcMeshTangent>  vTangents;
	TArray<FLinearColor>      vVertexColors;
	TArray<FVector>           vVertices;
	TArray<int32>             vTriangles;
	TArray<FVector>           vNormals;
	TArray<FVector2D>         vUV0;

private:


	long long                 iX;
	long long                 iY;
};


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------


class FWGenChunkMap
{
public:
	FWGenChunkMap(int32 ViewDistance, int32 WorldSize)
	{
		this->ViewDistance = ViewDistance;
		this->WorldSize    = WorldSize;
	}

	void addChunk(FWGenChunk* pChunk)
	{
		vChunks.push_back(pChunk);
	}

	FWGenChunk* getLastChunk() const
	{
		return vChunks.back();
	}

	~FWGenChunkMap()
	{
		for (size_t i = 0; i < vChunks.size(); i++)
		{
			delete vChunks[i];
		}
	}

private:

	int32 ViewDistance;
	// ViewDistance == 1  ---  Always loaded chunks: 3x3.
	// ViewDistance == 2  ---  Always loaded chunks: 5x5.
	// ViewDistance == 3  ---  Always loaded chunks: 7x7.
	// ViewDistance == 4  ---  Always loaded chunks: 9x9.
	// ... (The player is always in the central chunk) ...

	int32 WorldSize;


	std::vector<FWGenChunk*> vChunks;
};
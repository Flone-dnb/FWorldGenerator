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
		bool          GenerateWorld();

#if WITH_EDITOR
	virtual void  PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR



#if WITH_EDITOR
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
		bool  ComplexPreview = true;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ChunkPieceRowCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ChunkPieceColumnCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		float ChunkPieceSizeX = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		float ChunkPieceSizeY = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ViewDistance = 1;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		float GenerationFrequency = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationOctaves = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationSeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationMaxZFromActorZ = 15000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		bool  InvertWorld = false;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
		UMaterialInterface* GroundMaterial;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		bool  CreateWater = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float ZWaterLevelInWorld = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float WaterSize = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		UMaterialInterface* WaterMaterial;

protected:

	virtual void BeginPlay() override;



	// --------------------------------------------------------------



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
		USceneComponent* pRootNode;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
	//	UProceduralMeshComponent* pCentralMeshComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
		UStaticMeshComponent* WaterPlane;

#if WITH_EDITOR
	UPROPERTY()
		UBoxComponent* PreviewPlane;
#endif // WITH_EDITOR


private:

	void generateChunk(FWGenChunk* pChunk);

#if WITH_EDITOR
	void refreshPreview();
#endif // WITH_EDITOR


	FWGenChunkMap*           pChunkMap;
};


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------


enum EConnectionSide { CS_NONE, CS_LEFT, CS_RIGHT, CS_TOP, CS_BOTTOM };



class FWGenChunk
{
public:
	FWGenChunk(EConnectionSide eConnectionSide, UProceduralMeshComponent* pMeshComponent)
	{
		this->eConnectionSide = eConnectionSide;
		this->pMeshComponent = pMeshComponent;
	}

	~FWGenChunk()
	{
		pMeshComponent = nullptr;
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

	EConnectionSide           eConnectionSide;
};


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------


class FWGenChunkMap
{
public:
	FWGenChunkMap(FWGenChunk* pCentralChunk, int32 ViewDistance)
	{
		this->ViewDistance = ViewDistance;

		vChunks.push_back(pCentralChunk);
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


	std::vector<FWGenChunk*> vChunks;
};
// This file is part of the FWorldGenenerator.
// Copyright Aleksandr "Flone" Tretyakov (github.com/Flone-dnb).
// Licensed under the ZLib license.
// Refer to the LICENSE file included.

#pragma once

// UE
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"

// STL
#include <vector>
#include <random>
#include <thread>
#include <mutex>

#include "FWGen.generated.h"


class UStaticMeshComponent;


#define VERSION_SIZE 20
// Max size: 20 chars.
#define FWGEN_VERSION "FWG 1.0.1"

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

class FWGenChunkMap;
class AFWGChunk;
class FWGCallback;

UCLASS()
class FWORLDGENERATOR_API AFWGen : public AActor
{
	GENERATED_BODY()
	
public:	
	AFWGen();

	~AFWGen();

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		bool          BindFunctionToSpawn(UObject* FunctionOwner, FString FunctionName, float Layer,
			float ProbabilityToSpawn, bool IsBlocking);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		bool          BindFunctionToDespawnActors(UObject* FunctionOwner, FString FunctionName);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		void          UnBindFunctionToSpawn(FString FunctionName);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		void          AddOverlapToActorClass(UClass* OverlapToClass);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		void          RemoveOverlapToActorClass(UClass* OverlapToClass);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		FVector       GetFreeCellLocation(float Layer, bool SetBlocking = true);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		void          SaveWorldParamsToFile(FString PathToFile);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		void          LoadWorldParamsFromFile(FString PathToFile);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		void          GenerateWorld(int64 iCentralChunkX = 0, int64 iCentralChunkY = 0, AActor* Character = nullptr);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		int64     GetCentralChunkX();

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		int64     GetCentralChunkY();

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		int64     GetChunkXByLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "FWorldGenerator")
		int64     GetChunkYByLocation(FVector Location);


	// "Set" functions
		
		// Chunks

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Chunks")
			bool SetChunkPieceRowCount(int32 NewChunkPieceRowCount);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Chunks")
			bool SetChunkPieceColumnCount(int32 NewChunkPieceColumnCount);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Chunks")
			bool SetChunkPieceSizeX(float NewChunkPieceSizeX);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Chunks")
			bool SetChunkPieceSizeY(float NewChunkPieceSizeY);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Chunks")
			bool SetViewDistance(int32 NewViewDistance);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Chunks")
			void SetLoadUnloadChunkMaxZ(float NewLoadUnloadChunkMaxZ);


		// Generation

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Generation")
			bool SetGenerationFrequency(float NewGenerationFrequency);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Generation")
			bool SetGenerationOctaves(int32 NewGenerationOctaves);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Generation")
			bool SetGenerationSeed(int32 NewGenerationSeed);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Generation")
			bool SetGenerationMaxZFromActorZ(float NewGenerationMaxZFromActorZ);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Generation")
			void SetInvertWorld(bool InvertWorld);


		// World

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | World")
			bool SetWorldSize(int32 NewWorldSize);


		// Ground

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground")
			void SetGroundMaterial(UMaterialInterface* NewGroundMaterial);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground")
			bool SetFirstMaterialMaxRelativeHeight(float NewFirstMaterialMaxRelativeHeight);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground")
			bool SetSecondMaterialMaxRelativeHeight(float NewSecondMaterialMaxRelativeHeight);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground")
			bool SetMaterialHeightMaxDeviation(float NewMaterialHeightMaxDeviation);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground")
			bool SetTerrainCutHeightFromActorZ(float NewTerrainCutHeightFromActorZ);



		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground Material Blend")
			bool SetFirstMaterialOnOtherProbability(float FirstOnSecond, float FirstOnThird);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground Material Blend")
			bool SetSecondMaterialOnOtherProbability(float SecondOnFirst, float SecondOnThird);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground Material Blend")
			bool SetThirdMaterialOnOtherProbability(float ThirdOnFirst, float ThirdOnSecond);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Ground Material Blend")
			bool SetIncreasedMaterialBlendProbability(float NewIncreasedMaterialBlendProbability);


		// Water

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Water")
			void SetCreateWater(bool CreateWater);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Water")
			void SetSecondMaterialUnderWater(bool NewSecondMaterialUnderWater);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Water")
			bool SetZWaterLevelInWorld(float NewZWaterLevelInWorld);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Water")
			bool SetWaterSize(int32 NewWaterSize);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Water")
			void SetWaterMaterial(UMaterialInterface* NewWaterMaterial);



		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Additional Steps")
			void SetApplyGroundMaterialBlend(bool bApply);

		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Additional Steps")
			void SetApplySlopeDependentBlend(bool bApply);



		UFUNCTION(BlueprintCallable, Category = "FWorldGenerator | Slope Dependent Blend")
			bool SetMinSlopeHeightMultiplier(float NewMinSlopeHeightMultiplier);



		UFUNCTION(BlueprintCallable, Category = "Spawning Objects")
			bool SetDivideChunkXCount(int32 DivideChunkXCount);

		UFUNCTION(BlueprintCallable, Category = "Spawning Objects")
			bool SetDivideChunkYCount(int32 DivideChunkYCount);

		UFUNCTION(BlueprintCallable, Category = "Spawning Objects")
			bool SetMaxOffsetByX(float fMaxOffsetByX);

		UFUNCTION(BlueprintCallable, Category = "Spawning Objects")
			bool SetMaxOffsetByY(float fMaxOffsetByY);

		UFUNCTION(BlueprintCallable, Category = "Spawning Objects")
			bool SetMaxRotation(float fMaxRotation);

		UFUNCTION(BlueprintCallable, Category = "Spawning Objects")
			void SetMaxZDiffInCell(float fNewMaxZDiffInCell);


#if !UE_BUILD_SHIPPING
	virtual void  PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void  PostEditMove          (bool bFinished);
#endif // WITH_EDITOR


//#if !UE_BUILD_SHIPPING
#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
		bool  ComplexPreview = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
		bool  DrawChunkBounds = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save / Load")
		FString  FWGenVersion = FWGEN_VERSION;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save / Load")
		bool  ReadParamsFromFileRightNow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save / Load")
		bool  SaveParamsToFileRightNow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save / Load")
		FString  PathToSaveFile = L"";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save / Load")
		bool  LastSaveLoadOperationStatus = false;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ChunkPieceRowCount = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ChunkPieceColumnCount = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		float ChunkPieceSizeX = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		float ChunkPieceSizeY = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		int32 ViewDistance = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunks")
		float LoadUnloadChunkMaxZ = 200000.0f;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		float GenerationFrequency = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationOctaves = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 GenerationSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		float GenerationMaxZFromActorZ = 45000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		bool  InvertWorld = false;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
		int32 WorldSize = 0;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
		UMaterialInterface* GroundMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
		float FirstMaterialMaxRelativeHeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
		float SecondMaterialMaxRelativeHeight = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
		float MaterialHeightMaxDeviation = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
		float TerrainCutHeightFromActorZ = 1.0f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		bool  CreateWater = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		bool  SecondMaterialUnderWater = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		float ZWaterLevelInWorld = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		int32 WaterSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
		UMaterialInterface* WaterMaterial;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Additional Steps")
		bool ApplyGroundMaterialBlend = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Additional Steps")
		bool ApplySlopeDependentBlend = true;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Material Blend")
		float FirstMaterialOnSecondProbability = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Material Blend")
		float FirstMaterialOnThirdProbability  = 0.002f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Material Blend")
		float SecondMaterialOnFirstProbability = 0.008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Material Blend")
		float SecondMaterialOnThirdProbability = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Material Blend")
		float ThirdMaterialOnFirstProbability  = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Material Blend")
		float ThirdMaterialOnSecondProbability = 0.004f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Material Blend")
		float IncreasedMaterialBlendProbability = 0.16f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Dependent Blend")
		float MinSlopeHeightMultiplier = 0.006f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Objects")
		int32 DivideChunkXCount = 700;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Objects")
		int32 DivideChunkYCount = 700;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Objects")
		float MaxOffsetByX = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Objects")
		float MaxOffsetByY = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Objects")
		float MaxRotation = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning Objects")
		float MaxZDiffInCell = 45.0f;

protected:

	virtual void BeginPlay() override;



	// --------------------------------------------------------------



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
		USceneComponent* pRootNode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
		UStaticMeshComponent* WaterPlane;

#if WITH_EDITORONLY_DATA
//#if !UE_BUILD_SHIPPING
	UPROPERTY()
		UBoxComponent* PreviewPlane;
#endif // WITH_EDITOR


private:

	AFWGChunk*  generateChunk      (long long iX, long long iY, int32 iSectionIndex, bool bAroundCenter,
		long long iUnloadX = 0, long long iUnloadY = 0, bool bUnload = false);
	void  generateSeed             ();
	float pickVertexMaterial       (double height, std::uniform_real_distribution<float>* pUrd, std::mt19937_64* pRnd, float* pfLayerTypeWithoutRnd = nullptr);
	void  blendWorldMaterialsMore  (AFWGChunk* pOnlyForThisChunk = nullptr);
	void  applySlopeDependentBlend (AFWGChunk* pOnlyForThisChunk = nullptr);
	void  spawnObjects             (AFWGChunk* pOnlyForThisChunk = nullptr);
	void  createTriggerBoxForChunk (AFWGChunk* pChunk);

	bool  areEqual                 (float a, float b, float eps);
	void  compareHeightDifference  (AFWGChunk* pChunk, std::vector<bool>& vProcessedVertices, float& fCurrentZ, size_t iCompareToIndex, float& fSteepSlopeMinHeightDiff);

#if WITH_EDITORONLY_DATA
//#if !UE_BUILD_SHIPPING
	void refreshPreview();
#endif // WITH_EDITOR

	std::vector<FWGCallback>  vObjectsToSpawn;
	std::vector<FString>      vOverlapToClasses;


	UPROPERTY()
	UProceduralMeshComponent* pProcMeshComponent;


	UPROPERTY()
	UBoxComponent*            pBlockingVolumeX1;
	UPROPERTY()
	UBoxComponent*            pBlockingVolumeX2;
	UPROPERTY()
	UBoxComponent*            pBlockingVolumeY1;
	UPROPERTY()
	UBoxComponent*            pBlockingVolumeY2;


	int32                     iCurrentSectionIndex;


	FWGenChunkMap*            pChunkMap;
	FWGCallback*              pCallbackToDespawn;


	int32                     iGeneratedSeed;


	bool                      bWorldCreated;


	friend class FWGenChunkMap;
};

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

class FWGCallback
{
public:
	UObject*   pOwner;
	UFunction* pFunction;

	float      fLayer;
	float      fProbabilityToSpawn;
	FString    sFunctionName;

	bool       bIsBlocking;
};

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------


class FWGenChunkMap
{
public:

	FWGenChunkMap(AFWGen* pGen);

	void addChunk(AFWGChunk* pChunk);

	void clearWorld(UProceduralMeshComponent* pProcMeshComponent);

	void setCurrentChunk(AFWGChunk* pChunk);

	long long getCentralChunkX();
	long long getCentralChunkY();

	~FWGenChunkMap();

	std::vector<AFWGChunk*> vChunks;

private:

	void loadNewChunk(long long iLoadX, long long iLoadY, long long iUnloadX, long long iUnloadY);

	AFWGChunk* pCurrentChunk;
	AFWGen* pGen;

	std::mutex mtxLoadChunks;
};

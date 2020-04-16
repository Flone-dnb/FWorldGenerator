// This file is part of the FWorldGenenerator.
// Copyright Aleksandr "Flone" Tretyakov (github.com/Flone-dnb).
// Licensed under the ZLib license.
// Refer to the LICENSE file included.

#include "FWGen.h"

// UE
#include "Components/StaticMeshComponent.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>

// STL
#include <ctime>
#include <fstream>

// Custom
#include "FWGChunk.h"

// External
#include "PerlinNoise.hpp"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#include <EngineGlobals.h>
#endif // WITH_EDITOR

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------



AFWGen::AFWGen()
{
	PrimaryActorTick.bCanEverTick = false;
	bWorldCreated                 = false;


	iGeneratedSeed                = 0;



	// Mesh structure

	pRootNode = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = pRootNode;

	pProcMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("ProcMeshComp");
	//pProcMeshComponent->bUseAsyncCooking = true;  // commented because the world should be generated after GenerateWorld() returned, and not calculate something after the function returned.
	//pProcMeshComponent->RegisterComponent();
	pProcMeshComponent->SetupAttachment(RootComponent);

	// Enable collision
	pProcMeshComponent->ContainsPhysicsTriMeshData(true);



	pChunkMap = new FWGenChunkMap(this);
	pCallbackToDespawn = nullptr;




	// Water Plane

	WaterPlane = CreateDefaultSubobject<UStaticMeshComponent>("WaterPlane");
	WaterPlane->SetupAttachment(RootComponent);
	WaterPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	WaterPlane->SetStaticMesh(PlaneMeshAsset.Object);

	if (CreateWater == false) WaterPlane->SetVisibility(false);



	// Blocking volumes.
	pBlockingVolumeX1 = CreateDefaultSubobject<UBoxComponent>("Blocking Volume X1");
	pBlockingVolumeX1->SetupAttachment(RootComponent);
	pBlockingVolumeX1->BodyInstance.SetCollisionProfileName(TEXT("BlockAll"));

	pBlockingVolumeX2 = CreateDefaultSubobject<UBoxComponent>("Blocking Volume X2");
	pBlockingVolumeX2->SetupAttachment(RootComponent);
	pBlockingVolumeX2->BodyInstance.SetCollisionProfileName(TEXT("BlockAll"));

	pBlockingVolumeY1 = CreateDefaultSubobject<UBoxComponent>("Blocking Volume Y1");
	pBlockingVolumeY1->SetupAttachment(RootComponent);
	pBlockingVolumeY1->BodyInstance.SetCollisionProfileName(TEXT("BlockAll"));

	pBlockingVolumeY2 = CreateDefaultSubobject<UBoxComponent>("Blocking Volume Y2");
	pBlockingVolumeY2->SetupAttachment(RootComponent);
	pBlockingVolumeY2->BodyInstance.SetCollisionProfileName(TEXT("BlockAll"));



	// Materials

	WaterMaterial  = nullptr;
	GroundMaterial = nullptr;



	// Preview Plane

#if WITH_EDITOR
	PreviewPlane = CreateDefaultSubobject<UBoxComponent>("PreviewPlane");
	PreviewPlane->SetupAttachment(RootComponent);
	PreviewPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	refreshPreview();

	if (ComplexPreview) GenerateWorld();
#endif // WITH_EDITOR
}

AFWGen::~AFWGen()
{
	if (pChunkMap)
	{
		delete pChunkMap;
	}




	if (pProcMeshComponent->IsValidLowLevel())
	{
		if (pProcMeshComponent->IsPendingKill() == false)
		{
			pProcMeshComponent->DestroyComponent();
		}
	}



	if (pBlockingVolumeX1->IsValidLowLevel())
	{
		if (pBlockingVolumeX1->IsPendingKill() == false)
		{
			pBlockingVolumeX1->DestroyComponent();
		}
	}



	if (pBlockingVolumeX2->IsValidLowLevel())
	{
		if (pBlockingVolumeX2->IsPendingKill() == false)
		{
			pBlockingVolumeX2->DestroyComponent();
		}
	}



	if (pBlockingVolumeY1->IsValidLowLevel())
	{
		if (pBlockingVolumeY1->IsPendingKill() == false)
		{
			pBlockingVolumeY1->DestroyComponent();
		}
	}



	if (pBlockingVolumeY2->IsValidLowLevel())
	{
		if (pBlockingVolumeY2->IsPendingKill() == false)
		{
			pBlockingVolumeY2->DestroyComponent();
		}
	}


	if (pCallbackToDespawn)
	{
		delete pCallbackToDespawn;
	}
}

bool AFWGen::BindFunctionToSpawn(UObject* FunctionOwner, FString FunctionName, float Layer, float ProbabilityToSpawn, bool IsBlocking)
{
	FWGCallback callback;

	callback.pOwner = FunctionOwner;
	callback.fProbabilityToSpawn = ProbabilityToSpawn;
	callback.bIsBlocking = IsBlocking;
	callback.fLayer = Layer;

	for ( TFieldIterator<UFunction> FIT ( FunctionOwner->GetClass(), EFieldIteratorFlags::IncludeSuper ); FIT; ++FIT)
	{
		UFunction* Function = *FIT;
		if (Function->GetName() == FunctionName)
		{
			callback.pFunction = Function;

			vObjectsToSpawn.push_back(callback);

			return false;
		}
	}

	return true;
}

bool AFWGen::BindFunctionToDespawnActors(UObject* FunctionOwner, FString FunctionName)
{
	for ( TFieldIterator<UFunction> FIT ( FunctionOwner->GetClass(), EFieldIteratorFlags::IncludeSuper ); FIT; ++FIT)
	{
		UFunction* Function = *FIT;
		if (Function->GetName() == FunctionName)
		{
			if (pCallbackToDespawn == nullptr)
			{
				pCallbackToDespawn = new FWGCallback();
			}
			
			pCallbackToDespawn->pOwner = FunctionOwner;
			pCallbackToDespawn->sFunctionName = FunctionName;
			pCallbackToDespawn->pFunction = Function;

			return false;
		}
	}

	return true;
}

void AFWGen::UnBindFunctionToSpawn(FString FunctionName)
{
	for (size_t i = 0; i < vObjectsToSpawn.size(); i++)
	{
		if (vObjectsToSpawn[i].sFunctionName == FunctionName)
		{
			vObjectsToSpawn.erase( vObjectsToSpawn.begin() + i);
			break;
		}
	}
}

void AFWGen::AddOverlapToActorClass(UClass* OverlapToClass)
{
	vOverlapToClasses.push_back(OverlapToClass->GetName());
}

void AFWGen::RemoveOverlapToActorClass(UClass* OverlapToClass)
{
	for (size_t i = 0; i < vOverlapToClasses.size(); i++)
	{
		if (vOverlapToClasses[i] == OverlapToClass->GetName())
		{
			vOverlapToClasses.erase( vOverlapToClasses.begin() + i);
			break;
		}
	}
}

FVector AFWGen::GetFreeCellLocation(float Layer, bool SetBlocking)
{
	FVector location;

	for (size_t i = 0; i < pChunkMap->vChunks.size(); i++)
	{
		float fChunkX = GetActorLocation().X;
		float fChunkY = GetActorLocation().Y;

		if (pChunkMap->vChunks[i]->iX != 0)
		{
			fChunkX += (pChunkMap->vChunks[i]->iX * ChunkPieceColumnCount * ChunkPieceSizeX);
		}

		if (pChunkMap->vChunks[i]->iY != 0)
		{
			fChunkY += (pChunkMap->vChunks[i]->iY * ChunkPieceRowCount * ChunkPieceSizeY);
		}

		float fStartX = fChunkX - (ChunkPieceColumnCount * ChunkPieceSizeX) / 2;
		float fStartY = fChunkY - (ChunkPieceRowCount * ChunkPieceSizeY) / 2;

		float fXCellSize = ChunkPieceColumnCount * ChunkPieceSizeX / DivideChunkXCount;
		float fYCellSize = ChunkPieceRowCount    * ChunkPieceSizeY / DivideChunkYCount;


		for (size_t y = 0; y < pChunkMap->vChunks[i]->vChunkCells.size(); y++)
		{
			for (size_t x = 0; x < pChunkMap->vChunks[i]->vChunkCells[y].size(); x++)
			{
				if (pChunkMap->vChunks[i]->vChunkCells[y][x])
				{
					continue;
				}

				location.X = fStartX + x * fXCellSize + fXCellSize / 2;
				location.Y = fStartY + y * fYCellSize + fYCellSize / 2;
				location.Z = GetActorLocation().Z;

				FHitResult OutHit;
				FVector TraceStart(location.X, location.Y, GetActorLocation().Z + GenerationMaxZFromActorZ + 5.0f);
				FVector TraceEnd(location.X, location.Y, GetActorLocation().Z - 5.0f);
				FCollisionQueryParams CollisionParams;

				// Get Z.
				if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, CollisionParams))
				{
					if (OutHit.bBlockingHit)
					{
						location.Z = OutHit.ImpactPoint.Z;
					}
				}

				if ( CreateWater && (location.Z <= GetActorLocation().Z + GenerationMaxZFromActorZ * ZWaterLevelInWorld
					+ GenerationMaxZFromActorZ * 0.01f)) // error
				{
					// Water layer.

					if (areEqual(Layer, -0.5f, 0.1f))
					{
						if (SetBlocking)
						{
							pChunkMap->vChunks[i]->vChunkCells[y][x] = true;
						}

						return location;
					}
				}
				else if (location.Z <= GetActorLocation().Z + GenerationMaxZFromActorZ * FirstMaterialMaxRelativeHeight)
				{
					// First layer.

					if (areEqual(Layer, 0.0f, 0.1f))
					{
						if (SetBlocking)
						{
							pChunkMap->vChunks[i]->vChunkCells[y][x] = true;
						}

						return location;
					}
				}
				else if (location.Z <= GetActorLocation().Z + GenerationMaxZFromActorZ * SecondMaterialMaxRelativeHeight)
				{
					// Second layer.

					if (areEqual(Layer, 0.5f, 0.1f))
					{
						if (SetBlocking)
						{
							pChunkMap->vChunks[i]->vChunkCells[y][x] = true;
						}

						return location;
					}
				}
				else
				{
					// Third layer.

					if (areEqual(Layer, 1.0f, 0.1f))
					{
						if (SetBlocking)
						{
							pChunkMap->vChunks[i]->vChunkCells[y][x] = true;
						}

						return location;
					}
				}
			}
		}
	}

	location = FVector(0, 0, 0);
	return location;
}

void AFWGen::SaveWorldParamsToFile(FString PathToFile)
{
	PathToFile.Append(L".fwgs");

	std::ofstream saveFile(TCHAR_TO_UTF16(*PathToFile));

	// Version.
	std::string sVersion = FWGEN_VERSION;
	char vVersionBuffer[VERSION_SIZE];
	memset(vVersionBuffer, 0, VERSION_SIZE);
	std::memcpy(vVersionBuffer, sVersion.c_str(), sVersion.size());

	saveFile.write(vVersionBuffer, VERSION_SIZE);

	// Chunks.
	saveFile.write(reinterpret_cast<char*>(&ChunkPieceRowCount), sizeof(ChunkPieceRowCount));
	saveFile.write(reinterpret_cast<char*>(&ChunkPieceColumnCount), sizeof(ChunkPieceColumnCount));
	saveFile.write(reinterpret_cast<char*>(&ChunkPieceSizeX), sizeof(ChunkPieceSizeX));
	saveFile.write(reinterpret_cast<char*>(&ChunkPieceSizeY), sizeof(ChunkPieceSizeY));
	saveFile.write(reinterpret_cast<char*>(&ViewDistance), sizeof(ViewDistance));
	saveFile.write(reinterpret_cast<char*>(&LoadUnloadChunkMaxZ), sizeof(LoadUnloadChunkMaxZ));

	// Generation.
	saveFile.write(reinterpret_cast<char*>(&GenerationFrequency), sizeof(GenerationFrequency));
	saveFile.write(reinterpret_cast<char*>(&GenerationOctaves), sizeof(GenerationOctaves));
	saveFile.write(reinterpret_cast<char*>(&GenerationSeed), sizeof(GenerationSeed));
	saveFile.write(reinterpret_cast<char*>(&GenerationMaxZFromActorZ), sizeof(GenerationMaxZFromActorZ));
	saveFile.write(reinterpret_cast<char*>(&InvertWorld), sizeof(InvertWorld));

	// World.
	saveFile.write(reinterpret_cast<char*>(&WorldSize), sizeof(WorldSize));

	// Ground.
	saveFile.write(reinterpret_cast<char*>(&FirstMaterialMaxRelativeHeight), sizeof(FirstMaterialMaxRelativeHeight));
	saveFile.write(reinterpret_cast<char*>(&SecondMaterialMaxRelativeHeight), sizeof(SecondMaterialMaxRelativeHeight));
	saveFile.write(reinterpret_cast<char*>(&MaterialHeightMaxDeviation), sizeof(MaterialHeightMaxDeviation));
	saveFile.write(reinterpret_cast<char*>(&TerrainCutHeightFromActorZ), sizeof(TerrainCutHeightFromActorZ));

	// Water.
	saveFile.write(reinterpret_cast<char*>(&CreateWater), sizeof(CreateWater));
	saveFile.write(reinterpret_cast<char*>(&SecondMaterialUnderWater), sizeof(SecondMaterialUnderWater));
	saveFile.write(reinterpret_cast<char*>(&ZWaterLevelInWorld), sizeof(ZWaterLevelInWorld));
	saveFile.write(reinterpret_cast<char*>(&WaterSize), sizeof(WaterSize));

	// Additional Steps.
	saveFile.write(reinterpret_cast<char*>(&ApplyGroundMaterialBlend), sizeof(ApplyGroundMaterialBlend));
	saveFile.write(reinterpret_cast<char*>(&ApplySlopeDependentBlend), sizeof(ApplySlopeDependentBlend));

	// Ground Material Blend.
	saveFile.write(reinterpret_cast<char*>(&FirstMaterialOnSecondProbability), sizeof(FirstMaterialOnSecondProbability));
	saveFile.write(reinterpret_cast<char*>(&FirstMaterialOnThirdProbability), sizeof(FirstMaterialOnThirdProbability));
	saveFile.write(reinterpret_cast<char*>(&SecondMaterialOnFirstProbability), sizeof(SecondMaterialOnFirstProbability));
	saveFile.write(reinterpret_cast<char*>(&SecondMaterialOnThirdProbability), sizeof(SecondMaterialOnThirdProbability));
	saveFile.write(reinterpret_cast<char*>(&ThirdMaterialOnFirstProbability), sizeof(ThirdMaterialOnFirstProbability));
	saveFile.write(reinterpret_cast<char*>(&ThirdMaterialOnSecondProbability), sizeof(ThirdMaterialOnSecondProbability));
	saveFile.write(reinterpret_cast<char*>(&IncreasedMaterialBlendProbability), sizeof(IncreasedMaterialBlendProbability));

	// Slope Dependent Blend.
	saveFile.write(reinterpret_cast<char*>(&MinSlopeHeightMultiplier), sizeof(MinSlopeHeightMultiplier));

	// Spawning Objects.
	saveFile.write(reinterpret_cast<char*>(&DivideChunkXCount), sizeof(DivideChunkXCount));
	saveFile.write(reinterpret_cast<char*>(&DivideChunkYCount), sizeof(DivideChunkYCount));
	saveFile.write(reinterpret_cast<char*>(&MaxOffsetByX), sizeof(MaxOffsetByX));
	saveFile.write(reinterpret_cast<char*>(&MaxOffsetByY), sizeof(MaxOffsetByY));
	saveFile.write(reinterpret_cast<char*>(&MaxRotation), sizeof(MaxRotation));
	saveFile.write(reinterpret_cast<char*>(&MaxZDiffInCell), sizeof(MaxZDiffInCell));

	saveFile.close();

	LastSaveLoadOperationStatus = true;
}

void AFWGen::LoadWorldParamsFromFile(FString PathToFile)
{
	std::ifstream readFile(TCHAR_TO_UTF16(*PathToFile));

	if (readFile.is_open() == false)
	{
		LastSaveLoadOperationStatus = false;
		return;
	}

	// Version.
	char vVersionBuffer[VERSION_SIZE];
	memset(vVersionBuffer, 0, VERSION_SIZE);

	readFile.read(vVersionBuffer, VERSION_SIZE);


	std::string sVersion(vVersionBuffer);

	if (sVersion.substr(0, 3) != "FWG")
	{
		LastSaveLoadOperationStatus = false;
		return;
	}
	

	// Chunks.
	readFile.read(reinterpret_cast<char*>(&ChunkPieceRowCount), sizeof(ChunkPieceRowCount));
	readFile.read(reinterpret_cast<char*>(&ChunkPieceColumnCount), sizeof(ChunkPieceColumnCount));
	readFile.read(reinterpret_cast<char*>(&ChunkPieceSizeX), sizeof(ChunkPieceSizeX));
	readFile.read(reinterpret_cast<char*>(&ChunkPieceSizeY), sizeof(ChunkPieceSizeY));
	readFile.read(reinterpret_cast<char*>(&ViewDistance), sizeof(ViewDistance));
	readFile.read(reinterpret_cast<char*>(&LoadUnloadChunkMaxZ), sizeof(LoadUnloadChunkMaxZ));

	// Generation.
	readFile.read(reinterpret_cast<char*>(&GenerationFrequency), sizeof(GenerationFrequency));
	readFile.read(reinterpret_cast<char*>(&GenerationOctaves), sizeof(GenerationOctaves));
	readFile.read(reinterpret_cast<char*>(&GenerationSeed), sizeof(GenerationSeed));
	readFile.read(reinterpret_cast<char*>(&GenerationMaxZFromActorZ), sizeof(GenerationMaxZFromActorZ));
	readFile.read(reinterpret_cast<char*>(&InvertWorld), sizeof(InvertWorld));

	// World.
	readFile.read(reinterpret_cast<char*>(&WorldSize), sizeof(WorldSize));

	// Ground.
	readFile.read(reinterpret_cast<char*>(&FirstMaterialMaxRelativeHeight), sizeof(FirstMaterialMaxRelativeHeight));
	readFile.read(reinterpret_cast<char*>(&SecondMaterialMaxRelativeHeight), sizeof(SecondMaterialMaxRelativeHeight));
	readFile.read(reinterpret_cast<char*>(&MaterialHeightMaxDeviation), sizeof(MaterialHeightMaxDeviation));
	readFile.read(reinterpret_cast<char*>(&TerrainCutHeightFromActorZ), sizeof(TerrainCutHeightFromActorZ));

	// Water.
	readFile.read(reinterpret_cast<char*>(&CreateWater), sizeof(CreateWater));
	readFile.read(reinterpret_cast<char*>(&SecondMaterialUnderWater), sizeof(SecondMaterialUnderWater));
	readFile.read(reinterpret_cast<char*>(&ZWaterLevelInWorld), sizeof(ZWaterLevelInWorld));
	readFile.read(reinterpret_cast<char*>(&WaterSize), sizeof(WaterSize));

	// Additional Steps.
	readFile.read(reinterpret_cast<char*>(&ApplyGroundMaterialBlend), sizeof(ApplyGroundMaterialBlend));
	readFile.read(reinterpret_cast<char*>(&ApplySlopeDependentBlend), sizeof(ApplySlopeDependentBlend));

	// Ground Material Blend.
	readFile.read(reinterpret_cast<char*>(&FirstMaterialOnSecondProbability), sizeof(FirstMaterialOnSecondProbability));
	readFile.read(reinterpret_cast<char*>(&FirstMaterialOnThirdProbability), sizeof(FirstMaterialOnThirdProbability));
	readFile.read(reinterpret_cast<char*>(&SecondMaterialOnFirstProbability), sizeof(SecondMaterialOnFirstProbability));
	readFile.read(reinterpret_cast<char*>(&SecondMaterialOnThirdProbability), sizeof(SecondMaterialOnThirdProbability));
	readFile.read(reinterpret_cast<char*>(&ThirdMaterialOnFirstProbability), sizeof(ThirdMaterialOnFirstProbability));
	readFile.read(reinterpret_cast<char*>(&ThirdMaterialOnSecondProbability), sizeof(ThirdMaterialOnSecondProbability));
	readFile.read(reinterpret_cast<char*>(&IncreasedMaterialBlendProbability), sizeof(IncreasedMaterialBlendProbability));

	// Slope Dependent Blend.
	readFile.read(reinterpret_cast<char*>(&MinSlopeHeightMultiplier), sizeof(MinSlopeHeightMultiplier));

	// Spawning Objects.
	readFile.read(reinterpret_cast<char*>(&DivideChunkXCount), sizeof(DivideChunkXCount));
	readFile.read(reinterpret_cast<char*>(&DivideChunkYCount), sizeof(DivideChunkYCount));
	readFile.read(reinterpret_cast<char*>(&MaxOffsetByX), sizeof(MaxOffsetByX));
	readFile.read(reinterpret_cast<char*>(&MaxOffsetByY), sizeof(MaxOffsetByY));
	readFile.read(reinterpret_cast<char*>(&MaxRotation), sizeof(MaxRotation));
	readFile.read(reinterpret_cast<char*>(&MaxZDiffInCell), sizeof(MaxZDiffInCell));

	readFile.close();

	LastSaveLoadOperationStatus = true;
}

void AFWGen::GenerateWorld(int64 iCentralChunkX, int64 iCentralChunkY, AActor* pCharacter)
{
	if (pChunkMap)
	{
		pChunkMap->clearWorld(pProcMeshComponent);
	}


	generateSeed();

	if (WorldSize != -1)
	{
		size_t iChunkCount = (ViewDistance * 2 + 1) * (ViewDistance * 2 + 1);


		// Generate the chunks.

		int32 iSectionIndex = 0;

		
		for (long long y = iCentralChunkY + ViewDistance; y > iCentralChunkY - ViewDistance - 1; y--)
		{
			for (long long x = iCentralChunkX - ViewDistance; x < iCentralChunkX + ViewDistance + 1; x++)
			{
				bool bAroundCenter = false;

				if ((x <= iCentralChunkX + 1) && (x >= iCentralChunkX - 1) && (y <= iCentralChunkY + 1) && (y >= iCentralChunkY - 1))
				{
					bAroundCenter = true;
				}

				AFWGChunk* pNewChunk = generateChunk(x, y, iSectionIndex, bAroundCenter);

				pChunkMap->addChunk(pNewChunk);

				if (x == iCentralChunkX && y == iCentralChunkY)
				{
					pChunkMap->setCurrentChunk(pNewChunk);
				}

				iSectionIndex++;
			}
		}

		iCurrentSectionIndex = iSectionIndex;
	}
	else
	{
		AFWGChunk* pNewChunk = generateChunk(iCentralChunkX, iCentralChunkY, 0, false);

		pChunkMap->addChunk(pNewChunk);

		iCurrentSectionIndex = 1;
	}

	if (ApplyGroundMaterialBlend)
	{
		blendWorldMaterialsMore();
	}
	
	if (ApplySlopeDependentBlend)
	{
		applySlopeDependentBlend();
	}

	// Update mesh.
	for (size_t i = 0; i < pChunkMap->vChunks.size(); i++)
	{
		pProcMeshComponent->UpdateMeshSection_LinearColor(i, pChunkMap->vChunks[i]->vVertices, pChunkMap->vChunks[i]->vNormals,
			pChunkMap->vChunks[i]->vUV0, pChunkMap->vChunks[i]->vVertexColors, pChunkMap->vChunks[i]->vTangents);
	}



	// Water Plane

	WaterPlane->SetWorldLocation(FVector(
		GetActorLocation().X,
		GetActorLocation().Y,
		GetActorLocation().Z + (GenerationMaxZFromActorZ * ZWaterLevelInWorld)
	));

	WaterPlane->SetWorldScale3D(FVector(
		(ChunkPieceColumnCount*ChunkPieceSizeX) * (WaterSize / 100.0f),
		(ChunkPieceRowCount*ChunkPieceSizeY) * (WaterSize / 100.0f),
		0.1f
	));

	if (WaterMaterial)
	{
		WaterPlane->SetMaterial(0, WaterMaterial);
	}

	if (CreateWater)
	{
		WaterPlane->SetVisibility(true);
	}
	else
	{
		WaterPlane->SetVisibility(false);
	}


	spawnObjects();

	if (iCentralChunkX != 0 || iCentralChunkY != 0)
	{
		float fChunkX = GetActorLocation().X;
		float fChunkY = GetActorLocation().Y;

		if (GetCentralChunkX() != 0)
		{
			// Left or right chunk

			fChunkX += (GetCentralChunkX() * ChunkPieceColumnCount * ChunkPieceSizeX);
		}

		if (GetCentralChunkY() != 0)
		{
			// Top or bottom chunk

			fChunkY += (GetCentralChunkY() * ChunkPieceRowCount * ChunkPieceSizeY);
		}

		fChunkX += ChunkPieceColumnCount * ChunkPieceSizeX / 2;
		fChunkY += ChunkPieceRowCount * ChunkPieceSizeY / 2;

		pCharacter->SetActorLocation(FVector(fChunkX, fChunkY, pCharacter->GetActorLocation().Z));
	}
	
	if (WorldSize != -1)
	{
		// Create triggers.

#if WITH_EDITOR
		FlushPersistentDebugLines(GetWorld());
#endif // WITH_EDITOR

		for (size_t i = 0; i < pChunkMap->vChunks.size(); i++)
		{
			float fTriggerX = GetActorLocation().X;
			float fTriggerY = GetActorLocation().Y;
			float fTriggerZ = GetActorLocation().Z + LoadUnloadChunkMaxZ / 2;

			if (pChunkMap->vChunks[i]->iX != 0)
			{
				// Left or right chunk

				fTriggerX += (pChunkMap->vChunks[i]->iX * ChunkPieceColumnCount * ChunkPieceSizeX);
			}

			if (pChunkMap->vChunks[i]->iY != 0)
			{
				// Top or bottom chunk

				fTriggerY += (pChunkMap->vChunks[i]->iY * ChunkPieceRowCount * ChunkPieceSizeY);
			}

			createTriggerBoxForChunk(pChunkMap->vChunks[i]);

#if WITH_EDITOR
			if (DrawChunkBounds)
			{
				DrawDebugBox(GetWorld(), FVector(fTriggerX, fTriggerY, fTriggerZ), FVector (
					(ChunkPieceColumnCount * ChunkPieceSizeX / 2),
					(ChunkPieceRowCount * ChunkPieceSizeY / 2),
					LoadUnloadChunkMaxZ / 2), 
					FColor::Red, true);
			}
#endif // WITH_EDITOR
		}

		float fChunkXSize = ChunkPieceColumnCount * ChunkPieceSizeX;
		float fChunkYSize = ChunkPieceRowCount * ChunkPieceSizeY;

		pBlockingVolumeX1->SetWorldLocation(FVector(GetActorLocation().X - (ViewDistance + 1) * fChunkXSize,
			GetActorLocation().Y,
			GetActorLocation().Z + LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeX1->SetBoxExtent(FVector(ChunkPieceColumnCount * ChunkPieceSizeX / 2,
			(ChunkPieceRowCount * ChunkPieceSizeY / 2) * (ViewDistance * 3),
			LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeX1->SetGenerateOverlapEvents(true);



		pBlockingVolumeX2->SetWorldLocation(FVector(GetActorLocation().X + (ViewDistance + 1) * fChunkXSize,
			GetActorLocation().Y,
			GetActorLocation().Z + LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeX2->SetBoxExtent(FVector(ChunkPieceColumnCount * ChunkPieceSizeX / 2,
			(ChunkPieceRowCount * ChunkPieceSizeY / 2) * (ViewDistance * 3),
			LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeX2->SetGenerateOverlapEvents(true);



		pBlockingVolumeY1->SetWorldLocation(FVector(GetActorLocation().X,
			GetActorLocation().Y - (ViewDistance + 1) * fChunkYSize,
			GetActorLocation().Z + LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeY1->SetBoxExtent(FVector((ChunkPieceColumnCount * ChunkPieceSizeX / 2) * (ViewDistance * 3),
			ChunkPieceRowCount * ChunkPieceSizeY / 2,
			LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeY1->SetGenerateOverlapEvents(true);



		pBlockingVolumeY2->SetWorldLocation(FVector(GetActorLocation().X,
			GetActorLocation().Y + (ViewDistance + 1) * fChunkYSize,
			GetActorLocation().Z + LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeY2->SetBoxExtent(FVector((ChunkPieceColumnCount * ChunkPieceSizeX / 2) * (ViewDistance * 3),
			ChunkPieceRowCount * ChunkPieceSizeY / 2,
			LoadUnloadChunkMaxZ / 2));

		pBlockingVolumeY2->SetGenerateOverlapEvents(true);
	}
}

long long AFWGen::GetCentralChunkX()
{
	return pChunkMap->getCentralChunkX();
}

long long AFWGen::GetCentralChunkY()
{
	return pChunkMap->getCentralChunkY();
}

void AFWGen::blendWorldMaterialsMore(AFWGChunk* pOnlyForThisChunk)
{
	std::mt19937_64 gen(std::random_device{}());
	std::uniform_real_distribution<float> urd(0.0f, 1.0f);

	std::vector<AFWGChunk*> vChunkToProcess;

	if (pOnlyForThisChunk)
	{
		vChunkToProcess.push_back(pOnlyForThisChunk);
	}
	else
	{
		for (size_t i = 0; i < pChunkMap->vChunks.size(); i++)
		{
			vChunkToProcess.push_back(pChunkMap->vChunks[i]);
		}
	}


	for (size_t i = 0; i < vChunkToProcess.size(); i++)
	{
		size_t iVertexIndex = 0;

		for (size_t iRow = 0; iRow < ChunkPieceRowCount + 1; iRow++)
		{
			iVertexIndex = (ChunkPieceColumnCount + 1) * iRow;

			size_t iSkipPoints = 0;

			for (size_t iColumn = 0; iColumn < ChunkPieceColumnCount + 1; iColumn++)
			{
				if (iSkipPoints > 0)
				{
					iSkipPoints--;
					iVertexIndex++;

					continue;
				}

				if ((iRow >= 3) && ((iRow + 3) < (ChunkPieceRowCount + 1))
					&& (iColumn >= 3) && ((iColumn + 3) < (ChunkPieceColumnCount + 1)))
				{
					if (SecondMaterialUnderWater &&
						(vChunkToProcess[i]->vVertices[iVertexIndex].Z <= (GetActorLocation().Z + (GenerationMaxZFromActorZ * (ZWaterLevelInWorld + 0.008f)))))
					{
						// Under water material - don't touch.
					}
					else
					{
						bool bFirstLayerWithOtherColor  = false;
						bool bSecondLayerWithOtherColor = false;
						bool bThirdLayerWithOtherColor  = false;

						if ((vChunkToProcess[i]->vLayerIndex[iVertexIndex] == 1)
							&& (areEqual(vChunkToProcess[i]->vVertexColors[iVertexIndex].A, 0.0f, 0.1f) == false))
						{
							bFirstLayerWithOtherColor = true;
						}
						else if ((vChunkToProcess[i]->vLayerIndex[iVertexIndex] == 2)
							&& (areEqual(vChunkToProcess[i]->vVertexColors[iVertexIndex].A, 0.5f, 0.1f) == false))
						{
							bSecondLayerWithOtherColor = true;
						}
						else if ((vChunkToProcess[i]->vLayerIndex[iVertexIndex] == 3)
							&& (areEqual(vChunkToProcess[i]->vVertexColors[iVertexIndex].A, 1.0f, 0.1f) == false))
						{
							bThirdLayerWithOtherColor = true;
						}


						if (bFirstLayerWithOtherColor || bSecondLayerWithOtherColor || bThirdLayerWithOtherColor)
						{
							// Add a little more of other color.

							// . . . . .
							// . . . . .
							// . . + . .  <- we are in the center.
							// . . . . .
							// . . . . .

							// Close square:

							for (int32 iIndexY = -(ChunkPieceColumnCount + 1); iIndexY <= (ChunkPieceColumnCount + 1); iIndexY += (ChunkPieceColumnCount + 1))
							{
								for (int32 iIndexX = -1; iIndexX <= 1; iIndexX++)
								{
									if (urd(gen) <= IncreasedMaterialBlendProbability)
									{
										vChunkToProcess[i]->vVertexColors[iVertexIndex + iIndexY + iIndexX] = vChunkToProcess[i]->vVertexColors[iVertexIndex];
									}
								}
							}

							// Far square:

							for (int32 iIndexY = -((ChunkPieceColumnCount + 1) * 2 - 1); iIndexY <= ((ChunkPieceColumnCount + 1) * 2 - 1); iIndexY += (ChunkPieceColumnCount + 1))
							{
								for (int32 iIndexX = -2; iIndexX <= 2; iIndexX++)
								{
									if (urd(gen) <= (IncreasedMaterialBlendProbability / 2))
									{
										vChunkToProcess[i]->vVertexColors[iVertexIndex + iIndexY + iIndexX] = vChunkToProcess[i]->vVertexColors[iVertexIndex];
									}
								}
							}

							iSkipPoints = 2;
						}
					}
				}
				
				iVertexIndex++;
			}
		}
	}
}

void AFWGen::applySlopeDependentBlend(AFWGChunk* pOnlyForThisChunk)
{
	float fSteepSlopeMinHeightDiff = GenerationMaxZFromActorZ * MinSlopeHeightMultiplier;

	std::vector<AFWGChunk*> vChunksToProcess;

	if (pOnlyForThisChunk)
	{
		vChunksToProcess.push_back(pOnlyForThisChunk);
	}
	else
	{
		for (size_t iChunk = 0; iChunk < pChunkMap->vChunks.size(); iChunk++)
		{
			vChunksToProcess.push_back(pChunkMap->vChunks[iChunk]);
		}
	}

	for (size_t iChunk = 0; iChunk < vChunksToProcess.size(); iChunk++)
	{
		std::vector<bool> vProcessedVertices(vChunksToProcess[iChunk]->vVertices.Num());
		vProcessedVertices[0] = true;

		size_t iVertexIndex = 0;

		for (size_t iRow = 0; iRow < ChunkPieceRowCount + 1; iRow++)
		{
			for (size_t iColumn = 0; iColumn < ChunkPieceColumnCount + 1; iColumn++)
			{
				if (((iRow < 2) || (iRow > ChunkPieceRowCount - 3))
					|| ((iColumn < 2) || (iColumn > ChunkPieceColumnCount - 3)))
				{
					iVertexIndex++;
					continue;
				}

				bool bHasLeftPoints  = true;
				bool bHasTopPoints   = true;
				bool bHasRightPoints = true;
				bool bHasDownPoints  = true;

				if (iRow == 0)
				{
					bHasTopPoints = false;
				}

				if (iRow == ChunkPieceRowCount)
				{
					bHasDownPoints = false;
				}

				if (iColumn == 0)
				{
					bHasLeftPoints = false;
				}

				if (iColumn == ChunkPieceColumnCount)
				{
					bHasRightPoints = false;
				}

				float fCurrentVertexZ = vChunksToProcess[iChunk]->vVertices[iVertexIndex].Z;

				// Process the left points:
				if (bHasLeftPoints)
				{
					compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex - 1, fSteepSlopeMinHeightDiff);

					if (bHasTopPoints)
					{
						compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex - 1 - (ChunkPieceColumnCount + 1), fSteepSlopeMinHeightDiff);
					}

					if (bHasDownPoints)
					{
						compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex - 1 + (ChunkPieceColumnCount + 1), fSteepSlopeMinHeightDiff);
					}
				}

				// Process the right points:
				if (bHasRightPoints)
				{
				compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex + 1, fSteepSlopeMinHeightDiff);

				if (bHasTopPoints)
				{
					compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex + 1 - (ChunkPieceColumnCount + 1), fSteepSlopeMinHeightDiff);
				}

				if (bHasDownPoints)
				{
					compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex + 1 + (ChunkPieceColumnCount + 1), fSteepSlopeMinHeightDiff);
				}
				}

				// Top point:
				if (bHasTopPoints)
				{
					compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex - (ChunkPieceColumnCount + 1), fSteepSlopeMinHeightDiff);
				}

				// Down point:
				if (bHasDownPoints)
				{
					compareHeightDifference(vChunksToProcess[iChunk], vProcessedVertices, fCurrentVertexZ, iVertexIndex + (ChunkPieceColumnCount + 1), fSteepSlopeMinHeightDiff);
				}

				iVertexIndex++;
			}
		}
	}
}

void AFWGen::spawnObjects(AFWGChunk* pOnlyForThisChunk)
{
	if (vObjectsToSpawn.size() == 0)
	{
		return;
	}

	std::mt19937_64 gen(std::random_device{}());

	std::uniform_real_distribution<float> urd(0.0f, 1.0f);
	std::uniform_real_distribution<float> urd_rotation(-MaxRotation, MaxRotation);

	std::sort(vObjectsToSpawn.begin(), vObjectsToSpawn.end(),
		[](const FWGCallback& a, const FWGCallback& b) -> bool
		{
			return a.fProbabilityToSpawn < b.fProbabilityToSpawn;
		});

	std::vector<AFWGChunk*> vChunksToProcess;

	if (pOnlyForThisChunk)
	{
		vChunksToProcess.push_back(pOnlyForThisChunk);
	}
	else
	{
		for (size_t i = 0; i < pChunkMap->vChunks.size(); i++)
		{
			vChunksToProcess.push_back(pChunkMap->vChunks[i]);
		}
	}

	for (size_t i = 0; i < vChunksToProcess.size(); i++)
	{
		float fChunkX = GetActorLocation().X;
		float fChunkY = GetActorLocation().Y;

		if (vChunksToProcess[i]->iX != 0)
		{
			fChunkX += (vChunksToProcess[i]->iX * ChunkPieceColumnCount * ChunkPieceSizeX);
		}

		if (vChunksToProcess[i]->iY != 0)
		{
			fChunkY += (vChunksToProcess[i]->iY * ChunkPieceRowCount * ChunkPieceSizeY);
		}

		float fStartX = fChunkX - (ChunkPieceColumnCount * ChunkPieceSizeX) / 2;
		float fStartY = fChunkY - (ChunkPieceRowCount * ChunkPieceSizeY) / 2;

		float fXCellSize = ChunkPieceColumnCount * ChunkPieceSizeX / DivideChunkXCount;
		float fYCellSize = ChunkPieceRowCount    * ChunkPieceSizeY / DivideChunkYCount;


		std::uniform_real_distribution<float> offsetByX(-fXCellSize * MaxOffsetByX, fXCellSize * MaxOffsetByX);
		std::uniform_real_distribution<float> offsetByY(-fYCellSize * MaxOffsetByY, fYCellSize * MaxOffsetByY);


		// Divide all object into 4 layers.

		std::vector<FWGCallback> vWaterLayer;
		std::vector<FWGCallback> vFirstLayer;
		std::vector<FWGCallback> vSecondLayer;
		std::vector<FWGCallback> vThirdLayer;

		for (size_t i = 0; i < vObjectsToSpawn.size(); i++)
		{
			if (areEqual(vObjectsToSpawn[i].fLayer, -0.5f, 0.1f))
			{
				vWaterLayer.push_back(vObjectsToSpawn[i]);
			}
			else if (areEqual(vObjectsToSpawn[i].fLayer, 0.0f, 0.1f))
			{
				vFirstLayer.push_back(vObjectsToSpawn[i]);
			}
			else if (areEqual(vObjectsToSpawn[i].fLayer, 0.5f, 0.1f))
			{
				vSecondLayer.push_back(vObjectsToSpawn[i]);
			}
			else
			{
				vThirdLayer.push_back(vObjectsToSpawn[i]);
			}
		}


		for (size_t y = 0; y < vChunksToProcess[i]->vChunkCells.size(); y++)
		{
			for (size_t x = 0; x < vChunksToProcess[i]->vChunkCells[y].size(); x++)
			{
				FVector location;
				location.X = fStartX + x * fXCellSize + fXCellSize / 2;
				location.Y = fStartY + y * fYCellSize + fYCellSize / 2;
				location.Z = GetActorLocation().Z;


				location.X += offsetByX(gen);
				location.Y += offsetByY(gen);


				FHitResult OutHit;
				FVector TraceStart(location.X, location.Y, GetActorLocation().Z + GenerationMaxZFromActorZ + 5.0f);
				FVector TraceEnd(location.X, location.Y, GetActorLocation().Z - 5.0f);
				FCollisionQueryParams CollisionParams;

				// Get Z.
				if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, CollisionParams))
				{
					if (OutHit.bBlockingHit)
					{
						location.Z = OutHit.ImpactPoint.Z;
					}
				}



				// Check if this cell is on the steep slope.

				bool bSteepSlope = false;

				std::vector<float> vXOffset;
				vXOffset.push_back(TraceStart.X + fXCellSize / 2);
				vXOffset.push_back(TraceStart.X - fXCellSize / 2);
				

				for (size_t i = 0; i < vXOffset.size(); i++)
				{
					TraceStart.X = vXOffset[i];
					TraceEnd.X   = vXOffset[i];

					if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, CollisionParams))
					{
						if (OutHit.bBlockingHit)
						{
							if (fabs(OutHit.ImpactPoint.Z - location.Z) > MaxZDiffInCell)
							{
								bSteepSlope = true;
								break;
							}
						}
					}
				}

				if (bSteepSlope == false)
				{
					std::vector<float> vYOffset;
					vYOffset.push_back(TraceStart.Y + fYCellSize / 2);
					vYOffset.push_back(TraceStart.Y - fYCellSize / 2);

					for (size_t i = 0; i < vYOffset.size(); i++)
					{
						TraceStart.Y = vYOffset[i];
						TraceEnd.Y   = vYOffset[i];

						if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, CollisionParams))
						{
							if (OutHit.bBlockingHit)
							{
								if (fabs(OutHit.ImpactPoint.Z - location.Z) > MaxZDiffInCell)
								{
									bSteepSlope = true;
									break;
								}
							}
						}
					}
				}

				if (bSteepSlope)
				{
					vChunksToProcess[i]->vChunkCells[y][x] = true;

					continue;
				}


				std::vector<FWGCallback>* pCurrentLayer = nullptr;


				if ( CreateWater && (location.Z <= GetActorLocation().Z + GenerationMaxZFromActorZ * ZWaterLevelInWorld
					 + GenerationMaxZFromActorZ * 0.01f)) // error
				{
					// Water layer.

					pCurrentLayer = &vWaterLayer;
				}
				else if (location.Z <= GetActorLocation().Z + GenerationMaxZFromActorZ * FirstMaterialMaxRelativeHeight)
				{
					// First layer.

					pCurrentLayer = &vFirstLayer;
				}
				else if (location.Z <= GetActorLocation().Z + GenerationMaxZFromActorZ * SecondMaterialMaxRelativeHeight)
				{
					// Second layer.

					pCurrentLayer = &vSecondLayer;
				}
				else
				{
					// Third layer.

					pCurrentLayer = &vThirdLayer;
				}


				float fGeneratedProbForThisCell = urd(gen);
				float fFullProb = 0.0f;

				for (size_t k = 0; k < pCurrentLayer->size(); k++)
				{
					float fNextValue = 1.0f - fFullProb;
					if (k != pCurrentLayer->size() - 1)
					{
						fNextValue = pCurrentLayer->operator[](k).fProbabilityToSpawn;
					}

					if ((fGeneratedProbForThisCell > fFullProb) && (fGeneratedProbForThisCell <= fFullProb + fNextValue) )
					{
						if (pCurrentLayer->operator[](k).bIsBlocking)
						{
							vChunksToProcess[i]->vChunkCells[y][x] = true;
						}


						FTransform transform = FTransform(FRotator(0, urd_rotation(gen), 0), location, FVector(1, 1, 1));

						struct params
						{
							FTransform transform;
							int64 x;
							int64 y;
						};

						params p;
						p.transform = transform;
						p.x = vChunksToProcess[i]->iX;
						p.y = vChunksToProcess[i]->iY;

						pCurrentLayer->operator[](k).pOwner->ProcessEvent( pCurrentLayer->operator[](k).pFunction, &p);

						fFullProb += pCurrentLayer->operator[](k).fProbabilityToSpawn;

						break;
					}
					
					fFullProb += pCurrentLayer->operator[](k).fProbabilityToSpawn;
				}
			}
		}
	}
}

void AFWGen::createTriggerBoxForChunk(AFWGChunk* pChunk)
{
	float fTriggerX = GetActorLocation().X;
	float fTriggerY = GetActorLocation().Y;
	float fTriggerZ = GetActorLocation().Z + LoadUnloadChunkMaxZ / 2;

	if (pChunk->iX != 0)
	{
		// Left or right chunk

		fTriggerX += (pChunk->iX * ChunkPieceColumnCount * ChunkPieceSizeX);
	}

	if (pChunk->iY != 0)
	{
		// Top or bottom chunk

		fTriggerY += (pChunk->iY * ChunkPieceRowCount * ChunkPieceSizeY);
	}

	pChunk->SetActorLocation(FVector(fTriggerX, fTriggerY, fTriggerZ));
	pChunk->pTriggerBox->SetBoxExtent(FVector(ChunkPieceColumnCount * ChunkPieceSizeX / 2,
		ChunkPieceRowCount * ChunkPieceSizeY / 2,
		LoadUnloadChunkMaxZ / 2));

	pChunk->pTriggerBox->SetGenerateOverlapEvents(true);
	pChunk->setOverlapToActors(vOverlapToClasses);
}

bool AFWGen::SetMaxRotation(float fMaxRotation)
{
	if (fMaxRotation >= 0.0f)
	{
		MaxRotation = fMaxRotation;

		return false;
	}
	else
	{
		return true;
	}
}

void AFWGen::SetMaxZDiffInCell(float fNewMaxZDiffInCell)
{
	MaxZDiffInCell = fNewMaxZDiffInCell;
}

#if WITH_EDITOR

void AFWGen::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName MemberPropertyChanged = (PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None);
	

	if (
		MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ChunkPieceRowCount)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ChunkPieceColumnCount)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ChunkPieceSizeX)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ChunkPieceSizeY)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, GenerationFrequency)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, GenerationOctaves)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, GenerationSeed)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, GenerationMaxZFromActorZ)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ComplexPreview)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, InvertWorld)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, FirstMaterialOnSecondProbability)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, FirstMaterialOnThirdProbability)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, SecondMaterialOnFirstProbability)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, SecondMaterialOnThirdProbability)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ThirdMaterialOnFirstProbability)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ThirdMaterialOnSecondProbability)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, TerrainCutHeightFromActorZ)
		|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, DrawChunkBounds)
		)
	{
		if (DrawChunkBounds == false)
		{
			FlushPersistentDebugLines(GetWorld());
		}

		if ((TerrainCutHeightFromActorZ > 1.0f) || (TerrainCutHeightFromActorZ < 0.0f))
		{
			TerrainCutHeightFromActorZ = 1.0f;
		}

		if ((FirstMaterialOnSecondProbability < 0.0f) || (FirstMaterialOnSecondProbability > 1.0f))
		{
			FirstMaterialOnSecondProbability = 0.01f;
		}

		if ((FirstMaterialOnThirdProbability < 0.0f) || (FirstMaterialOnThirdProbability > 1.0f))
		{
			FirstMaterialOnThirdProbability = 0.01f;
		}

		if ((SecondMaterialOnFirstProbability < 0.0f) || (SecondMaterialOnFirstProbability > 1.0f))
		{
			SecondMaterialOnFirstProbability = 0.01f;
		}

		if ((SecondMaterialOnThirdProbability < 0.0f) || (SecondMaterialOnThirdProbability > 1.0f))
		{
			SecondMaterialOnThirdProbability = 0.01f;
		}

		if ((ThirdMaterialOnFirstProbability < 0.0f) || (ThirdMaterialOnFirstProbability > 1.0f))
		{
			ThirdMaterialOnFirstProbability = 0.01f;
		}

		if ((ThirdMaterialOnSecondProbability < 0.0f) || (ThirdMaterialOnSecondProbability > 1.0f))
		{
			ThirdMaterialOnSecondProbability = 0.01f;
		}

		if ((IncreasedMaterialBlendProbability < 0.0f) || (IncreasedMaterialBlendProbability > 1.0f))
		{
			IncreasedMaterialBlendProbability = 0.0f;
		}



		if (ChunkPieceRowCount < 1)
		{
			ChunkPieceRowCount = 1;
		}

		if (ChunkPieceColumnCount < 1)
		{
			ChunkPieceColumnCount = 1;
		}

		if (GenerationMaxZFromActorZ < 0.0f)
		{
			GenerationMaxZFromActorZ = 0.0f;
		}

		if (GenerationSeed < 0)
		{
			GenerationSeed = 0;
		}

		if (GenerationFrequency > 64.0f)
		{
			GenerationFrequency = 64.0f;
		}
		else if (GenerationFrequency < 0.1f)
		{
			GenerationFrequency = 0.1f;
		}

		if (GenerationOctaves > 16)
		{
			GenerationOctaves = 16;
		}
		else if (GenerationOctaves < 1)
		{
			GenerationOctaves = 1;
		}

		refreshPreview();

		if (ComplexPreview)
		{
			GenerateWorld();
		}
		else
		{
			pChunkMap->clearWorld(pProcMeshComponent);
		}
	}
	else if ( MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, CreateWater) 
			|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ZWaterLevelInWorld)
			|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, WaterSize)
			|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, WaterMaterial)
			|| MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, GroundMaterial))
	{
		if (ZWaterLevelInWorld < 0.0f)
		{
			ZWaterLevelInWorld = 0.0f;
		}
		else if (ZWaterLevelInWorld > 1.0f)
		{
			ZWaterLevelInWorld = 1.0f;
		}

		if (GroundMaterial)
		{
			pProcMeshComponent->SetMaterial(0, GroundMaterial);
		}

		if (CreateWater)
		{
			if (WaterMaterial)
			{
				WaterPlane->SetMaterial(0, WaterMaterial);
				WaterPlane->SetVisibility(true);
			}

			WaterPlane->SetWorldLocation(FVector(
				GetActorLocation().X,
				GetActorLocation().Y,
				GetActorLocation().Z + ((GenerationMaxZFromActorZ - GetActorLocation().Z) * ZWaterLevelInWorld)
			));
			WaterPlane->SetWorldScale3D(FVector(
				((ChunkPieceColumnCount) * ChunkPieceSizeX) * (WaterSize / 100.0f),
				((ChunkPieceRowCount) * ChunkPieceSizeY)    * (WaterSize / 100.0f),
				0.1f
			));
		}
		else
		{
			WaterPlane->SetVisibility(false);
		}
	}
	else if ((MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ViewDistance))
			||
			(MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, WorldSize)))
	{
		if (WorldSize < -1)
		{
			WorldSize = -1;
		}

		if (ViewDistance < 1)
		{
			ViewDistance = 1;
		}

		refreshPreview();

		if (ComplexPreview) GenerateWorld();
	}
	else if (MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, ReadParamsFromFileRightNow))
	{
		if (ReadParamsFromFileRightNow)
		{
			LoadWorldParamsFromFile(PathToSaveFile);
		}
	}
	else if (MemberPropertyChanged == GET_MEMBER_NAME_CHECKED(AFWGen, SaveParamsToFileRightNow))
	{
		if (SaveParamsToFileRightNow)
		{
			SaveWorldParamsToFile(PathToSaveFile);
		}
	}
	else
	{
		refreshPreview();

		if (ComplexPreview) GenerateWorld();
	}
}
#endif // WITH_EDITOR

bool AFWGen::SetChunkPieceRowCount(int32 NewChunkPieceRowCount)
{
	if (NewChunkPieceRowCount < 1)
	{
		return true;
	}
	else
	{
		ChunkPieceRowCount = NewChunkPieceRowCount;

		return false;
	}
}

bool AFWGen::SetChunkPieceColumnCount(int32 NewChunkPieceColumnCount)
{
	if (NewChunkPieceColumnCount < 1)
	{
		return true;
	}
	else
	{
		ChunkPieceColumnCount = NewChunkPieceColumnCount;

		return false;
	}
}

bool AFWGen::SetChunkPieceSizeX(float NewChunkPieceSizeX)
{
	if (NewChunkPieceSizeX < 0.5f)
	{
		return true;
	}
	else
	{
		ChunkPieceSizeX = NewChunkPieceSizeX;

		return false;
	}
}

bool AFWGen::SetChunkPieceSizeY(float NewChunkPieceSizeY)
{
	if (NewChunkPieceSizeY < 0.5f)
	{
		return true;
	}
	else
	{
		ChunkPieceSizeY = NewChunkPieceSizeY;

		return false;
	}
}

bool AFWGen::SetViewDistance(int32 NewViewDistance)
{
	if (NewViewDistance < 1)
	{
		return true;
	}
	else
	{
		ViewDistance = NewViewDistance;

		return false;
	}
}

void AFWGen::SetLoadUnloadChunkMaxZ(float NewLoadUnloadChunkMaxZ)
{
	LoadUnloadChunkMaxZ = NewLoadUnloadChunkMaxZ;
}

bool AFWGen::SetGenerationFrequency(float NewGenerationFrequency)
{
	if ((NewGenerationFrequency > 64.0f) || (NewGenerationFrequency < 0.1f))
	{
		return true;
	}
	else
	{
		GenerationFrequency = NewGenerationFrequency;

		return false;
	}
}

bool AFWGen::SetGenerationOctaves(int32 NewGenerationOctaves)
{
	if ((NewGenerationOctaves > 16) || (NewGenerationOctaves < 1))
	{
		return true;
	}
	else
	{
		GenerationOctaves = NewGenerationOctaves;

		return false;
	}
}

bool AFWGen::SetGenerationSeed(int32 NewGenerationSeed)
{
	if (NewGenerationSeed < 0)
	{
		return true;
	}
	else
	{
		GenerationSeed = NewGenerationSeed;

		return false;
	}
}

bool AFWGen::SetGenerationMaxZFromActorZ(float NewGenerationMaxZFromActorZ)
{
	if (NewGenerationMaxZFromActorZ < 0.0f)
	{
		return true;
	}
	else
	{
		GenerationMaxZFromActorZ = NewGenerationMaxZFromActorZ;

		return false;
	}
}

void AFWGen::SetInvertWorld(bool InvertWorld)
{
	this->InvertWorld = InvertWorld;
}

bool AFWGen::SetWorldSize(int32 NewWorldSize)
{
	if (NewWorldSize < -1)
	{
		return true;
	}
	else
	{
		WorldSize = NewWorldSize;

		return false;
	}
}

void AFWGen::SetGroundMaterial(UMaterialInterface* NewGroundMaterial)
{
	GroundMaterial = NewGroundMaterial;
}

bool AFWGen::SetFirstMaterialMaxRelativeHeight(float NewFirstMaterialMaxRelativeHeight)
{
	if ((NewFirstMaterialMaxRelativeHeight < 0.0f) || (NewFirstMaterialMaxRelativeHeight > 1.0f) || (NewFirstMaterialMaxRelativeHeight > SecondMaterialMaxRelativeHeight))
	{
		return true;
	}
	else
	{
		FirstMaterialMaxRelativeHeight = NewFirstMaterialMaxRelativeHeight;

		return false;
	}
}

bool AFWGen::SetSecondMaterialMaxRelativeHeight(float NewSecondMaterialMaxRelativeHeight)
{
	if ((NewSecondMaterialMaxRelativeHeight < 0.0f) || (NewSecondMaterialMaxRelativeHeight > 1.0f) || (NewSecondMaterialMaxRelativeHeight < FirstMaterialMaxRelativeHeight))
	{
		return true;
	}
	else
	{
		SecondMaterialMaxRelativeHeight = NewSecondMaterialMaxRelativeHeight;

		return false;
	}
}

bool AFWGen::SetMaterialHeightMaxDeviation(float NewMaterialHeightMaxDeviation)
{
	if ((NewMaterialHeightMaxDeviation > 1.0f) || (NewMaterialHeightMaxDeviation < 0.0f))
	{
		return true;
	}
	else
	{
		MaterialHeightMaxDeviation = NewMaterialHeightMaxDeviation;

		return false;
	}
}

bool AFWGen::SetTerrainCutHeightFromActorZ(float NewTerrainCutHeightFromActorZ)
{
	if ((NewTerrainCutHeightFromActorZ > 1.0f) || (NewTerrainCutHeightFromActorZ < 0.0f))
	{
		return true;
	}
	else
	{
		TerrainCutHeightFromActorZ = NewTerrainCutHeightFromActorZ;
		return false;
	}
}

bool AFWGen::SetFirstMaterialOnOtherProbability(float FirstOnSecond, float FirstOnThird)
{
	if ( ((FirstOnSecond > 1.0f) || (FirstOnSecond < 0.0f)) || ((FirstOnThird > 1.0f) || (FirstOnThird < 0.0f)))
	{
		return true;
	}
	else
	{
		FirstMaterialOnSecondProbability = FirstOnSecond;
		FirstMaterialOnThirdProbability  = FirstOnThird;
		return false;
	}
}

bool AFWGen::SetSecondMaterialOnOtherProbability(float SecondOnFirst, float SecondOnThird)
{
	if ( ((SecondOnFirst > 1.0f) || (SecondOnFirst < 0.0f)) || ((SecondOnThird > 1.0f) || (SecondOnThird < 0.0f)))
	{
		return true;
	}
	else
	{
		SecondMaterialOnFirstProbability = SecondOnFirst;
		SecondMaterialOnThirdProbability = SecondOnThird;
		return false;
	}
}

bool AFWGen::SetThirdMaterialOnOtherProbability(float ThirdOnFirst, float ThirdOnSecond)
{
	if ( ((ThirdOnFirst > 1.0f) || (ThirdOnFirst < 0.0f)) || ((ThirdOnSecond > 1.0f) || (ThirdOnSecond < 0.0f)))
	{
		return true;
	}
	else
	{
		ThirdMaterialOnFirstProbability  = ThirdOnFirst;
		ThirdMaterialOnSecondProbability = ThirdOnSecond;
		return false;
	}
}

void AFWGen::SetCreateWater(bool CreateWater)
{
	this->CreateWater = CreateWater;
}

bool AFWGen::SetZWaterLevelInWorld(float NewZWaterLevelInWorld)
{
	if ((NewZWaterLevelInWorld < 0.0f) || (NewZWaterLevelInWorld > 1.0f))
	{
		return true;
	}
	else
	{
		ZWaterLevelInWorld = NewZWaterLevelInWorld;

		return false;
	}
}

bool AFWGen::SetWaterSize(int32 NewWaterSize)
{
	if (NewWaterSize <= 0)
	{
		return true;
	}
	else
	{
		WaterSize = NewWaterSize;

		return false;
	}
}

void AFWGen::SetWaterMaterial(UMaterialInterface* NewWaterMaterial)
{
	WaterMaterial = NewWaterMaterial;
}

#if WITH_EDITOR
void AFWGen::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	refreshPreview();

	if (ComplexPreview) GenerateWorld();
}
#endif // WITH_EDITOR

void AFWGen::BeginPlay()
{
	Super::BeginPlay();
}

void AFWGen::generateSeed()
{
	uint32_t seed = 0;

	if (GenerationSeed == 0)
	{
		// Generate seed
		std::mt19937_64 gen(std::random_device{}());
		std::uniform_int_distribution<unsigned int> uid(0, UINT_MAX);

		seed = uid(gen);
	}
	else
	{
		seed = GenerationSeed;
	}

	iGeneratedSeed = seed;
}

float AFWGen::pickVertexMaterial(double height, std::uniform_real_distribution<float>* pUrd, std::mt19937_64* pRnd, float* pfLayerTypeWithoutRnd)
{
	std::uniform_real_distribution<float> urd_mat(0.005f, 1.0f);
	std::uniform_int_distribution<int> urd_bool(0, 1);


	float fDiviation = (*pUrd)(*pRnd);

	float fVertexColor = 0.0f; // apply first material to the vertex

	if ( (height >= (FirstMaterialMaxRelativeHeight + fDiviation)) 
		&& (height <= (SecondMaterialMaxRelativeHeight + fDiviation)) )
	{
		fVertexColor = 0.5f; // apply second material to the vertex

		if (pfLayerTypeWithoutRnd)
		{
			*pfLayerTypeWithoutRnd = fVertexColor;
		}

		float fFirstProb = urd_mat(*pRnd);
		float fThirdProb  = urd_mat(*pRnd);

		bool bPickFirst = false, bPickThird = false;

		if (fFirstProb <= FirstMaterialOnSecondProbability)
		{
			bPickFirst = true;
		}

		if (fThirdProb <= ThirdMaterialOnSecondProbability)
		{
			bPickThird = true;
		}

		if (bPickFirst && bPickThird)
		{
			if (urd_bool(*pRnd))
			{
				// Pick Third
				fVertexColor = 1.0f; // apply third material to the vertex
			}
			else
			{
				// Pick First
				fVertexColor = 0.0f; // apply first material to the vertex
			}
		}
		else
		{
			if (bPickFirst)
			{
				fVertexColor = 0.0f; // apply first material to the vertex
			}

			if (bPickThird)
			{
				fVertexColor = 1.0f; // apply third material to the vertex
			}
		}
	}
	else if (height >= (SecondMaterialMaxRelativeHeight + fDiviation))
	{
		fVertexColor = 1.0f; // apply third material to the vertex

		if (pfLayerTypeWithoutRnd)
		{
			*pfLayerTypeWithoutRnd = fVertexColor;
		}

		float fFirstProb  = urd_mat(*pRnd);
		float fSecondProb = urd_mat(*pRnd);

		bool bPickFirst = false, bPickSecond = false;

		if (fFirstProb <= FirstMaterialOnThirdProbability)
		{
			bPickFirst = true;
		}

		if (fSecondProb <= SecondMaterialOnThirdProbability)
		{
			bPickSecond = true;
		}

		if (bPickFirst && bPickSecond)
		{
			if (urd_bool(*pRnd))
			{
				// Pick Third
				fVertexColor = 0.5f; // apply second material to the vertex
			}
			else
			{
				// Pick First
				fVertexColor = 0.0f; // apply first material to the vertex
			}
		}
		else
		{
			if (bPickFirst)
			{
				fVertexColor = 0.0f; // apply first material to the vertex
			}

			if (bPickSecond)
			{
				fVertexColor = 0.5f; // apply second material to the vertex
			}
		}
	}
	else
	{
		// first material

		fVertexColor = 0.0f;

		if (pfLayerTypeWithoutRnd)
		{
			*pfLayerTypeWithoutRnd = fVertexColor;
		}

		float fSecondProb = urd_mat(*pRnd);
		float fThirdProb  = urd_mat(*pRnd);

		bool bPickSecond = false, bPickThird = false;

		if (fSecondProb <= SecondMaterialOnFirstProbability)
		{
			bPickSecond = true;
		}

		if (fThirdProb <= ThirdMaterialOnFirstProbability)
		{
			bPickThird = true;
		}

		if (bPickSecond && bPickThird)
		{
			if (urd_bool(*pRnd))
			{
				// Pick Third
				fVertexColor = 1.0f; // apply third material to the vertex
			}
			else
			{
				// Pick Second
				fVertexColor = 0.5f; // apply second material to the vertex
			}
		}
		else
		{
			if (bPickSecond)
			{
				fVertexColor = 0.5f; // apply second material to the vertex
			}

			if (bPickThird)
			{
				fVertexColor = 1.0f; // apply third material to the vertex
			}
		}
	}

	return fVertexColor;
}

AFWGChunk* AFWGen::generateChunk(long long iX, long long iY, int32 iSectionIndex, bool bAroundCenter,
	long long iUnloadX, long long iUnloadY, bool bUnload)
{
	// Generation setup

	uint32_t seed = iGeneratedSeed;



	// We ++ here because we start to make polygons from 2nd row
	int32 iCorrectedRowCount = ChunkPieceRowCount + 1;
	int32 iCorrectedColumnCount = ChunkPieceColumnCount + 1;




	// Perlin Noise setup

	const siv::PerlinNoise perlinNoise(seed);
	const double fx = ((iCorrectedColumnCount - 1) * ChunkPieceSizeX) / GenerationFrequency;
	const double fy = ((iCorrectedRowCount - 1) * ChunkPieceSizeY) / GenerationFrequency;




	// Prepare chunk coordinates

	float fChunkX = GetActorLocation().X;
	float fChunkY = GetActorLocation().Y;

	if (iX != 0)
	{
		// Left or right chunk

		fChunkX += (iX * ChunkPieceColumnCount * ChunkPieceSizeX);
	}
	
	if (iY != 0)
	{
		// Top or bottom chunk

		fChunkY += (iY * ChunkPieceRowCount * ChunkPieceSizeY);
	}




	// Generation params

	float fStartX = fChunkX - ((iCorrectedColumnCount - 1) * ChunkPieceSizeX) / 2;
	float fStartY = fChunkY - ((iCorrectedRowCount    - 1) * ChunkPieceSizeY) / 2;

	FVector vPrevLocation(fStartX, fStartY, GetActorLocation().Z);




	// Create chunk.

	AFWGChunk* pNewChunk = nullptr;

	if (bUnload == false)
	{
		// You don't want to use NewObject for Actors (only UObjects).
		//AFWGChunk* pNewChunk = NewObject<AFWGChunk>(GetTransientPackage(), MakeUniqueObjectName(this, AFWGChunk::StaticClass(), "Chunk_"));
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		pNewChunk = GetWorld()->SpawnActor<AFWGChunk>(AFWGChunk::StaticClass(), FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1)), params);
		pNewChunk->setInit(iX, iY, iSectionIndex, bAroundCenter);
		pNewChunk->setChunkSize(DivideChunkXCount, DivideChunkYCount);
		pNewChunk->setChunkMap(pChunkMap);
	}
	else
	{
		for (size_t i = 0; i < pChunkMap->vChunks.size(); i++)
		{
			if (pChunkMap->vChunks[i]->iX == iUnloadX
				&& pChunkMap->vChunks[i]->iY == iUnloadY)
			{
				pNewChunk = pChunkMap->vChunks[i];

				pNewChunk->setUpdate(iX, iY, bAroundCenter);
				pNewChunk->setChunkSize(DivideChunkXCount, DivideChunkYCount);

				iSectionIndex = pNewChunk->iSectionIndex;

				break;
			}
		}
	}



	// Prepare random for vectex color

	std::mt19937_64 rnd(iGeneratedSeed);
	std::uniform_real_distribution<float> urd(-MaterialHeightMaxDeviation, MaterialHeightMaxDeviation);


	// Generation

	float fMaxGeneratedZ = GetActorLocation().Z;
	size_t iMaxGeneratedZIndex = 0;

	for (int32 i = 0; i < iCorrectedRowCount; i++)
	{
		for (int32 j = 0; j < iCorrectedColumnCount; j++)
		{
			// Generate vertex
			double generatedValue = perlinNoise.octaveNoise0_1(vPrevLocation.X / fx, vPrevLocation.Y / fy, GenerationOctaves);

			if (generatedValue > TerrainCutHeightFromActorZ)
			{
				generatedValue = TerrainCutHeightFromActorZ;
			}

			if (InvertWorld)
			{
				generatedValue = 1.0 - generatedValue;
			}


			pNewChunk->vNormals      .Add(FVector(0, 0, 1.0f));
			pNewChunk->vUV0          .Add(FVector2D(i, j));
			pNewChunk->vTangents     .Add(FProcMeshTangent(0.0f, 1.0f, 0.0f));



			// Set vertex Z

			// Here return value from perlinNoise.octaveNoise0_1 can be
			// 0    , but should be   GetActorLocation().Z
			// ...  , but should be   value from interval [GetActorLocation().Z; GenerationMaxZFromActorZ]
			// 1    , but should be   GenerationMaxZFromActorZ

			vPrevLocation.Z = GetActorLocation().Z + (GenerationMaxZFromActorZ * generatedValue);

			if (vPrevLocation.Z > fMaxGeneratedZ)
			{
				fMaxGeneratedZ = vPrevLocation.Z;
				iMaxGeneratedZIndex = pNewChunk->vVertices.Num();
			}

			pNewChunk->vVertices .Add (vPrevLocation);



			// Set "material" to vertex

			float fAlphaColor = 0.0f;

			float fAlphaColorWithoutRnd = 0.0f;

			if (SecondMaterialUnderWater && (generatedValue < (ZWaterLevelInWorld + 0.005f)))
			{
				fAlphaColor = 0.5f;
				fAlphaColorWithoutRnd = 0.5f;
			}
			else
			{
				if ((i == 0) || (i == iCorrectedRowCount - 1) || (j == 0) || (j == iCorrectedColumnCount - 1))
				{
					pickVertexMaterial(generatedValue, &urd, &rnd, &fAlphaColorWithoutRnd);
					fAlphaColor = fAlphaColorWithoutRnd;
				}
				else
				{
					fAlphaColor = pickVertexMaterial(generatedValue, &urd, &rnd, &fAlphaColorWithoutRnd);
				}
			}

			pNewChunk->vVertexColors .Add(FLinearColor(0.0f, 0.0f, 0.0f, fAlphaColor));

			if (areEqual(fAlphaColorWithoutRnd, 0.0f, 0.1f))
			{
				// First Layer

				pNewChunk->vLayerIndex.push_back(1);
			}
			else if (areEqual(fAlphaColorWithoutRnd, 0.5f, 0.1f))
			{
				// Second Layer
				pNewChunk->vLayerIndex.push_back(2);
			}
			else
			{
				// Third Layer
				pNewChunk->vLayerIndex.push_back(3);
			}



			vPrevLocation.X += ChunkPieceSizeX;

			if (i != 0)
			{
				//     j = 0,   1,   2,   3  ...
				// i = 0:  +----+----+----+- ...
				//         |   /|   /|   /|
				//         |  / |  / |  / |
				//         | /  | /  | /  |
				// i = 1:  +----+----+----+- ...

				int32 iFirstIndexInRow = (i - 1) * iCorrectedColumnCount;

				if (j == 0)
				{
					// Add triangle #1
					pNewChunk->vTriangles.Add(iFirstIndexInRow + j);
					pNewChunk->vTriangles.Add(i * iCorrectedColumnCount + j);
					pNewChunk->vTriangles.Add(iFirstIndexInRow + j + 1);
				}
				else
				{
					// Add triangle #2
					pNewChunk->vTriangles.Add(iFirstIndexInRow + j);
					pNewChunk->vTriangles.Add(i * iCorrectedColumnCount + j - 1);
					pNewChunk->vTriangles.Add(i * iCorrectedColumnCount + j);

					if (j < (iCorrectedColumnCount - 1))
					{
						// Add triangle #1
						pNewChunk->vTriangles.Add(iFirstIndexInRow + j);
						pNewChunk->vTriangles.Add(i * iCorrectedColumnCount + j);
						pNewChunk->vTriangles.Add(iFirstIndexInRow + j + 1);
					}
				}
			}
		}

		vPrevLocation.Set(fStartX, vPrevLocation.Y + ChunkPieceSizeY, GetActorLocation().Z);
	}

	pNewChunk->iMaxZVertexIndex = iMaxGeneratedZIndex;

	if (bUnload == false)
	{
		pProcMeshComponent->CreateMeshSection_LinearColor(iSectionIndex, pNewChunk->vVertices, pNewChunk->vTriangles, pNewChunk->vNormals,
			pNewChunk->vUV0, pNewChunk->vVertexColors, pNewChunk->vTangents, true);

		// Set material
		if (GroundMaterial)
		{
			pProcMeshComponent->SetMaterial(iSectionIndex, GroundMaterial);
		}

		pNewChunk->setMeshSection(pProcMeshComponent->GetProcMeshSection(iSectionIndex));
	}
	else
	{
		pProcMeshComponent->UpdateMeshSection_LinearColor(iSectionIndex, pNewChunk->vVertices, pNewChunk->vNormals, 
			pNewChunk->vUV0, pNewChunk->vVertexColors, pNewChunk->vTangents);
	}

	return pNewChunk;
}

#if WITH_EDITOR
void AFWGen::refreshPreview()
{
	if (WorldSize == -1)
	{
		PreviewPlane ->SetBoxExtent ( FVector (
			(ChunkPieceColumnCount * ChunkPieceSizeX / 2),
			(ChunkPieceRowCount * ChunkPieceSizeY / 2),
			GenerationMaxZFromActorZ / 2
		)
		);
	}
	else if (WorldSize == 0)
	{
		PreviewPlane ->SetBoxExtent ( FVector (
			(ViewDistance * 2 + 1) * (ChunkPieceColumnCount * ChunkPieceSizeX / 2),
			(ViewDistance * 2 + 1) * (ChunkPieceRowCount * ChunkPieceSizeY / 2),
			GenerationMaxZFromActorZ / 2
		)
		);
	}
	else
	{
		PreviewPlane ->SetBoxExtent ( FVector (
			((WorldSize * ViewDistance) * 2 + 1) * (ChunkPieceColumnCount * ChunkPieceSizeX / 2),
			((WorldSize * ViewDistance) * 2 + 1) * (ChunkPieceRowCount * ChunkPieceSizeY / 2),
			GenerationMaxZFromActorZ / 2
		)
		);
	}

	PreviewPlane ->SetWorldLocation ( FVector (
		GetActorLocation () .X,
		GetActorLocation () .Y,
		GetActorLocation () .Z + (GenerationMaxZFromActorZ) / 2
	)   
	);
}
#endif // WITH_EDITOR



bool AFWGen::areEqual(float a, float b, float eps)
{
	return fabs(a - b) < eps;
}

void AFWGen::compareHeightDifference(AFWGChunk* pChunk, std::vector<bool>& vProcessedVertices, float& fCurrentZ, size_t iCompareToIndex, float& fSteepSlopeMinHeightDiff)
{
	if (vProcessedVertices[iCompareToIndex] == false)
	{
		if (fabs(pChunk->vVertices[iCompareToIndex].Z - fCurrentZ) > fSteepSlopeMinHeightDiff)
		{
			pChunk->vVertexColors[iCompareToIndex] = FLinearColor(0.0f, 0.0f, 0.0f, 0.5f);
		}

		vProcessedVertices[iCompareToIndex] = true;
	}
}

void AFWGen::SetSecondMaterialUnderWater(bool NewSecondMaterialUnderWater)
{
	SecondMaterialUnderWater = NewSecondMaterialUnderWater;
}

bool AFWGen::SetIncreasedMaterialBlendProbability(float NewIncreasedMaterialBlendProbability)
{
	if (NewIncreasedMaterialBlendProbability < 0.0f || NewIncreasedMaterialBlendProbability > 1.0f)
	{
		return true;
	}
	else
	{
		IncreasedMaterialBlendProbability = NewIncreasedMaterialBlendProbability;

		return false;
	}
}

void AFWGen::SetApplyGroundMaterialBlend(bool bApply)
{
	ApplyGroundMaterialBlend = bApply;
}

void AFWGen::SetApplySlopeDependentBlend(bool bApply)
{
	ApplySlopeDependentBlend = bApply;
}

bool AFWGen::SetMinSlopeHeightMultiplier(float NewMinSlopeHeightMultiplier)
{
	if (NewMinSlopeHeightMultiplier < 0.0f || NewMinSlopeHeightMultiplier > 1.0f)
	{
		return true;
	}
	else
	{
		MinSlopeHeightMultiplier = NewMinSlopeHeightMultiplier;

		return false;
	}
}

bool AFWGen::SetDivideChunkXCount(int32 DivideChunkXCount)
{
	if (DivideChunkXCount < 1)
	{
		return true;
	}
	else
	{
		this->DivideChunkXCount = DivideChunkXCount;

		return false;
	}
}

bool AFWGen::SetDivideChunkYCount(int32 DivideChunkYCount)
{
	if (DivideChunkYCount < 1)
	{
		return true;
	}
	else
	{
		this->DivideChunkYCount = DivideChunkYCount;

		return false;
	}
}

bool AFWGen::SetMaxOffsetByX(float fMaxOffsetByX)
{
	if (fMaxOffsetByX >= 0.0f && fMaxOffsetByX <= 1.0f)
	{
		MaxOffsetByX = fMaxOffsetByX;

		return false;
	}
	else
	{
		return true;
	}
}

bool AFWGen::SetMaxOffsetByY(float fMaxOffsetByY)
{
	if (fMaxOffsetByY >= 0.0f && fMaxOffsetByY <= 1.0f)
	{
		MaxOffsetByY = fMaxOffsetByY;

		return false;
	}
	else
	{
		return true;
	}
}

FWGenChunkMap::FWGenChunkMap(AFWGen* pGen)
{
	pCurrentChunk = nullptr;
	this->pGen    = pGen;
}

void FWGenChunkMap::loadNewChunk(long long iLoadX, long long iLoadY, long long iUnloadX, long long iUnloadY)
{
	if (pGen->WorldSize != -1)
	{
		bool bAroundCenter = false;

		if ((iLoadX <= 1) && (iLoadX >= -1) && (iLoadY <= 1) && (iLoadY >= -1))
		{
			bAroundCenter = true;
		}


		AFWGChunk* pNewChunk = pGen->generateChunk(iLoadX, iLoadY, pGen->iCurrentSectionIndex, bAroundCenter, iUnloadX, iUnloadY, true);

		if (pGen->ApplyGroundMaterialBlend)
		{
			pGen->blendWorldMaterialsMore(pNewChunk);
		}

		if (pGen->ApplySlopeDependentBlend)
		{
			pGen->applySlopeDependentBlend(pNewChunk);
		}

		// Update mesh.
		pGen->pProcMeshComponent->UpdateMeshSection_LinearColor(pNewChunk->iSectionIndex, pNewChunk->vVertices, pNewChunk->vNormals,
			pNewChunk->vUV0, pNewChunk->vVertexColors, pNewChunk->vTangents);



		// Move Water Plane.

		float fX = pGen->GetActorLocation().X;
		float fY = pGen->GetActorLocation().Y;

		if (iLoadX != 0)
		{
			fX += (iLoadX * pGen->ChunkPieceColumnCount * pGen->ChunkPieceSizeX);
		}
		
		if (iLoadY != 0)
		{
			fY += (iLoadY * pGen->ChunkPieceRowCount * pGen->ChunkPieceSizeY);
		}


		pGen->WaterPlane->SetWorldLocation(FVector(
			fX,
			fY,
			pGen->GetActorLocation().Z + (pGen->GenerationMaxZFromActorZ * pGen->ZWaterLevelInWorld)
		));


		// Do last steps.

		// Unload old actors.
		if (pGen->pCallbackToDespawn)
		{
			struct params
			{
				int64 x;
				int64 y;
			};

			params p;
			p.x = iUnloadX;
			p.y = iUnloadY;

			pGen->pCallbackToDespawn->pOwner->ProcessEvent( pGen->pCallbackToDespawn->pFunction, &p);
		}

		pGen->spawnObjects(pNewChunk);

		pGen->createTriggerBoxForChunk(pNewChunk);
	}
}

void FWGenChunkMap::addChunk(AFWGChunk* pChunk)
{
	vChunks.push_back(pChunk);
}

void FWGenChunkMap::clearWorld(UProceduralMeshComponent* pProcMeshComponent)
{
	for (size_t i = 0; i < vChunks.size(); i++)
	{
		if (!vChunks[i]->IsValidLowLevel())
		{
			continue;;
		}

		if (vChunks[i]->IsPendingKill())
		{
			continue;
		}

		vChunks[i]->Destroy();
		vChunks[i] = nullptr;
	}

	vChunks.clear();

	pProcMeshComponent->ClearAllMeshSections();
}

void FWGenChunkMap::setCurrentChunk(AFWGChunk* pChunk)
{
	if (pCurrentChunk)
	{
		if (pGen->WorldSize > 0)
		{
			long long iMaxCoord = pGen->WorldSize;

			if (abs(pChunk->iX) == pGen->WorldSize || abs(pChunk->iY) == pGen->WorldSize)
			{
				return;
			}
		}

		long long offsetX = pChunk->iX - pCurrentChunk->iX;
		long long offsetY = pChunk->iY - pCurrentChunk->iY;

		if (offsetX != 0 && offsetY != 0)
		{
			return;
		}

		float fChunkXSize = pGen->ChunkPieceColumnCount * pGen->ChunkPieceSizeX;
		float fChunkYSize = pGen->ChunkPieceRowCount * pGen->ChunkPieceSizeY;

		mtxLoadChunks.lock();



		// Move blocking volumes.

		FVector volumeLocation = pGen->pBlockingVolumeX1->GetComponentLocation();
		volumeLocation.X += offsetX * fChunkXSize;
		volumeLocation.Y += offsetY * fChunkYSize;
		pGen->pBlockingVolumeX1->SetWorldLocation(volumeLocation);

		volumeLocation = pGen->pBlockingVolumeX2->GetComponentLocation();
		volumeLocation.X += offsetX * fChunkXSize;
		volumeLocation.Y += offsetY * fChunkYSize;
		pGen->pBlockingVolumeX2->SetWorldLocation(volumeLocation);

		volumeLocation = pGen->pBlockingVolumeY1->GetComponentLocation();
		volumeLocation.X += offsetX * fChunkXSize;
		volumeLocation.Y += offsetY * fChunkYSize;
		pGen->pBlockingVolumeY1->SetWorldLocation(volumeLocation);

		volumeLocation = pGen->pBlockingVolumeY2->GetComponentLocation();
		volumeLocation.X += offsetX * fChunkXSize;
		volumeLocation.Y += offsetY * fChunkYSize;
		pGen->pBlockingVolumeY2->SetWorldLocation(volumeLocation);



		long long iX = pChunk->iX + (offsetX * pGen->ViewDistance);
		long long iY = pChunk->iY + (offsetY * pGen->ViewDistance);

		long long iUnloadX = pCurrentChunk->iX + (-offsetX * pGen->ViewDistance);
		long long iUnloadY = pCurrentChunk->iY + (-offsetY * pGen->ViewDistance);

		pCurrentChunk = pChunk;

		loadNewChunk(iX, iY, iUnloadX, iUnloadY);


		// Generate other chunks.

		for (int32 i = 1; i <= pGen->ViewDistance; i++)
		{
			if (offsetX == 0)
			{
				loadNewChunk(iX - i, iY, iUnloadX + i, iUnloadY);
				loadNewChunk(iX + i, iY, iUnloadX - i, iUnloadY);
			}
			else if (offsetY == 0)
			{
				loadNewChunk(iX, iY - i, iUnloadX, iUnloadY + i);
				loadNewChunk(iX, iY + i, iUnloadX, iUnloadY - i);
			}
		}

		mtxLoadChunks.unlock();
	}
	else
	{
		pCurrentChunk = pChunk;
	}
}

long long FWGenChunkMap::getCentralChunkX()
{
	if (pCurrentChunk)
	{
		return pCurrentChunk->iX;
	}
	else
	{
		return 0;
	}
}

long long FWGenChunkMap::getCentralChunkY()
{
	if (pCurrentChunk)
	{
		return pCurrentChunk->iY;
	}
	else
	{
		return 0;
	}
}

FWGenChunkMap::~FWGenChunkMap()
{
	for (size_t i = 0; i < vChunks.size(); i++)
	{
		if (!vChunks[i]->IsValidLowLevel())
		{
			continue;;
		}

		if (vChunks[i]->IsPendingKill())
		{
			continue;
		}

		vChunks[i]->Destroy();
		vChunks[i] = nullptr;
	}
}
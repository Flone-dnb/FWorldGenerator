// This file is part of the FWorldGenenerator.
// Copyright Aleksandr "Flone" Tretyakov (github.com/Flone-dnb).
// Licensed under the ZLib license.
// Refer to the LICENSE file included.


#include "FWGChunk.h"

#include "FWGen.h"

AFWGChunk::AFWGChunk()
{
	pTriggerBox = CreateDefaultSubobject<UBoxComponent>(MakeUniqueObjectName(this, UBoxComponent::StaticClass(), "Trigger"));
	pTriggerBox->SetupAttachment(RootComponent);
	pTriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	pTriggerBox->BodyInstance.SetCollisionProfileName(TEXT("OverlapAll"));
	
	// need to have this on BeginPlay because here they can cause some problems when working with blueprints
	//pTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFWGChunk::OnBeginOverlap);
	//pTriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFWGChunk::OnEndOverlap);
}

void AFWGChunk::BeginPlay()
{
	Super::BeginPlay();

	pTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFWGChunk::OnBeginOverlap);
	//pTriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFWGChunk::OnEndOverlap);
}

void AFWGChunk::setInit(long long ix, long long iy, int32 iSectionindex, bool bAroundcenter)
{
	pMeshSection = nullptr;

	this->iX = ix;
	this->iY = iy;

	this->iSectionIndex = iSectionindex;

	this->bAroundCenter = bAroundcenter;
}

void AFWGChunk::setUpdate(long long ix, long long iy, bool bAroundcenter)
{
	clearChunk();

	this->iX = ix;
	this->iY = iy;

	this->bAroundCenter = bAroundcenter;
}

void AFWGChunk::setChunkSize(int32 iXCount, int32 iYCount)
{
	vChunkCells.resize(iYCount);

	for (size_t i = 0; i < vChunkCells.size(); i++)
	{
		vChunkCells[i].resize(iXCount);
	}
}

void AFWGChunk::setChunkMap(FWGenChunkMap* pChunkmap)
{
	this->pChunkMap = pChunkmap;
}

void AFWGChunk::setOverlapToActors(std::vector<FString> vClasses)
{
	vClassesToOverlap = vClasses;
}

void AFWGChunk::clearChunk()
{
	vVertices     .Empty();
	vTriangles    .Empty();
	vNormals      .Empty();
	vUV0          .Empty();
	vVertexColors .Empty();
	vTangents     .Empty();

	vLayerIndex.clear();
	vChunkCells.clear();
}

void AFWGChunk::setMeshSection(FProcMeshSection* pMeshsection)
{
	this->pMeshSection  = pMeshsection;
}

AFWGChunk::~AFWGChunk()
{
	if (!pTriggerBox->IsValidLowLevel())
	{
		return;
	}

	if (pTriggerBox->IsPendingKill())
	{
		return;
	}

	pTriggerBox->DestroyComponent();
	pTriggerBox = nullptr;
}

void AFWGChunk::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ( (OtherActor != nullptr ) && (OtherActor != this) && ( OtherComp != nullptr ) )  
	{
		for (size_t i = 0; i < vClassesToOverlap.size(); i++)
		{
			if (OtherActor->GetClass()->GetName() == vClassesToOverlap[i])
			{
				pChunkMap->setCurrentChunk(this);

				break;
			}
		}
	}
}

//void AFWGChunk::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if ( (OtherActor != nullptr ) && (OtherActor != this) && ( OtherComp != nullptr ) ) 
//	{
//		for (size_t i = 0; i < vClassesToOverlap.size(); i++)
//		{
//			if (OtherActor->GetClass()->GetName() == vClassesToOverlap[i])
//			{
//				break;
//			}
//		}
//	}
//}
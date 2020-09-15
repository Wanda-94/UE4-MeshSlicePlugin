// Fill out your copyright notice in the Description page of Project Settings.



#include "MeshSlicer.h"
#include "..\Public\MeshSlicer.h"

// Sets default values
AMeshSlicer::AMeshSlicer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	IsInit = false;

	InitSliceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SliceMesh"));

	InitSliceProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SliceProcMesh"));

}

// Called when the game starts or when spawned
void AMeshSlicer::BeginPlay()
{
	Super::BeginPlay();
	
	//InitMeshSlice(FString("/RURUDOOSLICER/Mesh/Market/halfmarket"));

}

// Called every frame
void AMeshSlicer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsInit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Anchor Num:" + FString::FromInt(SliceAnchorArray.Num()));
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "ProcMesh Num:" + FString::FromInt(SliceProcMeshArray.Num()));
		FTransform BaseTransform = FTransform::Identity;
		for (int i = 0; i < SliceAnchorArray.Num(); i++)
		{
			SliceProcMeshArray[i]->SetWorldTransform(BaseTransform);
		}
		UpdateAllShowStatus();
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, "Cut num:" + FString::FromInt(SlicePartProcMeshCutIndArray.Num()));
		SetAllShowStatus();
		CreateAndShowCutPart();
	}

}

int AMeshSlicer::GetProceduralMeshVertexNum(UProceduralMeshComponent* ProcMesh)
{
	int TotalVertexNum = 0;
	for (int SectionInd = 0; SectionInd < ProcMesh->GetNumSections(); SectionInd++)
	{
		FProcMeshSection* Section = ProcMesh->GetProcMeshSection(SectionInd);
		if (Section != nullptr)
		{
			TotalVertexNum += (Section->ProcVertexBuffer.Num());
		}
	}
	return TotalVertexNum;
}

int AMeshSlicer::GetProceduralMeshFaceNum(UProceduralMeshComponent* ProcMesh)
{
	int TotalFaceNum = 0;
	for (int SectionInd = 0; SectionInd < ProcMesh->GetNumSections(); SectionInd++)
	{
		FProcMeshSection* Section = ProcMesh->GetProcMeshSection(SectionInd);
		if (Section != nullptr)
		{
			TotalFaceNum += (Section->ProcIndexBuffer.Num());
		}
	}
	return TotalFaceNum/3;
}

void AMeshSlicer::SliceProceduralMeshByFaceNum(FSliceAnchor& CurrSliceAnchor,UProceduralMeshComponent* CurrSliceProcMesh)
{
	int CurrFaceNum = GetProceduralMeshFaceNum(CurrSliceProcMesh);
	if (CurrFaceNum > MaxFaceNum)
	{
		float XLength = CurrSliceAnchor.RightUpAnchor.X - CurrSliceAnchor.LeftDownAnchor.X;
		float YLength = CurrSliceAnchor.RightUpAnchor.Y - CurrSliceAnchor.LeftDownAnchor.Y;
		float XSlicePoint = (CurrSliceAnchor.RightUpAnchor.X + CurrSliceAnchor.LeftDownAnchor.X) * 0.5f;
		float YSlicePoint = (CurrSliceAnchor.RightUpAnchor.Y + CurrSliceAnchor.LeftDownAnchor.Y) * 0.5f;
		FVector SlicePosition = FVector(XSlicePoint, YSlicePoint, 0.0f);
		FVector SliceNormal;
		if (XLength > YLength)
		{
			//slice by x axis

			SliceNormal = FVector(1.0f,0.0f,0.0f);
		}
		else
		{
			SliceNormal = FVector(0.0f,1.0f,0.0f);
		}
		UProceduralMeshComponent* HalfProcMesh;
		UKismetProceduralMeshLibrary::SliceProceduralMesh(CurrSliceProcMesh,SlicePosition,SliceNormal,true, HalfProcMesh,
			EProcMeshSliceCapOption::NoCap,nullptr);
		FSliceAnchor ProcMeshAnchor1, ProcMeshAnchor2;
		if (XLength > YLength)
		{
			ProcMeshAnchor1.LeftDownAnchor = CurrSliceAnchor.LeftDownAnchor;
			ProcMeshAnchor1.RightUpAnchor = FVector(XSlicePoint,CurrSliceAnchor.RightUpAnchor.Y,0.0f);
			ProcMeshAnchor2.LeftDownAnchor = FVector(XSlicePoint,CurrSliceAnchor.LeftDownAnchor.Y,0.0f);
			ProcMeshAnchor2.RightUpAnchor = CurrSliceAnchor.RightUpAnchor;
		}
		else
		{
			ProcMeshAnchor1.LeftDownAnchor = CurrSliceAnchor.LeftDownAnchor;
			ProcMeshAnchor1.RightUpAnchor = FVector(CurrSliceAnchor.RightUpAnchor.X, YSlicePoint, 0.0f);
			ProcMeshAnchor2.LeftDownAnchor = FVector(CurrSliceAnchor.LeftDownAnchor.X,YSlicePoint, 0.0f);
			ProcMeshAnchor2.RightUpAnchor = CurrSliceAnchor.RightUpAnchor;
		}
		SliceProceduralMeshByFaceNum(ProcMeshAnchor1, HalfProcMesh);
		SliceProceduralMeshByFaceNum(ProcMeshAnchor2, CurrSliceProcMesh);
	}
	else
	{
		SliceAnchorArray.Add(CurrSliceAnchor);
		SliceProcMeshArray.Add(CurrSliceProcMesh);
	}
}

void AMeshSlicer::UpdateAllShowStatus()
{
	SlicePartProcMeshCutIndArray.Empty();
	for (int i = 0; i < SliceProcMeshArray.Num(); i++)
	{
		int ShowStatus = CheckShowStatus(SliceAnchorArray[i]);
		if (ShowStatus == -1)
		{
			SlicePartShowFlag[i] = false;
		}
		else if (ShowStatus == 1)
		{
			SlicePartShowFlag[i] = true;
		}
		else
		{
			SlicePartShowFlag[i] = false;
			SlicePartProcMeshCutIndArray.Add(i);
		}
	}
}

int AMeshSlicer::CheckShowStatus(const FSliceAnchor& PartAnchor)
{
	//-1 out 1 in 0 cut
	float TrackAnchorMinX = TrackAnchor.LeftDownAnchor.X;
	float TrackAnchorMaxX = TrackAnchor.RightUpAnchor.X;
	float TrackAnchorMinY = TrackAnchor.LeftDownAnchor.Y;
	float TrackAnchorMaxY = TrackAnchor.RightUpAnchor.Y;

	float PartAnchorMinX = PartAnchor.LeftDownAnchor.X;
	float PartAnchorMaxX = PartAnchor.RightUpAnchor.X;
	float PartAnchorMinY = PartAnchor.LeftDownAnchor.Y;
	float PartAnchorMaxY = PartAnchor.RightUpAnchor.Y;

	if (PartAnchorMinX >= TrackAnchorMinX &&
		PartAnchorMaxX<=TrackAnchorMaxX &&
		PartAnchorMinY>=TrackAnchorMinY &&
		PartAnchorMaxY <= TrackAnchorMaxY)
	{
		return 1;
	}
	else if (PartAnchorMaxX<=TrackAnchorMinX ||
		PartAnchorMinX>=TrackAnchorMaxX ||
		PartAnchorMaxY<=TrackAnchorMinY ||
		PartAnchorMinY>=TrackAnchorMaxY)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void AMeshSlicer::SetAllShowStatus()
{
	FTransform TransformOffset = FTransform(FQuat::Identity,(TrackAnchor.LeftDownAnchor+TrackAnchor.RightUpAnchor)*(-0.5),FVector(1.0f));
	for (int i = 0; i < SlicePartShowFlag.Num(); i++)
	{
		if (SlicePartShowFlag[i])
		{
			SliceProcMeshArray[i]->SetWorldTransform(TransformOffset*ProcMeshTransform);
		}
		SliceProcMeshArray[i]->SetVisibility(SlicePartShowFlag[i]);
	}
}

void AMeshSlicer::CreateAndShowCutPart()
{
	FTransform TransformOffset = FTransform(FQuat::Identity, (TrackAnchor.LeftDownAnchor + TrackAnchor.RightUpAnchor) * (-0.5), FVector(1.0f));
	for (int i = 0; i < SlicePartProcMeshArray.Num(); i++)
	{
		SlicePartProcMeshArray[i]->SetVisibility(false);
	}
	for (int i = 0; i < SlicePartProcMeshCutIndArray.Num(); i++)
	{
		int CutMeshInd = SlicePartProcMeshCutIndArray[i];
		UProceduralMeshComponent* TemProcMesh = SliceProcMeshArray[CutMeshInd];
		UProceduralMeshComponent* OtherProcMesh = SlicePartProcMeshArray[i];
		OtherProcMesh->ClearAllMeshSections();
		for (int SectionInd = 0; SectionInd < TemProcMesh->GetNumSections(); SectionInd++)
		{
			OtherProcMesh->SetProcMeshSection(SectionInd,*(TemProcMesh->GetProcMeshSection(SectionInd)));
			OtherProcMesh->SetMaterial(SectionInd,TemProcMesh->GetMaterial(SectionInd));
		}
		OtherProcMesh->SetWorldTransform(FTransform::Identity);
		CutPart(SliceAnchorArray[CutMeshInd],OtherProcMesh);
		OtherProcMesh->SetWorldTransform(TransformOffset*ProcMeshTransform);
		OtherProcMesh->SetVisibility(true);
	}
}

void AMeshSlicer::CutPart(const FSliceAnchor& PartAnchor, UProceduralMeshComponent* PartProcMesh)
{
	float TrackAnchorMinX = TrackAnchor.LeftDownAnchor.X;
	float TrackAnchorMaxX = TrackAnchor.RightUpAnchor.X;
	float TrackAnchorMinY = TrackAnchor.LeftDownAnchor.Y;
	float TrackAnchorMaxY = TrackAnchor.RightUpAnchor.Y;

	float PartAnchorMinX = PartAnchor.LeftDownAnchor.X;
	float PartAnchorMaxX = PartAnchor.RightUpAnchor.X;
	float PartAnchorMinY = PartAnchor.LeftDownAnchor.Y;
	float PartAnchorMaxY = PartAnchor.RightUpAnchor.Y;

	UProceduralMeshComponent* TemProcMesh;

	UKismetProceduralMeshLibrary::SliceProceduralMesh(PartProcMesh, TrackAnchor.LeftDownAnchor, FVector(1.0f, 0.0f, 0.0f), false, TemProcMesh, EProcMeshSliceCapOption::NoCap, nullptr);
	UKismetProceduralMeshLibrary::SliceProceduralMesh(PartProcMesh, TrackAnchor.LeftDownAnchor, FVector(0.0f, 1.0f, 0.0f), false, TemProcMesh, EProcMeshSliceCapOption::NoCap, nullptr);
	UKismetProceduralMeshLibrary::SliceProceduralMesh(PartProcMesh, TrackAnchor.RightUpAnchor, FVector(-1.0f, 0.0f, 0.0f), false, TemProcMesh, EProcMeshSliceCapOption::NoCap, nullptr);
	UKismetProceduralMeshLibrary::SliceProceduralMesh(PartProcMesh, TrackAnchor.RightUpAnchor, FVector(0.0f, -1.0f, 0.0f), false, TemProcMesh, EProcMeshSliceCapOption::NoCap, nullptr);

}

void AMeshSlicer::SetTrackAnchor(FVector NewLeftDownAnchor, FVector NewRightUpAnchor)
{
	TrackAnchor.LeftDownAnchor = NewLeftDownAnchor;
	TrackAnchor.RightUpAnchor = NewRightUpAnchor;
}

void AMeshSlicer::InitMeshSlice(FString InitStaticMeshFilePath, int InitMaxFaceNum, int InitLodLevel)
{

	if (IsInit)
	{
		return;
	}

	MaxFaceNum = InitMaxFaceNum;

	LodLevel = InitLodLevel;

	StaticMeshFilePath = InitStaticMeshFilePath;

	ProcMeshTransform = FTransform::Identity;

	UStaticMesh* StaticMeshRef = (UStaticMesh*)StaticLoadObject(UStaticMesh::StaticClass(), NULL, *StaticMeshFilePath);

	if (StaticMeshRef)
	{
		InitSliceMesh->SetStaticMesh(StaticMeshRef);
		InitSliceMesh->SetVisibility(false);
		StaticMeshRef->bAllowCPUAccess = true;
	}

	UMaterialInterface* CutMaterialRef = (UMaterialInterface*)StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *MaterialFilePath);

	if (CutMaterialRef)
	{
		MatCutPart = CutMaterialRef;
	}

	UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(InitSliceMesh, LodLevel, InitSliceProcMesh, false);

	InitSliceProcMesh->SetWorldTransform(FTransform::Identity);

	InitSliceBoxCenter = InitSliceProcMesh->CalcLocalBounds().GetBox().GetCenter();

	InitSliceBoxSize = InitSliceProcMesh->CalcLocalBounds().GetBox().GetSize();

	InitSliceProcMeshAnchor.LeftDownAnchor = FVector(InitSliceBoxCenter.X - InitSliceBoxSize.X * 0.5f, InitSliceBoxCenter.Y - InitSliceBoxSize.Y * 0.5f, 0.0f);

	InitSliceProcMeshAnchor.RightUpAnchor = FVector(InitSliceBoxCenter.X + InitSliceBoxSize.X * 0.5f, InitSliceBoxCenter.Y + InitSliceBoxSize.Y * 0.5f, 0.0f);

	TrackAnchor.LeftDownAnchor = InitSliceProcMeshAnchor.LeftDownAnchor - InitSliceBoxCenter;

	TrackAnchor.RightUpAnchor = InitSliceProcMeshAnchor.RightUpAnchor - InitSliceBoxCenter;

	SliceProceduralMeshByFaceNum(InitSliceProcMeshAnchor, InitSliceProcMesh);

	for (int i = 0; i < SliceProcMeshArray.Num(); i++)
	{
		SlicePartShowFlag.Add(false);

		SlicePartProcMeshArray.Add(NewObject<UProceduralMeshComponent>(SliceProcMeshArray[i]->GetOuter()));

		SlicePartProcMeshArray[SlicePartProcMeshArray.Num() - 1]->SetWorldTransform(FTransform::Identity);
		
		SlicePartProcMeshArray[SlicePartProcMeshArray.Num() - 1]->SetVisibility(false);
		
		SlicePartProcMeshArray[SlicePartProcMeshArray.Num() - 1]->RegisterComponent();
	}

	IsInit = true;
}

void AMeshSlicer::SetProcMeshTransform(FTransform NewProcMeshTransform)
{
	ProcMeshTransform = NewProcMeshTransform;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ConstructorHelpers.h"
#include "Engine.h"
#include "GeomTools.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshSlicer.generated.h"

//note:The Track Anchor is in ProcMesh local transform rather than world transform,
//we put ProcMesh always in identity transform and cut it,
//but when render ProcMesh and CutProMesh we put them to ProcMeshTransform


USTRUCT(BlueprintType)
struct RURUDOOSLICER_API FSliceAnchor {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
		FVector LeftDownAnchor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector RightUpAnchor;

};

UCLASS()
class RURUDOOSLICER_API AMeshSlicer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeshSlicer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UFUNCTION(BlueprintCallable)
		int GetProceduralMeshVertexNum(UProceduralMeshComponent* ProcMesh);

	UFUNCTION(BlueprintCallable)
		int GetProceduralMeshFaceNum(UProceduralMeshComponent* ProcMesh);

	UFUNCTION(BlueprintCallable)
		void SliceProceduralMeshByFaceNum(FSliceAnchor& CurrSliceAnchor,UProceduralMeshComponent* CurrSliceProcMesh);

	UFUNCTION(BlueprintCallable)
		void UpdateAllShowStatus();

	UFUNCTION(BlueprintCallable)
		int CheckShowStatus(const FSliceAnchor& PartAnchor);

	UFUNCTION(BlueprintCallable)
		void SetAllShowStatus();

	UFUNCTION(BlueprintCallable)
		void CreateAndShowCutPart();

	UFUNCTION(BlueprintCallable)
		void CutPart(const FSliceAnchor& PartAnchor,UProceduralMeshComponent* PartProcMesh);

public:

	UFUNCTION(BlueprintCallable)
		void SetTrackAnchor(FVector NewLeftDownAnchor,FVector NewRightUpAnchor);

	UFUNCTION(BlueprintCallable)
		void InitMeshSlice(FString InitStaticMeshFilePath,int InitMaxFaceNum = 6000,int InitLodLevel = 3);

	UFUNCTION(BlueprintCallable)
		void SetProcMeshTransform(FTransform NewProcMeshTransform);

private:
	//FString StaticMeshFilePath = "/RURUDOOSLICER/Mesh/City/BigBigCity";
	UStaticMeshComponent* InitSliceMesh;

	UProceduralMeshComponent* InitSliceProcMesh;

	UMaterialInterface* MatCutPart;

	bool IsInit;

	int MaxFaceNum;

	int LodLevel;

	FString StaticMeshFilePath ;

	FString MaterialFilePath = "/RURUDOOSLICER/Material/Mat_CutPart";

	FVector InitSliceBoxCenter;

	FVector InitSliceBoxSize;

	FSliceAnchor InitSliceProcMeshAnchor;

	FSliceAnchor TrackAnchor;

	FTransform ProcMeshTransform;

	TArray<UProceduralMeshComponent*> SlicePartProcMeshArray;

	TArray<UProceduralMeshComponent*> SliceProcMeshArray;

	TArray<bool> SlicePartShowFlag;

	TArray<int> SlicePartProcMeshCutIndArray;

	TArray<FSliceAnchor> SliceAnchorArray;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Placeable.generated.h"

UCLASS()
class CTFPROJECT_API APlaceable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlaceable();
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PlaceableMesh;

protected:

private:	
	
};

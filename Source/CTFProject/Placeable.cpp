// Fill out your copyright notice in the Description page of Project Settings.

#include "Placeable.h"


// Sets default values
APlaceable::APlaceable()
{
   PlaceableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceableMesh"));
}


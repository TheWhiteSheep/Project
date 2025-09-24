// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyBaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API AMyBaseGameMode : public AGameMode
{
	GENERATED_BODY()
	
public: 
	AMyBaseGameMode(); 

	/* Get a valid spawn location and rotation for a new player or respawn. */
	FTransform GetSpawnPoint();

	/* Respawn the pawn controlled by the given PlayerController. */
	void RespawnActor(APlayerController* PlayerController);
};

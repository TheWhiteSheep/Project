// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBaseGameMode.h"
#include "GameFramework/PlayerStart.h" 
#include <Kismet/GameplayStatics.h>
#include "MyBasePlayerController.h"
#include "MyBasePlayerState.h"
#include "MyBaseGameState.h"

AMyBaseGameMode::AMyBaseGameMode()
{
    /* Set the pawn to null, as we will assign it when we spawn. */
    DefaultPawnClass = nullptr;

    /* Set the PlayerController class to our BasePlayerController class. */
    PlayerControllerClass = AMyBasePlayerController::StaticClass();

    /* Set the GameState class to our GameState class. */
    GameStateClass = AMyBaseGameState::StaticClass();

    /* Set the PlayerState class to our PlayerState class. */
    PlayerStateClass = AMyBasePlayerState::StaticClass();
}

FTransform AMyBaseGameMode::GetSpawnPoint()
{
    /* Store an array of the PlayerStarts. */
    TArray<AActor*> PlayerStarts;

    /* Get all of the actors that exist in the world that are PlayerStarts. */
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    /* If the PlayerStarts that exist in the world is greater than 0. */
    if (PlayerStarts.Num() > 0)
    {
        /* Then we get a random PlayerStart in the array. */
        int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);

        /* And we return that PlayerStart Transform. */
        return PlayerStarts[RandomIndex]->GetTransform();
    }

    /* Otherwise we return (0,0,0) */
    return FTransform();
}

void AMyBaseGameMode::RespawnActor(APlayerController* PlayerController)
{
    /* Return early if the PlayerController is invalid. */
    if (!PlayerController) { return; }

    /* Get the pawn currently controlled by this PlayerController. */
    APawn* CurrentPawn = PlayerController->GetPawn();

    /* Return early if the PlayerController doesn't currently possess a pawn. */
    if (CurrentPawn) {  
        /* Destroy the currently possessed pawn to make room for the new one. */
        CurrentPawn->Destroy();
    }

    /* Determine the spawn location and rotation for the new pawn. */
    FTransform SpawnTransform = GetSpawnPoint();

    /* Load the Blueprint class for the PlayerPawn. */
    UClass* PlayerPawnBPClass = StaticLoadClass(
        APawn::StaticClass(),
        nullptr,
        TEXT("/Game/ThirdPerson/Blueprints/BP_BaseCharacter.BP_BaseCharacter_C")
    );

    /* Warn if the Blueprint class couldn't be loaded. */
    if (PlayerPawnBPClass) {
        /* Spawn a new pawn at the specified spawn point. */
        APawn* NewCharacter = GetWorld()->SpawnActor<APawn>(
            PlayerPawnBPClass,
            SpawnTransform.GetLocation(),
            SpawnTransform.GetRotation().Rotator(),
            FActorSpawnParameters()
        );

        /* Return early if spawning failed. */
        if (NewCharacter) {  

            /* Possess the newly spawned pawn with the PlayerController. */
            PlayerController->Possess(NewCharacter);
        }
    }
}

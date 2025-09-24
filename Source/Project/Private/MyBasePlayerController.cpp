// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBasePlayerController.h"
#include "MyCameraManager.h"
#include "InputMappingContext.h"
#include <MyBaseGameMode.h>

AMyBasePlayerController::AMyBasePlayerController()
{
    /* Assign the Camera manager when the playercontroller is constructed. */
    PlayerCameraManagerClass = AMyCameraManager::StaticClass();

    UInputMappingContext* Default_IMC = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default.IMC_Default")));

    if (Default_IMC)
    {
        DefaultMappingContexts.Add(Default_IMC);
    }
}

void AMyBasePlayerController::BeginPlay()
{
    Super::BeginPlay();

    /* Return early if not local. */
    if (IsLocalController()) 
    { 
        /* If we are local then ask the Server to spawn the player. */
        ServerSpawnPlayer(this); 
    }
}

void AMyBasePlayerController::ServerSpawnPlayer_Implementation(APlayerController* PlayerController)
{
    /* Store a reference of the GameMode. */
    AMyBaseGameMode* Gamemode = Cast<AMyBaseGameMode>(GetWorld()->GetAuthGameMode());

    /* Return early if not the Gamemode. */
    if (Gamemode)
    {
        /* Call to our GameMode and Respawn the actor. */
        Gamemode->RespawnActor(this);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthGameMode pointer: %p"), GetWorld()->GetAuthGameMode());
    }
}

void AMyBasePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (IsLocalPlayerController())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
            {
                Subsystem->AddMappingContext(CurrentContext, 0);
            }
        }
    }
}
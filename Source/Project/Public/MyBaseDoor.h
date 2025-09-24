#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractiveInterface.h"
#include "MyBaseDoor.generated.h"

/**
 * AMyBaseDoor
 *
 * Basic door actor that can be interacted with using the InteractiveInterface.
 * Supports opening and closing with rotation, replication for networked games,
 * and smooth rotation using a timer.
 */
UCLASS()
class PROJECT_API AMyBaseDoor : public AActor, public IInteractiveInterface
{
    GENERATED_BODY()

public:
    /** Constructor: sets default values for this actor */
    AMyBaseDoor();

    /**
     * Implements the interaction interface function.
     * Called when a player interacts with this door.
     * @param Interactor The actor that triggered the interaction.
     */
    virtual void Interact_Implementation(AActor* Interactor) override;

protected:
    /** Called when the game starts or the actor is spawned */
    virtual void BeginPlay() override;

public:
    /** Called every frame */
    virtual void Tick(float DeltaTime) override;

    /** The static mesh representing the door */
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* DoorMesh;

    /** The static mesh representing the door */
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* DoorFrameMesh;


    /** The rotation of the door when closed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    FRotator ClosedRotation;

    /** The rotation of the door when open */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    FRotator OpenRotation;

    /** The speed at which the door rotates to open/close */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenSpeed = 2.0f;

    /**
     * Tracks whether the door is currently open.
     * Replicated to clients; calls OnRep_IsOpen() on change.
     */
    UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
    bool bIsOpen;

    /** Called on clients when bIsOpen is replicated */
    UFUNCTION()
    void OnRep_IsOpen();

    /** Toggles the door open/closed; handles authority check */
    UFUNCTION()
    void ToggleDoor();

    /** Smoothly rotates the door toward its target rotation */
    UFUNCTION()
    void UpdateDoorRotation();

    /** Server-side function to toggle the door in networked games */
    UFUNCTION(Server, Reliable)
    void Server_ToggleDoor();

private:
    /** Timer handle for door rotation updates */
    FTimerHandle DoorTimerHandle;
};

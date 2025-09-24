#include "MyBaseDoor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

/**
 * Constructor
 * Sets default values for this actor's properties.
 * Creates the door mesh and sets default rotations.
 */
AMyBaseDoor::AMyBaseDoor()
{
    // Disable Tick() by default since door does not need to update every frame
    PrimaryActorTick.bCanEverTick = false;

    // Create the door mesh and set as root component
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    RootComponent = DoorMesh;

    // Default rotations for open and closed states
    ClosedRotation = FRotator(0.f, 0.f, 0.f);
    OpenRotation = FRotator(0.f, 90.f, 0.f);
    bIsOpen = false;

    // Enable replication for networked games
    bReplicates = true;
    DoorMesh->SetIsReplicated(true);
}

/**
 * Called when the game starts or the actor is spawned
 * Sets the initial rotation of the door based on its open/closed state.
 */
void AMyBaseDoor::BeginPlay()
{
    Super::BeginPlay();
    DoorMesh->SetWorldRotation(bIsOpen ? OpenRotation : ClosedRotation);
}

/**
 * Called every frame
 * Currently unused as door updates are handled via timers.
 */
void AMyBaseDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

/**
 * Interact implementation from the interface
 * Called when a player interacts with the door
 * @param Interactor The actor performing the interaction
 */
void AMyBaseDoor::Interact_Implementation(AActor* Interactor)
{
    ToggleDoor();
}

/**
 * Toggles the door open/closed
 * Handles authority: if server, directly toggles; if client, calls server function.
 */
void AMyBaseDoor::ToggleDoor()
{
    if (HasAuthority())
    {
        // Flip the open state
        bIsOpen = !bIsOpen;

        // Trigger rotation update on server (and replicated to clients)
        OnRep_IsOpen();
    }
    else
    {
        // If client, request server to toggle
        Server_ToggleDoor();
    }
}

/**
 * Server-side RPC to toggle the door
 * Ensures the server has authority over the door state
 */
void AMyBaseDoor::Server_ToggleDoor_Implementation()
{
    ToggleDoor();
}

/**
 * Called when bIsOpen is replicated to clients
 * Starts the timer to smoothly interpolate the door rotation
 */
void AMyBaseDoor::OnRep_IsOpen()
{
    // Start a timer that repeatedly calls UpdateDoorRotation
    GetWorldTimerManager().SetTimer(DoorTimerHandle, this, &AMyBaseDoor::UpdateDoorRotation, 0.01f, true);
}

/**
 * Smoothly interpolates the door's rotation towards the target (open or closed)
 * Stops the timer when the rotation is close enough to the target
 */
void AMyBaseDoor::UpdateDoorRotation()
{
    FRotator TargetRot = bIsOpen ? OpenRotation : ClosedRotation;

    // Smoothly interpolate current rotation to target rotation
    DoorMesh->SetRelativeRotation(FMath::RInterpTo(
        DoorMesh->GetRelativeRotation(),
        TargetRot,
        GetWorld()->GetDeltaSeconds(),
        OpenSpeed
    ));

    // Stop timer when rotation is within 0.1 degrees of target
    if (DoorMesh->GetRelativeRotation().Equals(TargetRot, 0.1f))
    {
        GetWorldTimerManager().ClearTimer(DoorTimerHandle);
    }
}

/**
 * Specifies which properties are replicated over the network
 * @param OutLifetimeProps Array to store properties that should replicate
 */
void AMyBaseDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate door open state
    DOREPLIFETIME(AMyBaseDoor, bIsOpen);
}

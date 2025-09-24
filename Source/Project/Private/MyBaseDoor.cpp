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
    // Disable ticking by default; the door does not need to update every frame
    PrimaryActorTick.bCanEverTick = false;

    // Create the Door Frame component, which will act as the root of the actor
    DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
    // Set the frame as the root component of this actor
    RootComponent = DoorFrameMesh;

    // Create the Door component, which will be a child of the frame
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    // Attach the door to the frame so it moves/rotates with it
    DoorMesh->SetupAttachment(DoorFrameMesh);

    /* Load the Door Frame mesh asset from the content browser
    ConstructorHelpers::FObjectFinder is used in the constructor to locate assets at compile time */
    ConstructorHelpers::FObjectFinder<UStaticMesh> DoorFrameMeshAsset(TEXT("/Game/LevelPrototyping/Meshes/DoorFrame.DoorFrame"));
    /* Load the Door mesh asset from the content browser */
    ConstructorHelpers::FObjectFinder<UStaticMesh> DoorMeshAsset(TEXT("/Game/LevelPrototyping/Meshes/Door.Door"));

    /* If the frame asset was successfully found, assign it to the frame component */
    if (DoorFrameMeshAsset.Succeeded()) { DoorFrameMesh->SetStaticMesh(DoorFrameMeshAsset.Object); }

    /* If the door asset was successfully found, assign it to the door component */
    if (DoorMeshAsset.Succeeded()) { DoorMesh->SetStaticMesh(DoorMeshAsset.Object); }

    /* Apply a relative location offset to the door so it aligns correctly
    This is needed because the door pivot is on the corner rather than the center */
    if (DoorMesh) { DoorMesh->SetRelativeLocation(FVector(40.f, 0.f, 0.f)); }

    /*Set default rotations for door states
    ClosedRotation = door is fully closed */
    ClosedRotation = FRotator(0.f, 0.f, 0.f);
    /* OpenRotation = door is rotated 90 degrees open */
    OpenRotation = FRotator(0.f, 90.f, 0.f);

    /* Initial door state; false = closed, true = open */
    bIsOpen = false;

    /* Enable replication so this actor's state can be seen on client and server. */
    bReplicates = true;
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
    DoorMesh->SetRelativeRotation(FMath::RInterpTo(DoorMesh->GetRelativeRotation(),TargetRot,GetWorld()->GetDeltaSeconds(),OpenSpeed));

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
#include "MyBaseMovementComponent.h"
#include "GameFramework/Character.h"

UMyBaseMovementComponent::UMyBaseMovementComponent()
{
    // Enable crouching for this movement component.
    // This allows the character to crouch both in gameplay
    // and in navigation/AI pathfinding systems.
    NavAgentProps.bCanCrouch = true;

    // Define the capsule’s half-height when crouched.
    // Determines how much the collision capsule shrinks while crouching.
    SetCrouchedHalfHeight(60.0f);

    // Whether the character wants to sprint.
    // Initialized to false so the character begins walking by default.
    Safe_bWantsToSprint = false;

    // Movement speed when sprinting.
    SprintSpeed = 500.0f;

    // Movement speed when walking normally (not sprinting).
    WalkSpeed = 250.0f;

    //  Movement speed when crouch walking
    MaxWalkSpeedCrouched = 250.0f;
}


FNetworkPredictionData_Client* UMyBaseMovementComponent::GetPredictionData_Client() const
{
    // Ensure the movement component has a valid owning pawn.
    // Prediction data is always tied to a specific character/pawn,
    // so this must not be null at this point.
    check(PawnOwner != nullptr);

    // If this client does not yet have prediction data allocated,
    // we need to create our own custom version of it.
    if (ClientPredictionData == nullptr)
    {
        // We need a non-const pointer to assign ClientPredictionData,
        // so temporarily cast away constness.
        UMyBaseMovementComponent* MutableThis = const_cast<UMyBaseMovementComponent*>(this);

        // Allocate our custom prediction data type.
        // This is where we plug in our own class (FNetworkPredictionData_Client_MyData)
        // instead of Unreal’s default, so we can support sprinting or other custom inputs.
        MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_MyData(*this);

        // Configure network smoothing distances:
        // - MaxSmoothNetUpdateDist: maximum distance the character can be corrected smoothly.
        //   Beyond this, the correction will cause a hard snap.
        // - NoSmoothNetUpdateDist: distance beyond which no smoothing is attempted at all.
        MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
        MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
    }

    // Return the (cached or newly created) client prediction data object.
    return ClientPredictionData;
}


void UMyBaseMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    // First, call the base implementation to handle built-in flags.
    Super::UpdateFromCompressedFlags(Flags);

    // Our custom sprint flag is stored in FLAG_Custom_0.
    // Safe_bWantsToSprint is updated from the replicated bit.
    Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UMyBaseMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
    // Always call the parent class implementation first
    // so that the engine's default movement logic still executes.
    // This ensures we don't break built-in behavior such as gravity,
    // floor detection, or collision handling.
    Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

    // Only apply sprinting/walking logic when the character is in the Walking movement mode.
    if (MovementMode == MOVE_Walking)
    {
        // If the character has flagged that it wants to sprint,
        // raise the maximum walking speed to the SprintSpeed value.
        if (Safe_bWantsToSprint)
        {
            MaxWalkSpeed = SprintSpeed;
        }
        // Otherwise, default to the standard walking speed.
        else
        {
            MaxWalkSpeed = WalkSpeed;
        }
    }
}

bool UMyBaseMovementComponent::FSavedMove_MyMove::CanCombineWith(const FSavedMovePtr& NewMove,ACharacter* InCharacter, float MaxDelta) const
{
    // Cast the generic FSavedMovePtr into our custom FSavedMove_MyMove type
    // so that we can access the sprint flag we added.
    const FSavedMove_MyMove* NewCharMove = static_cast<FSavedMove_MyMove*>(NewMove.Get());

    // Check if the sprinting state differs between this saved move and the new one.
    // If one move was made while sprinting and the other was not,
    // they cannot be combined because they represent distinct player input states.
    if (Saved_bWantsToSprint != NewCharMove->Saved_bWantsToSprint)
    {
        return false; // Keep them separate to preserve correct movement history.
    }

    // If the sprint states match, fall back to the default parent implementation.
    // The base logic will check other criteria (like acceleration, rotation, etc.)
    // to decide whether the two moves can be combined.
    return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

// Resets saved move values to defaults.
void UMyBaseMovementComponent::FSavedMove_MyMove::Clear()
{
    FSavedMove_Character::Clear();

    /* Reset the flag. */
    Saved_bWantsToSprint = 0;
}

// Packs our sprint state into the compressed flag byte.
uint8 UMyBaseMovementComponent::FSavedMove_MyMove::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();

    // If we are sprinting
    if (Saved_bWantsToSprint)
        // Use the first custom flag slot
        Result |= FLAG_Custom_0;

    return Result;
}

// Saves data for this move (called on client when recording a move).
void UMyBaseMovementComponent::FSavedMove_MyMove::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
    // Call parent to capture normal movement inputs.
    FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

    // Cast to our custom movement component to access sprint flag.
    UMyBaseMovementComponent* CharacterMovement = Cast<UMyBaseMovementComponent>(C->GetCharacterMovement());

    // Save whether sprinting was active for this move.
    Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
}

// Applies this saved move back onto the character (used during replay/resimulation).
void UMyBaseMovementComponent::FSavedMove_MyMove::PrepMoveFor(ACharacter* C)
{
    Super::PrepMoveFor(C);

    // Cast to our custom movement component.
    UMyBaseMovementComponent* CharacterMovement = Cast<UMyBaseMovementComponent>(C->GetCharacterMovement());

    // Restore sprint flag for resimulation.
    CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
}

// Constructor — just forwards to parent class.
UMyBaseMovementComponent::FNetworkPredictionData_Client_MyData::FNetworkPredictionData_Client_MyData(const UCharacterMovementComponent& ClientMovement)
    : Super(ClientMovement)
{
}

// Allocate a new instance of our custom saved move class.
// This is used by the network prediction system to record
// and replay client movement input for reconciliation.
FSavedMovePtr UMyBaseMovementComponent::FNetworkPredictionData_Client_MyData::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_MyMove);
}

// Called when the player starts sprinting.
// Sets the sprint flag so that our movement code knows to apply sprint speed.
// Also forces crouching off, since sprinting and crouching are mutually exclusive.
void UMyBaseMovementComponent::StartSprinting()
{
    Safe_bWantsToSprint = true;

    /* Cancel crouch state when sprinting begins. */
    bWantsToCrouch = false;
}

// Called when the player stops sprinting.
// Clears the sprint flag so that walking speed is restored.
void UMyBaseMovementComponent::StopSprinting()
{
    Safe_bWantsToSprint = false;
}

// Returns whether the character is currently sprinting.
// This wraps the sprint flag for convenient checks in other systems.
bool UMyBaseMovementComponent::IsSprinting() const
{
    return Safe_bWantsToSprint;
}

// Called when the player starts crouching.
// Sets the crouch flag, which is handled by CharacterMovementComponent.
// Also disables sprinting, since crouching and sprinting cannot happen at the same time.
void UMyBaseMovementComponent::StartCrouching()
{
    bWantsToCrouch = true;

    /* Cancel sprint state when crouching begins. */
    Safe_bWantsToSprint = false;
}

// Called when the player stops crouching.
// Clears the crouch flag, returning the character to standing state.
void UMyBaseMovementComponent::StopCrouching()
{
    bWantsToCrouch = false;
}
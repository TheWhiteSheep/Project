#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "MyBaseMovementComponent.generated.h"

class ACharacter;

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_API UMyBaseMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

    // A saved move is a "snapshot" of movement state for a single frame.
    // On the client: multiple saved moves are queued and later sent to the server.
    // On the server: these moves are replayed to reproduce client movement exactly.
    // By subclassing FSavedMove_Character, we can add extra flags (e.g., sprinting).
    class FSavedMove_MyMove : public FSavedMove_Character {

        typedef FSavedMove_Character Super;

        /** Whether the player wanted to sprint at this frame (1-bit flag). */
        uint8 Saved_bWantsToSprint : 1;

        /**
         * Determines if two saved moves can be combined into one.
         * Used for network efficiency — if two moves are identical (same inputs/flags),
         * they don’t need to be sent separately.
         */
        virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

        /** Resets this saved move back to its default state. */
        virtual void Clear() override;

        /**
         * Returns compressed flags representing this move’s state.
         * Unreal packs common actions (jumping, crouching, etc.) into a byte,
         * and here we extend that to include sprinting.
         */
        virtual uint8 GetCompressedFlags() const override;

        /**
         * Records data for this move (called when move is created on the client).
         * Saves acceleration, delta time, and custom flags such as sprinting.
         */
        virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

        /**
         * Prepares the character for re-simulating this move (on server or client).
         * Applies saved custom state (e.g., whether sprint was pressed).
         */
        virtual void PrepMoveFor(ACharacter* C) override;
    };


    // Stores client-side prediction data.
    // Responsible for allocating new saved moves of our custom type.
    class FNetworkPredictionData_Client_MyData : public FNetworkPredictionData_Client_Character
    {
    public:
        /** Constructor: forwards to parent constructor. */
        FNetworkPredictionData_Client_MyData(const UCharacterMovementComponent& ClientMovement);

        typedef FNetworkPredictionData_Client_Character Super;

        /** Allocates a new FSavedMove_MyMove (instead of default FSavedMove_Character). */
        virtual FSavedMovePtr AllocateNewMove() override;
    };

    /** Runtime flag — true if the player wants to sprint (safe for client/server use). */
    bool Safe_bWantsToSprint;

    /** Max speed while sprinting. */
    UPROPERTY(EditDefaultsOnly)
    float SprintSpeed;

    /** Max speed while walking. */
    UPROPERTY(EditDefaultsOnly)
    float WalkSpeed;

public:
    /** Default constructor — sets initial values. */
    UMyBaseMovementComponent();

    /**
     * Returns our custom client prediction data object.
     * This is what tells Unreal to use FNetworkPredictionData_Client_MyData and allocate FSavedMove_MyMove for saving moves.
     */
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
    /**
     * Updates state based on compressed flags received from the client.
     * Called on the server to apply sprinting/jumping/etc. from the client’s input.
     */
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;

    /**
     * Called every time movement is updated.
     * Useful for applying movement logic
     */
    virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

public:
    /** Activates sprinting (sets the flag so saved moves will capture it). */
    void StartSprinting();

    /** Deactivates sprinting (resets the flag). */
    void StopSprinting();

    /* Returns the Safe_bWantsToSprint flag */
    bool IsSprinting() const;

    /** Activates crouching (sets the flag so saved moves will capture it). */
    void StartCrouching();

    /** Deactivates crouching (resets the flag). */
    void StopCrouching();
};
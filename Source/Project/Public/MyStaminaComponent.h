

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyBaseMovementComponent.h"
#include "MyStaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_API UMyStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyStaminaComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

    /**
     * Current stamina value. Replicates to clients whenever it changes via OnRep_CurrentStamina.
     */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina)
    float CurrentStamina;

    /**
     * Maximum stamina value. Replicated to clients.
     */
    UPROPERTY(Replicated)
    float MaximumStamina;

    /**
     * Indicates whether the player currently has any stamina left.
     */
    UPROPERTY()
    bool bHasStamina;

    /**
     * Indicates whether the player can sprint based on current stamina.
     */
    UPROPERTY()
    bool bCanSprint;

    /**
     * Called when CurrentStamina is replicated to update clients.
     */
    UFUNCTION()
    void OnRep_CurrentStamina();

    /**
    * The time it takes for the stamina to decrease/increase.
    */
    UPROPERTY(EditDefaultsOnly, Category = "Stamina|Timer")
    float RegenTime;

    /**
    * The DrainAmount is how much Stamina will decrease after the RegenTime float value.
    */
    UPROPERTY(EditDefaultsOnly, Category = "Stamina|Timer")
    float DrainAmount;

    /**
    * The FillAmount is how much Stamina will be increased after the RegenTime float value.
    */
    UPROPERTY(EditDefaultsOnly, Category = "Stamina|Timer")
    float FillAmount;

public:

    /**
     * Event triggered whenever stamina changes. Can be bound in Blueprint.
     */
    UPROPERTY(BlueprintAssignable, Category = "Stamina|Event")
    FOnStaminaChanged OnStaminaChanged;

    /**
     * Returns the current stamina value.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina|Query")
    float GetCurrentStamina() const;

    /**
     * Returns the maximum stamina value.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina|Query")
    float GetMaximumStamina() const;

    /**
     * Returns true if the player has any stamina left.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina|Query")
    bool HasStamina() const;

    /**
     * Returns true if the player can currently sprint.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina|Query")
    bool CanSprint() const;

    /**
     * Returns true if the player's stamina is full.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina|Query")
    bool HasFullStamina() const;

    /**
     * Increases the current stamina by Amount on the server.
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stamina|Modifier")
    void ServerIncreaseCurrentStamina(float Amount);

    /**
     * Decreases the current stamina by Amount on the server.
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stamina|Modifier")
    void ServerDecreaseCurrentStamina(float Amount);

    /**
     * Sets the current stamina to a specific Amount on the server.
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stamina|Modifier")
    void ServerSetCurrentStamina(float Amount);

    /**
     * Increases the maximum stamina by Amount on the server.
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stamina|Modifier")
    void ServerIncreaseMaximumStamina(float Amount);

    /**
     * Decreases the maximum stamina by Amount on the server.
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stamina|Modifier")
    void ServerDecreaseMaximumStamina(float Amount);

    /**
     * Sets the maximum stamina to a specific Amount on the server.
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stamina|Modifier")
    void ServerSetMaximumStamina(float Amount);

    /**
     * Returns the current stamina as a percentage of maximum stamina.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina|UI")
    float GetStaminaPercentage() const;

    /**
     * Updates bHasStamina and bCanSprint based on current stamina values.
     */
    UFUNCTION(BlueprintCallable, Category = "Stamina|Status")
    void UpdateStaminaStatus();

    /**
     * Timer handle used to repeatedly call StaminaTick() for stamina manipulation.
     */
    FTimerHandle StaminaDrainTimer;

    /**
     * Starts the stamina manipulation timer (draining or regenerating stamina).
     */
    void StartStaminaManipulation();

    /**
     * Stops the stamina manipulation timer.
     */
    void StopStaminaManipulation();

    /**
     * Tick function called by the timer to handle stamina drain/regeneration.
     */
    void StaminaTick();

    /**
     * Returns true if the stamina manipulation timer is currently active.
     */
    bool IsStaminaTimerActive();

    /**
     * Cached pointer to the owning character to avoid repeated casts.
     */
    ACharacter* OwnerCharacter;

    /**
     * Cached pointer to the movement component for sprint checks.
     */
    UMyBaseMovementComponent* MyMovementComponent;
};

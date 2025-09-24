// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_API UMyHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyHealthComponent();

private:
	UPROPERTY(Replicated)
	float BaseCurrentHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(Replicated)
	float CurrentMaximumHealth;

	UPROPERTY(Replicated)
	bool bIsActorDead;

	UPROPERTY(Replicated)
	bool bIsActorHealable;

	UFUNCTION()
	void OnRep_CurrentHealth();

public:

	/*=========================== Delegates ===============================*/

	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FOnHealthChanged OnHealthChanged;

	/*=========================== Queries ===============================*/

	/**
	 * Gets the current base health value of the actor.
	 *
	 * @return The current base health as a float.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	float GetBaseCurrentHealth() const;

	/**
	 * Gets the actor's current health.
	 *
	 * Unlike GetBaseCurrentHealth, this reflects the actual health value
	 * after damage, healing, or other modifications.
	 *
	 * @return The current health as a float.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	float GetCurrentHealth() const;


	/**
	 * Gets the actor's current maximum health.
	 *
	 * This affects the maximum health if modifiers such as buffs,
	 * debuffs, equipment, or effects are applied to the actor.
	 *
	 * @return The current maximum health as a float.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	float GetCurrentMaximumHealth() const;


	/**
	 * Checks whether the actor is considered dead.
	 *
	 * An actor is dead if their current health is less than or equal to zero.
	 *
	 * @return True if the actor's current health is zero or below, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	bool IsActorDead() const;

	/**
	 * Checks whether the actor is considered alive.
	 *
	 * An actor is alive if their current health is greater than zero.
	 *
	 * @return True if the actor's current health is above zero, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	bool IsActorAlive() const;


	/**
	 * Checks whether the actor can be healed.
	 *
	 * This depends on the internal bIsActorHealable flag, which may be
	 * controlled by game logic such as effects, states, or conditions.
	 *
	 * @return True if the actor can be healed, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	bool IsActorHealable() const;

	/**
	 * Checks whether the actor is at full health.
	 *
	 * An actor is considered full health if its current health
	 * is exactly equal to its current maximum health.
	 *
	 * @return True if the actor is at full health, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	bool IsActorFullHealth() const;

	/**
	 * Checks if the actor's current health percentage is below or equal to a given threshold.
	 *
	 * The health percentage is calculated as CurrentHealth / CurrentMaximumHealth.
	 * Threshold should be provided as a normalized value between 0.0 and 1.0.
	 *
	 * @param Threshold The health ratio to compare against (0.0�1.0).
	 * @return True if the current health percentage is less than or equal to the threshold, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	bool IsActorBelowHealthPercentage(float Threshold) const;

	/**
	 * Checks if the actor's current health percentage is above or equal to a given threshold.
	 *
	 * The health percentage is calculated as CurrentHealth / CurrentMaximumHealth.
	 * Threshold should be provided as a normalized value between 0.0 and 1.0.
	 *
	 * @param Threshold The health ratio to compare against (0�1).
	 * @return True if the current health percentage is greater than or equal to the threshold, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|Query")
	bool IsActorAboveHealthPercentage(float Threshold) const;

	/*=========================== Modifiers ===============================*/

	/**
	 * Sets whether the actor is considered dead (server-authoritative).
	 * If marked as dead, CurrentHealth will be set to zero.
	 *
	 * @param bDead True to mark the actor as dead, false otherwise.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerSetActorDead(bool bDead);

	/**
	 * Sets whether the actor can currently be healed (server-authoritative).
	 *
	 * @param bHealable True if the actor can be healed, false otherwise.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerIsActorHealable(bool bHealable);


	/**
	 * Increases the actor's base current health by a specified amount (server-authoritative).
	 * If BaseCurrentHealth exceeds CurrentMaximumHealth, CurrentMaximumHealth is also increased.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount Amount to increase base current health by.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerIncreaseBaseCurrentHealth(float Amount);

	/**
	 * Increases the actor's current health by a specified amount (server-authoritative).
	 * Current health is clamped between 0 and CurrentMaximumHealth.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount Amount to increase current health by.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerIncreaseCurrentHealth(float Amount);

	/**
	 * Increases the actor's current maximum health by a specified amount (server-authoritative).
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount Amount to increase current maximum health by.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerIncreaseCurrentMaximumHealth(float Amount);

	/**
	 * Decreases the actor's base current health by a specified amount (server-authoritative).
	 * BaseCurrentHealth will not go below zero.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount Amount to decrease base current health by.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerDecreaseBaseCurrentHealth(float Amount);

	/**
	 * Decreases the actor's current health by a specified amount (server-authoritative).
	 * CurrentHealth is clamped between 0 and CurrentMaximumHealth.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount Amount to decrease current health by.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerDecreaseCurrentHealth(float Amount);

	/**
	 * Decreases the actor's current maximum health by a specified amount (server-authoritative).
	 * CurrentMaximumHealth will not go below zero, and CurrentHealth will be clamped to the new maximum.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount Amount to decrease current maximum health by.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerDecreaseCurrentMaximumHealth(float Amount);

	/**
	 * Sets the actor's base current health to a specified value (server-authoritative).
	 * Ignored if the actor cannot be healed or if Amount is zero or negative.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount New base current health value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerSetBaseCurrentHealth(float Amount);

	/**
	 * Sets the actor's current health to a specified value (server-authoritative).
	 * CurrentHealth is clamped between 0 and CurrentMaximumHealth.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount New current health value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerSetCurrentHealth(float Amount);

	/**
	 * Sets the actor's current maximum health to a specified value (server-authoritative).
	 * CurrentHealth is clamped to not exceed the new maximum health.
	 * Automatically calls UpdateHealthStatus() after the change.
	 *
	 * @param Amount New current maximum health value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health|Modifier")
	void ServerSetCurrentMaximumHealth(float Amount);

	/*============================= User interface =================================*/

	/**
	 * Returns the actor's current health as a normalized percentage (0.0�1.0).
	 * If CurrentMaximumHealth is zero or less, returns 0.0.
	 *
	 * @return Current health divided by maximum health.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|UI")
	float GetHealthPercentage() const;

	/**
	 * Returns the actor's base current health as a text value for UI display.
	 *
	 * @param bRound If true, rounds the value to the nearest integer.
	 * @return Base current health as FText.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|UI")
	FText GetBaseCurrentHealthText(bool bRound) const;

	/**
	 * Returns the actor's current health as a text value for UI display.
	 *
	 * @param bRound If true, rounds the value to the nearest integer.
	 * @return Current health as FText.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|UI")
	FText GetCurrentHealthText(bool bRound) const;

	/**
	 * Returns the actor's current maximum health as a text value for UI display.
	 *
	 * @param bRound If true, rounds the value to the nearest integer.
	 * @return Current maximum health as FText.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|UI")
	FText GetCurrentMaximumHealthText(bool bRound) const;

	/**
	 * Returns the actor's current health over maximum health as a formatted fraction for UI display.
	 *
	 * @param bRound If true, rounds the values to the nearest integer.
	 * @return Formatted FText in the form "Current / Maximum".
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health|UI")
	FText GetHealthFractionText(bool bRound) const;


	/*============================= Status =================================*/

	/**
	 * Updates the internal health status of the actor.
	 *
	 * This function checks whether the actor is dead and updates the bIsActorDead flag.
	 * It also broadcasts the OnHealthChanged event for any listeners (e.g., UI, gameplay systems).
	 *
	 * Typically called automatically by server-authoritative modifier functions after a health change.
	 * Can also be called manually if you modify health directly.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health|Status")
	void UpdateHealthStatus();
};

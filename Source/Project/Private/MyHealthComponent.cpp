// Fill out your copyright notice in the Description page of Project Settings.

#include "MyHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UMyHealthComponent::UMyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	BaseCurrentHealth = 100.0f;
	CurrentHealth = 100.0f;
	CurrentMaximumHealth = 100.0f;
	bIsActorDead = false;
	bIsActorHealable = true;
}

void UMyHealthComponent::OnRep_CurrentHealth()
{
	UpdateHealthStatus();
}

float UMyHealthComponent::GetBaseCurrentHealth() const
{
	return BaseCurrentHealth;
}

float UMyHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UMyHealthComponent::GetCurrentMaximumHealth() const
{
	return CurrentMaximumHealth;
}

bool UMyHealthComponent::IsActorDead() const
{
	return CurrentHealth <= 0.0f;
}

bool UMyHealthComponent::IsActorAlive() const
{
	return CurrentHealth > 0.0f;
}

bool UMyHealthComponent::IsActorHealable() const
{
	return bIsActorHealable;
}

bool UMyHealthComponent::IsActorFullHealth() const
{
	return CurrentHealth == CurrentMaximumHealth;
}

bool UMyHealthComponent::IsActorBelowHealthPercentage(float Threshold) const
{
	if (CurrentMaximumHealth <= 0.0f) { return false; }

	const float CurrentHealthPercentage = CurrentHealth / CurrentMaximumHealth;
	return CurrentHealthPercentage <= Threshold;
}

bool UMyHealthComponent::IsActorAboveHealthPercentage(float Threshold) const
{
	if (CurrentMaximumHealth <= 0.0f) { return false; }

	const float CurrentHealthPercentage = CurrentHealth / CurrentMaximumHealth;
	return CurrentHealthPercentage >= Threshold;
}

void UMyHealthComponent::ServerIsActorHealable_Implementation(bool bHealable)
{
	if (bIsActorHealable == bHealable) { return; }

	bIsActorHealable = bHealable;
}

void UMyHealthComponent::ServerSetActorDead_Implementation(bool bDead)
{
	if (bIsActorDead == bDead) return;

	bIsActorDead = bDead;

	if (bDead) { CurrentHealth = 0.0f; }

	UpdateHealthStatus();
}

void UMyHealthComponent::ServerIncreaseBaseCurrentHealth_Implementation(float Amount)
{
	if (!bIsActorHealable || BaseCurrentHealth <= 0.0f) { return; }

	BaseCurrentHealth += Amount;

	if (CurrentMaximumHealth < BaseCurrentHealth) {
		CurrentMaximumHealth = BaseCurrentHealth;
	}

	UpdateHealthStatus();
}

void UMyHealthComponent::ServerDecreaseBaseCurrentHealth_Implementation(float Amount)
{
	BaseCurrentHealth = FMath::Max(BaseCurrentHealth - Amount, 0.0f);

	UpdateHealthStatus();
}

void UMyHealthComponent::ServerSetBaseCurrentHealth_Implementation(float Amount)
{
	if (!bIsActorHealable || Amount <= 0.0f) { return; }

	BaseCurrentHealth = Amount;

	UpdateHealthStatus();
}

void UMyHealthComponent::ServerIncreaseCurrentHealth_Implementation(float Amount)
{
	if (!bIsActorHealable) { return; }

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, CurrentMaximumHealth);
	UpdateHealthStatus();
}

void UMyHealthComponent::ServerDecreaseCurrentHealth_Implementation(float Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, CurrentMaximumHealth);
	UpdateHealthStatus();
}

void UMyHealthComponent::ServerSetCurrentHealth_Implementation(float Amount)
{
	CurrentHealth = FMath::Clamp(Amount, 0.0f, CurrentMaximumHealth);
	UpdateHealthStatus();
}

void UMyHealthComponent::ServerIncreaseCurrentMaximumHealth_Implementation(float Amount)
{
	if (!bIsActorHealable) { return; }

	CurrentMaximumHealth += Amount;
	UpdateHealthStatus();
}

void UMyHealthComponent::ServerDecreaseCurrentMaximumHealth_Implementation(float Amount)
{
	CurrentMaximumHealth = FMath::Max(0.0f, CurrentMaximumHealth - Amount);
	CurrentHealth = FMath::Min(CurrentHealth, CurrentMaximumHealth);

	UpdateHealthStatus();
}

void UMyHealthComponent::ServerSetCurrentMaximumHealth_Implementation(float Amount)
{
	CurrentMaximumHealth = Amount;
	CurrentHealth = FMath::Min(CurrentHealth, CurrentMaximumHealth);

	UpdateHealthStatus();
}

void UMyHealthComponent::UpdateHealthStatus()
{
	bIsActorDead = IsActorDead();

	OnHealthChanged.Broadcast();
}

float UMyHealthComponent::GetHealthPercentage() const
{
	if (CurrentMaximumHealth <= 0.0f) { return 0.0f; }

	return CurrentHealth / CurrentMaximumHealth;
}

FText UMyHealthComponent::GetBaseCurrentHealthText(bool bRound) const
{
	if (bRound) {
		return FText::AsNumber(FMath::RoundToInt(BaseCurrentHealth));
	}

	return FText::AsNumber(BaseCurrentHealth);
}

FText UMyHealthComponent::GetCurrentHealthText(bool bRound) const
{
	if (bRound) {
		return FText::AsNumber(FMath::RoundToInt(CurrentHealth));
	}

	return FText::AsNumber(CurrentHealth);
}

FText UMyHealthComponent::GetCurrentMaximumHealthText(bool bRound) const
{
	if (bRound) {
		return FText::AsNumber(FMath::RoundToInt(CurrentMaximumHealth));
	}

	return FText::AsNumber(CurrentMaximumHealth);
}

FText UMyHealthComponent::GetHealthFractionText(bool bRound) const
{
	FText CurrentText = bRound ? FText::AsNumber(FMath::RoundToInt(CurrentHealth)) : FText::AsNumber(CurrentHealth);
	FText MaxText = bRound ? FText::AsNumber(FMath::RoundToInt(CurrentMaximumHealth)) : FText::AsNumber(CurrentMaximumHealth);

	return FText::Format(NSLOCTEXT("Health", "HealthFraction", "{0} / {1}"), CurrentText, MaxText);
}

void UMyHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyHealthComponent, BaseCurrentHealth);
	DOREPLIFETIME(UMyHealthComponent, CurrentHealth);
	DOREPLIFETIME(UMyHealthComponent, CurrentMaximumHealth);
	DOREPLIFETIME(UMyHealthComponent, bIsActorDead);
	DOREPLIFETIME(UMyHealthComponent, bIsActorHealable);
}
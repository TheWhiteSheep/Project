


#include "MyStaminaComponent.h"
#include "MyBaseMovementComponent.h"
#include <Net/UnrealNetwork.h>
#include <MyBaseCharacter.h>


// Sets default values for this component's properties
UMyStaminaComponent::UMyStaminaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentStamina = 100.0f;
	MaximumStamina = 100.0f;
	bCanSprint = true;
	bHasStamina = true;

	RegenTime = 1.0f;

    StaminaRegenDuration = 120.0f;
    StaminaRegenRate = MaximumStamina / StaminaRegenDuration;
    StaminaDrainDuration = 120.0f;
    StaminaDrainRate = MaximumStamina / StaminaDrainDuration;
}


// Called when the game starts
void UMyStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* OwnerCharacterTemp = Cast<ACharacter>(GetOwner()))
	{
		OwnerCharacter = OwnerCharacterTemp;
		MyMovementComponent = Cast<UMyBaseMovementComponent>(OwnerCharacter->GetMovementComponent());
	}	
}

void UMyStaminaComponent::OnRep_CurrentStamina()
{
    /** Update local stamina status whenever it is replicated to clients */
    UpdateStaminaStatus();
}

float UMyStaminaComponent::GetCurrentStamina() const
{
    /** Simply return the current stamina value */
    return CurrentStamina;
}

float UMyStaminaComponent::GetMaximumStamina() const
{
    /** Return the maximum stamina value */
    return MaximumStamina;
}

bool UMyStaminaComponent::HasFullStamina() const
{
    /** Check if current stamina is equal to or exceeds maximum stamina */
    return CurrentStamina >= MaximumStamina;
}

bool UMyStaminaComponent::HasStamina() const
{
    /** Check if the player has any stamina remaining */
    return CurrentStamina > 0.0f;
}

bool UMyStaminaComponent::CanSprint() const
{
    /** Determine if the player has enough stamina to sprint (threshold = 5) */
    return CurrentStamina >= 5.0f;
}

void UMyStaminaComponent::ServerIncreaseCurrentStamina_Implementation(float Amount)
{
    /** Increase the current stamina by the specified amount */
    CurrentStamina += Amount;

    /** Clamp stamina at MaximumStamina and stop the timer if full */
    if (CurrentStamina >= MaximumStamina) {
        CurrentStamina = MaximumStamina;
        StopRegeneration();
    }

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerDecreaseCurrentStamina_Implementation(float Amount)
{
    /** Decrease the current stamina by the specified amount */
    CurrentStamina -= Amount;

    /** Clamp stamina at zero and prevent sprinting if depleted */
    if (CurrentStamina <= 0.0f) {
        CurrentStamina = 0.0f;
        bCanSprint = false;
    }

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerSetCurrentStamina_Implementation(float Amount)
{
    /** Set the current stamina to the specified amount */
    CurrentStamina = Amount;

    /** Clamp stamina at MaximumStamina */
    if (CurrentStamina >= MaximumStamina) {
        CurrentStamina = MaximumStamina;
    }

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerSetMaximumStamina_Implementation(float Amount)
{
    /** Ignore invalid maximum stamina values */
    if (MaximumStamina <= 0.0f) { return; }

    /** Set maximum stamina */
    MaximumStamina = Amount;

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerDecreaseMaximumStamina_Implementation(float Amount)
{
    /** Ignore invalid maximum stamina values */
    if (MaximumStamina <= 0.0f) { return; }

    /** Decrease the maximum stamina by the specified amount */
    MaximumStamina -= Amount;

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerIncreaseMaximumStamina_Implementation(float Amount)
{
    /** Increase the maximum stamina by the specified amount */
    MaximumStamina += Amount;

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

float UMyStaminaComponent::GetStaminaPercentage() const
{
    /** Avoid division by zero */
    if (MaximumStamina <= 0.0f) { return 0.0f; }

    /** Return the current stamina as a percentage of the maximum */
    return CurrentStamina / MaximumStamina;
}

void UMyStaminaComponent::UpdateStaminaStatus()
{
    /** Update flags based on current stamina */
    bHasStamina = HasStamina();
    bCanSprint = CanSprint();

    /** Broadcast an event so UI or other systems can react */
    OnStaminaChanged.Broadcast(CurrentStamina, MaximumStamina);
}

void UMyStaminaComponent::StartRegeneration()
{
    // Stop any active draining
    StopDraining();

    if (!GetWorld()) return;

    // Clear any existing regeneration timer
    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);

    // Timer interval for smooth updates (~60Hz)
    const float Interval = 1.0f / 60.0f;

    // Start looping timer
    GetWorld()->GetTimerManager().SetTimer(
        RegenTimerHandle,
        this,
        &UMyStaminaComponent::UpdateStaminaTick,
        Interval,
        true
    );

    bIsRegenerating = true;
}

void UMyStaminaComponent::StopRegeneration()
{
    if (!GetWorld()) return;

    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
    bIsRegenerating = false;
}

void UMyStaminaComponent::StartDraining()
{
    StopRegeneration();

    if (!GetWorld()) return;

    GetWorld()->GetTimerManager().ClearTimer(DrainTimerHandle);

    const float Interval = 1.0f / 60.0f;

    GetWorld()->GetTimerManager().SetTimer(
        DrainTimerHandle,
        this,
        &UMyStaminaComponent::UpdateStaminaTick,
        Interval,
        true
    );

    bIsRegenerating = false;
}

void UMyStaminaComponent::StopDraining()
{
    if (!GetWorld()) return;

    GetWorld()->GetTimerManager().ClearTimer(DrainTimerHandle);
    bIsRegenerating = true;
}

void UMyStaminaComponent::UpdateStaminaTick()
{
    if (!GetWorld()) return;

    const float DeltaTime = 1.0f / 60.0f;

    if (!bIsRegenerating)
    {
        ConsumeStamina(DeltaTime);

        if (CurrentStamina <= 0.0f)
        {
            CurrentStamina = 0.0f;
            StopDraining();
        }
    }
    else
    {
        Regenerate(DeltaTime);

        if (CurrentStamina >= MaximumStamina)
        {
            CurrentStamina = MaximumStamina;
            StopRegeneration();
        }
    }

    UpdateStaminaStatus();
}

void UMyStaminaComponent::Regenerate(float DeltaTime)
{
    if (CurrentStamina >= MaximumStamina) return;

    CurrentStamina = FMath::Min(CurrentStamina + StaminaRegenRate * DeltaTime, MaximumStamina);
}

void UMyStaminaComponent::ConsumeStamina(float DeltaTime)
{
    if (CurrentStamina <= 0.0f) return;

    CurrentStamina = FMath::Max(CurrentStamina - StaminaDrainRate * DeltaTime, 0.0f);
}

void UMyStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyStaminaComponent, CurrentStamina);
	DOREPLIFETIME(UMyStaminaComponent, MaximumStamina);
}
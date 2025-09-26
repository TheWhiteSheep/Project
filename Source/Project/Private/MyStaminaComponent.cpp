


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
    /** Only execute on the server */
    if (!GetOwner()->HasAuthority()) { return; }

    /** Increase the current stamina by the specified amount */
    CurrentStamina += Amount;

    /** Clamp stamina at MaximumStamina and stop the timer if full */
    if (CurrentStamina >= MaximumStamina) {
        CurrentStamina = MaximumStamina;
        StopStaminaManipulation();
    }

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerDecreaseCurrentStamina_Implementation(float Amount)
{
    /** Only execute on the server */
    if (!GetOwner()->HasAuthority()) { return; }

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
    /** Only execute on the server */
    if (!GetOwner()->HasAuthority()) { return; }

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
    /** Only execute on the server */
    if (!GetOwner()->HasAuthority()) { return; }

    /** Ignore invalid maximum stamina values */
    if (MaximumStamina <= 0.0f) { return; }

    /** Set maximum stamina */
    MaximumStamina = Amount;

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerDecreaseMaximumStamina_Implementation(float Amount)
{
    /** Only execute on the server */
    if (!GetOwner()->HasAuthority()) { return; }

    /** Ignore invalid maximum stamina values */
    if (MaximumStamina <= 0.0f) { return; }

    /** Decrease the maximum stamina by the specified amount */
    MaximumStamina -= Amount;

    /** Update internal flags and broadcast changes */
    UpdateStaminaStatus();
}

void UMyStaminaComponent::ServerIncreaseMaximumStamina_Implementation(float Amount)
{
    /** Only execute on the server */
    if (!GetOwner()->HasAuthority()) { return; }

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
    OnStaminaChanged.Broadcast();
}

void UMyStaminaComponent::StartStaminaManipulation()
{
    /** Only start the timer if the world exists */
    if (GetWorld()) {
        /** Set a recurring timer that calls StaminaTick every RegenTime seconds */
        GetWorld()->GetTimerManager().SetTimer(
            StaminaDrainTimer,
            this,
            &UMyStaminaComponent::StaminaTick,
            RegenTime,
            true
        );
    }
}

void UMyStaminaComponent::StopStaminaManipulation()
{
    /** Only stop the timer if the world exists */
    if (GetWorld()) {
        /** Clear the recurring stamina timer */
        GetWorld()->GetTimerManager().ClearTimer(StaminaDrainTimer);
    }
}

void UMyStaminaComponent::StaminaTick()
{
	/**
	* If OwnerCharacter and MyMovementComponent is invalid then break function.
	*/
	if (!OwnerCharacter || !MyMovementComponent) return;

	/** Cache booleans to avoid repeated function calls */
	const bool bIsSprinting = MyMovementComponent->IsSprinting();
	const bool bHasStamina = HasStamina();
	const bool bHasFullStamina = HasFullStamina();

	/**
	 * Stop sprinting if the player is currently sprinting but out of stamina.
	 * This prevents the player from continuing to sprint when they have no stamina left.
	 */
	if (bIsSprinting && !bHasStamina)
	{
		if (AMyBaseCharacter* MyChar = Cast<AMyBaseCharacter>(OwnerCharacter))
		{
			MyChar->StopSprinting();
		}
	}
	/**
	 * If the player is sprinting and has stamina, drain stamina.
	 */
	else if (bIsSprinting && bHasStamina)
	{
		ServerDecreaseCurrentStamina(1.0f);
	}
	/**
	 * If the player is not sprinting and stamina is not full, regenerate stamina.
	 * This ensures stamina gradually recovers when the player is resting.
	 */
	else if (!bHasFullStamina)
	{
		ServerIncreaseCurrentStamina(1.0f);
	}
}

bool UMyStaminaComponent::IsStaminaTimerActive()
{
	return GetWorld()->GetTimerManager().IsTimerActive(StaminaDrainTimer);
}

void UMyStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyStaminaComponent, CurrentStamina);
	DOREPLIFETIME(UMyStaminaComponent, MaximumStamina);
}
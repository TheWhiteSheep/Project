// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCameraManager.h"
#include "MyBaseCharacter.h"
#include "MyBaseMovementComponent.h"
#include "Components/CapsuleComponent.h"


AMyCameraManager::AMyCameraManager()
{
	CrouchBlendDuration = 0.1f;
}

void AMyCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (AMyBaseCharacter* MyCharacter = Cast<AMyBaseCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		UMyBaseMovementComponent* CharacterMovementComponent = MyCharacter->GetMyBaseMovementComponent();
		FVector TargetCrouchOffset = FVector(0, 0, CharacterMovementComponent->GetCrouchedHalfHeight() - MyCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.0f, 1.0f));

		if (CharacterMovementComponent->IsCrouching())
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}

		OutVT.POV.Location += Offset;
	}
}
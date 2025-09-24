// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MyCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API AMyCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) 
	float CrouchBlendDuration;
	
	float CrouchBlendTime;

public:
	AMyCameraManager();

	UFUNCTION()
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
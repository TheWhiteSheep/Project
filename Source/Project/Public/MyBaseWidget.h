// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/ProgressBar.h>
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "MyBaseWidget.generated.h"

/**
 * UMyBaseWidget
 *
 * This class represents a basic health bar widget for the player.
 * It is derived from UUserWidget and is designed to be created in Blueprints.
 *
 * Responsibilities:
 * - Update the health bar percentage when the player's health changes.
 * - Optionally change the health bar color based on percentage thresholds.
 * - Respond to health changes via a delegate binding.
 */
UCLASS()
class PROJECT_API UMyBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Updates the visual fill of the health bar.
	 *
	 * @param Percent - The health percentage (0.0 to 1.0) to set the bar's fill amount.
	 *                  Typically obtained from the owning health component.
	 *                  Can also be used to dynamically change the color of the bar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void UpdateHealthBar(float Percent);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void UpdateStaminaBar(float Percent);

	/**
	 * Handler function bound to the OnHealthChanged delegate of UMyHealthComponent.
	 *
	 * This function is called whenever the owning character's health changes.
	 * It fetches the latest health percentage and updates the health bar accordingly.
	 *
	 * This allows the widget to automatically reflect health changes in real-time.
	 */
	UFUNCTION()
	void OnHealthChangedHandler();

	UFUNCTION()
	void OnStaminaChangedHandler();

protected:

	/**
	 * HealthBar
	 *
	 * A UProgressBar created in the Blueprint version of this widget.
	 * The "BindWidget" specifier automatically binds this variable to a ProgressBar
	 * in the UMG Designer that has the same name (HealthBar).
	 *
	 * This is the UI element that visually displays the player's current health.
	 */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;
};

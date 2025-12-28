// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBaseWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

/**
 * UpdateHealthBar
 *
 * Updates the progress bar's fill and color based on the player's health percentage.
 *
 * @param Percent - The current health percentage (0.0 to 1.0)
 */
void UMyBaseWidget::UpdateHealthBar(float Percent)
{
    // Set the progress bar's fill amount to match the current health
    HealthBar->SetPercent(Percent);

    // Determine the color of the health bar based on health thresholds
    FLinearColor NewColor = FLinearColor::Green; // Default color = green (high health)

    if (Percent <= 0.25f) // If health is very low
    {
        NewColor = FLinearColor::Red; // Red indicates critical health
    }
    else if (Percent <= 0.50f) // If health is low but not critical
    {
        NewColor = FLinearColor(1.0f, 0.65f, 0.0f, 1.0f); // Orange color for low health
    }
    else if (Percent <= 0.7f) // If health is moderately low
    {
        NewColor = FLinearColor::Yellow; // Yellow color for medium health
    }

    // Apply the new color to the progress bar's fill
    HealthBar->SetFillColorAndOpacity(NewColor);
}

void UMyBaseWidget::UpdateStaminaBar(float Percent)
{
    StaminaBar->SetPercent(Percent);
}

void UMyBaseWidget::OnHealthChangedHandler(float CurrentHealth, float CurrentMaximumHealth)
{
    // Make sure the HealthBar is valid before attempting to modify it
    if (!IsValid(HealthBar)) return;

    // Get the owning player of this widget
    if (APlayerController* PC = GetOwningPlayer())
    {
        // Get the pawn (character) controlled by this player
        if (APawn* Pawn = PC->GetPawn())
        {
            UpdateHealthBar(CurrentHealth / CurrentMaximumHealth);
        }
    }
}

void UMyBaseWidget::OnStaminaChangedHandler(float CurrentStamina, float MaximumStamina)
{
    if (!IsValid(StaminaBar)) return;

    if (APlayerController* PC = GetOwningPlayer()) 
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            UpdateStaminaBar(CurrentStamina/MaximumStamina);
            UE_LOG(LogTemp, Log, TEXT("StaminaBar was updated."));
        }
    }
}
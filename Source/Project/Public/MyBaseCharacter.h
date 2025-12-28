#pragma once

#include "CoreMinimal.h"
#include "MyHealthComponent.h"
#include <MyStaminaComponent.h>
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MyBaseWidget.h"  
#include "MyBaseCharacter.generated.h"

class UMyBaseMovementComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;

UCLASS()
class PROJECT_API AMyBaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMyBaseCharacter(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void BeginPlay() override;

    /*This mesh will be visible to others on the server.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* ServerMesh;

    /*This mesh will only be visible to you in first and third person.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* ClientMesh;

    /*The Camera Arm that will be used when in FirstPerson View or Perspective.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FPSpringArm;

    /*The Camera that will be used when in FirstPerson View or Perspective.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FPCamera;

    /*The Camera Arm that will be used when in ThirdPerson View or Perspective.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* TPSpringArm;

    /*The Camera that will be used when in ThirdPerson View or Perspective.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* TPCamera;

    /** Custom Movement Component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
    UMyBaseMovementComponent* MyMovement;

    /** Input Actions */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* ChangePerspectiveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* InteractAction;

    // Health component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    UMyHealthComponent* MyHealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    UMyStaminaComponent* MyStaminaComponent;

    // Your widget class to spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UMyBaseWidget> WidgetClass;

    // The instance of the widget
    UPROPERTY()
    UMyBaseWidget* WidgetInstance;


public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    /* Returns a pointer to this character's custom movement component for direct access to movement functions and properties. */
    UMyBaseMovementComponent* GetMyBaseMovementComponent();

    /* The distance at which the character interacts with the object in first person. 
    When in ThirdPerson it also adds on the CameraDistance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractDistance = 120.f;

    /* Starts sprinting (tells server we want to sprinting). */
    void StartSprinting();
    /* Stops sprinting (tells server we want to stop sprinting). */
    void StopSprinting();
    /* Starts crouching (tells server we want to crouch). */
    void StartCrouching();
    /* Stops crouching (tells server we want to uncrouch). */
    void StopCrouching();
    /* Toggles between perspectives:
    Calls ActivateFirstPerson or ActivateThirdPerson based on the bIsThirdPerson boolean*/
    void ToggleView();
    /*Toggles the first person perspective for the player and changes cameras.*/
    void ActivateFirstPerson();
    /*Toggles the ThirdPerson perspective for the player and chagnes cameras.*/
    void ActivateThirdPerson();
    /* Tracks whether the player is in third-person view (client-side only). */
	bool bIsThirdPerson = false;

protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    /* Server RPC: performs interaction with a target actor. */
    UFUNCTION(Server, Reliable)
    void Server_Interact(AActor* TargetActor);
    /* Client input handler: triggers interaction trace and calls server. */
    UFUNCTION()
    void OnInteract();
};
#pragma once

#include "CoreMinimal.h"
#include "MyHealthComponent.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
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

    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

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

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    /* Returns a pointer to this character's custom movement component for direct access to movement functions and properties. */
    UMyBaseMovementComponent* GetMyBaseMovementComponent();

    /* The distance at which the character interacts with the object in first person. 
    When in ThirdPerson it also adds on the CameraDistance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float BaseInteractDistance;
    
    /* The distance at which the camera sits away from the Character in ThirdPerson. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistance;


protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    /* Starts sprinting (tells server we want to sprinting). */
    void StartSprinting();
    /* Stops sprinting (tells server we want to stop sprinting). */
    void StopSprinting();
    /* Starts crouching (tells server we want to crouch). */
    void StartCrouching();    
    /* Stops crouching (tells server we want to uncrouch). */
    void StopCrouching();
    /* Toggles between first-person and third-person camera. */
    void OnChangePerspective();
    /* Tracks whether the player is in third-person view (client-side only). */
    bool bIsThirdPerson;
    /* Server RPC: performs interaction with a target actor. */
    UFUNCTION(Server, Reliable)
    void Server_Interact(AActor* TargetActor);
    /* Client input handler: triggers interaction trace and calls server. */
    UFUNCTION()
    void OnInteract();
};
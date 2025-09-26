#include "MyBaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "InteractiveInterface.h"
#include "MyBaseMovementComponent.h"
#include <MyBaseWidget.h>
#include "MyHealthComponent.h"
#include "MyStaminaComponent.h"

/**
 * Constructor for AMyBaseCharacter
 *
 * Sets up default properties and components for the character, including:
 * - Custom movement component
 * - Collision capsule
 * - Camera boom (spring arm)
 * - Follow camera
 * - Movement settings like speed, jump, and rotation behavior
 *
 * Many of these settings can be adjusted in Blueprints for faster iteration.
 */
AMyBaseCharacter::AMyBaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UMyBaseMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    /** Enable Tick() for this character so it updates every frame */
    PrimaryActorTick.bCanEverTick = true;

	/* The distance of your Spring Arm or how far away the camera is from you in thirdperson. */
	CameraDistance = 300.0f;
	
	/* The distance in which you can interact with objects when in first person. */
	BaseInteractDistance = 120.0f;

    /** Set size for the collision capsule (used for physics and collisions) */
    GetCapsuleComponent()->InitCapsuleSize(30.0f, 96.0f);

	 /* Do not rotate character pitch (up/down) based on controller. */
	bUseControllerRotationPitch = false;

	/* Do not rotate character yaw (left/right) based on controller. */
	bUseControllerRotationYaw = false;

	/* Do not rotate character roll (tilt) based on controller. */
	bUseControllerRotationRoll = false;

	/* Determines if we are first person perspective or third person perspective. */
	bIsThirdPerson = true;

	 /* If true, the character will rotate to face the direction it is moving instead of the controller’s rotation. */
	GetCharacterMovement()->bOrientRotationToMovement = true;

	/**
	 * The speed at which the character rotates to face movement direction.
	 * Only used if bOrientRotationToMovement is true.
	 * In this example, the character can rotate up to 300 degrees per second.
	 */
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 0.0f);

	/* The height of the player’s viewpoint (camera) relative to the capsule bottom while crouching. */
	CrouchedEyeHeight = 52.0f;

    /* JumpZVelocity: how high the character jumps */
    GetCharacterMovement()->JumpZVelocity = 280.0f;
    /* AirControl: how much control player has in air. */
	GetCharacterMovement()->AirControl = 0.05f;
    /* MinAnalogWalkSpeed: minimum speed for analog input */
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    /* BrakingDecelerationWalking: how fast character stops on ground */
    GetCharacterMovement()->BrakingDecelerationWalking = 900.0f;
    /* BrakingDecelerationFalling: how fast character stops in air */
    GetCharacterMovement()->BrakingDecelerationFalling = 0.0f;

    /** Create a camera boom (spring arm) that follows the character
     *  Pulls the camera closer if it collides with the environment */
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    /** Attach to character root */
    CameraBoom->SetupAttachment(RootComponent);
    /** Distance from character */
    CameraBoom->TargetArmLength = CameraDistance;
    /** Rotate boom based on controller input */
    CameraBoom->bUsePawnControlRotation = true;

    /** Creates a camera that follows the character */
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    /* We attach it to the end of the camera boom */
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    /* Camera itself does not rotate independently */
    FollowCamera->bUsePawnControlRotation = false;

	/* Add the HealthCompnoent to the Character. */
	MyHealthComponent = CreateDefaultSubobject<UMyHealthComponent>(TEXT("MyHealthComponent"));
	/* Add the StaminaCompnoent to the Character. */
	MyStaminaComponent = CreateDefaultSubobject<UMyStaminaComponent>(TEXT("MyStaminaComponent"));
}


void AMyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	MyMovement = Cast<UMyBaseMovementComponent>(GetCharacterMovement());

	// Only do UI on the owning client
	if (!IsLocallyControlled())
		return;

	// Check if the health component exists and is valid
	if (!IsValid(MyHealthComponent))
	{
		return; // Exit early because we can't update the UI without a health component
	}

	// Check if the Stamina component exists and is valid
	if (!IsValid(MyStaminaComponent))
	{
		return; // Exit early because we can't update the UI without a Stamina component
	}

	// Check if the widget class is valid (must be assigned in the Blueprint or code)
	if (!IsValid(WidgetClass))
	{
		return; // Exit early because we can't create a widget without a valid class
	}

	// Get the player controller that owns this character
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Create the actual widget instance from the class
		WidgetInstance = CreateWidget<UMyBaseWidget>(PC, WidgetClass);

		if (WidgetInstance) // If widget creation succeeded
		{
			// Add the widget to the viewport so the player can see it
			WidgetInstance->AddToViewport();

			// Initialize the health bar fill based on the current health percentage
			WidgetInstance->UpdateHealthBar(MyHealthComponent->GetHealthPercentage());

			// Initialize the Stamina bar fill based on the current stamina percentage
			WidgetInstance->UpdateStaminaBar(MyStaminaComponent->GetStaminaPercentage());

			// Make sure the widget is visible
			WidgetInstance->SetVisibility(ESlateVisibility::Visible);

			// Bind the widget's handler to the health component's OnHealthChanged delegate
			// This ensures that the health bar updates automatically whenever the character's health changes
			MyHealthComponent->OnHealthChanged.AddDynamic(WidgetInstance, &UMyBaseWidget::OnHealthChangedHandler);

			// Bind the widget's handler to the Stamin component's OnStaminaChanged delegate
			// This ensures that the Stamina Bar updates automatically whenever the character's Stamina changes
			MyStaminaComponent->OnStaminaChanged.AddDynamic(WidgetInstance, &UMyBaseWidget::OnStaminaChangedHandler);
		}
	}
}

/* 
 * Called every frame. 
 * A frame in this context is just a single cycle of the game loop.
 * So if your game is running at 60 frames per second (fps), 
 * then Tick will be called about 60 times per second on that Actor.
*/
void AMyBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/*
 * Called to bind functionality to input.
 * This function sets up how the player’s input (keyboard, mouse, controller, etc.)
 * is connected to the character’s actions.
 *
 * In this example, we’re using the Enhanced Input system:
 * JumpAction: Starts jumping when pressed, stops when released.
 * MoveAction: Handles character movement each time input is triggered.
 * LookAction: Handles looking/aiming each time input is triggered.
 * SprintAction: Starts sprinting while held, stops when released.
 *
 * Unreal automatically calls this function for you when the PlayerController
 * possesses this Character, so you typically dont call it manually.
 */
void AMyBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyBaseCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyBaseCharacter::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyBaseCharacter::OnInteract);

		EnhancedInputComponent->BindAction(ChangePerspectiveAction, ETriggerEvent::Started, this, &AMyBaseCharacter::OnChangePerspective);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AMyBaseCharacter::StartSprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMyBaseCharacter::StopSprinting);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AMyBaseCharacter::StartCrouching);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AMyBaseCharacter::StopCrouching);
	}
}

UMyBaseMovementComponent* AMyBaseCharacter::GetMyBaseMovementComponent()
{
	return MyMovement;
}

/*
 * Handles character movement input.
 *
 * Input comes in as a 2D vector (X = right/left, Y = forward/backward).
 * We take the Controller’s rotation to figure out which direction is considered "forward."
 * We only care about the yaw so the character doesn’t tilt up/down when moving.
 * From that yaw, we build forward and right direction vectors.
 * Finally, we apply movement input in those directions, scaled by the input values.
 */
void AMyBaseCharacter::Move(const FInputActionValue& Value)
{
	/* If our Controller is not valid or returns nothing, exit the function. */
	if (GetController() == nullptr) return;

	/* converts generic input value into a 2D vector for movement. */
	FVector2D MovementVector = Value.Get<FVector2D>();

	/* We cache the controllers rotation */
	const FRotator Rotation = GetController()->GetControlRotation();
	/* makes sure your character moves forward relative to where the camera is facing left/right */
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	/* Based on where the controller is facing, this is the forward direction to move */
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	/* Same concept, but this is the right direction to move. */
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	/* Adds movement towards a direction based on input */
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}


/*
 * Handles looking around with mouse or controller input.
 *
 * Input comes in as a 2D vector (X = turn left/right, Y = look up/down).
 * AddControllerYawInput(X) rotates the character left/right (yaw).
 * AddControllerPitchInput(Y) rotates the camera up/down (pitch).
 *
 * This directly affects the controllers rotation, which in turn
 * updates the character and camera orientation in the game world.
 */
void AMyBaseCharacter::Look(const FInputActionValue& Value)
{
	/* Get the 2D look input(X = turn left / right, Y = look up / down) */
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	/* Rotate the controller (and character) left/right based on X input */
	AddControllerYawInput(LookAxisVector.X);

	/* Rotate the controller (and camera) up/down based on Y input */
	AddControllerPitchInput(LookAxisVector.Y);
}

/*
 * Handles sprinting input for the character.
 *
 * StartSprinting(): Tells the movement component to Increase speed and set the Safe_bWantsToSprint flag to true.
 *
 * This is called when the sprint input action is started.
 */
void AMyBaseCharacter::StartSprinting()
{
	if (!MyStaminaComponent->HasStamina()) { return; }

	MyMovement->StartSprinting();

	if (!MyStaminaComponent->IsStaminaTimerActive()) {
		MyStaminaComponent->StartStaminaManipulation();
	}
}

/*
 * Handles sprinting input for the character.
 *
 * StopSprinting(): Tells the movement component to decrease speed and set the Safe_bWantsToSprint flag to false.
 *
 * This is called when the sprint input action is completed.
 */
void AMyBaseCharacter::StopSprinting()
{
	MyMovement->StopSprinting();
}

/*
 * Handles Crouching input for the character.
 *
 * StartCrouching(): Tells the movement component to we want to crouch and sets the bWantsToCrouch flag to true.
 *
 * This is called when the crouching input has started.
 */
void AMyBaseCharacter::StartCrouching()
{
	MyMovement->StartCrouching();
}

/*
 * Handles Crouching input for the character.
 *
 * StopCrouching(): Tells the movement component to we want to stop crouching and sets the bWantsToCrouch flag to false.
 *
 * This is called when the crouching input has completed.
 */
void AMyBaseCharacter::StopCrouching()
{
	MyMovement->StopCrouching();
}

/* Toggles between first - person and third - person perspectives.
* In first-person: camera attaches to the head bone, and character rotates with camera yaw.
* In third-person: camera attaches to the spring arm, and character rotates with movement direction. */
void AMyBaseCharacter::OnChangePerspective()
{
	if (bIsThirdPerson)
	{
		// Attach the camera directly to the character's head bone
		FollowCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("head"));

		// Allow the camera to rotate based on controller input (yaw/pitch)
		FollowCamera->bUsePawnControlRotation = true;

		// Apply a small relative offset so the camera isn't exactly inside the head
		FollowCamera->SetRelativeLocation(FVector(0.0f, 10.0f, 0.0f));

		// Make the whole character rotate with the controller's yaw
		bUseControllerRotationYaw = true;

		// Disable rotation based on movement direction (since rotation is now manual)
		GetCharacterMovement()->bOrientRotationToMovement = false;

		// Update state: character is no longer in third person
		bIsThirdPerson = false;
	}
	else
	{
		// Reattach the camera back onto the spring arm (CameraBoom)
		FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		// Do not let the camera itself rotate from controller input (spring arm handles it)
		FollowCamera->bUsePawnControlRotation = false;

		// Reset relative location so the camera is positioned correctly on the boom
		FollowCamera->SetRelativeLocation(FVector::ZeroVector);

		// Character no longer rotates directly with the camera
		bUseControllerRotationYaw = false;

		// Character rotation will follow the movement direction instead
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// Update state: character is now in third person
		bIsThirdPerson = true;
	}
}

void AMyBaseCharacter::Server_Interact_Implementation(AActor* TargetActor)
{
	// Server authority: This function only runs on the server
	// after the client calls the RPC `Server_Interact(TargetActor)`.

	// Safety check: make sure the actor actually implements your custom interface
	if (TargetActor->Implements<UInteractiveInterface>())
	{
		// Call the interface function on the actor.
		// The 'this' pointer is passed along so the interactable knows who interacted.
		IInteractiveInterface::Execute_Interact(TargetActor, this);
	}
}

void AMyBaseCharacter::OnInteract()
{
	// Only allow the *locally controlled* player (the one holding the controller) 
	// to run interaction logic. Prevents remote clients from firing traces.
	if (!IsLocallyControlled()) return;

	// Setup a hit result to store trace info
	FHitResult Hit;

	// Create collision params and tell it to ignore the player itself
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// Pick different interaction distances depending on view mode
	// If third-person, use a defined distance; otherwise (e.g., first-person) use 120 units
	float TraceDistance = bIsThirdPerson ? (CameraDistance + BaseInteractDistance) : BaseInteractDistance;

	// The trace starts at the camera’s location
	FVector Start = FollowCamera->GetComponentLocation();
	// And ends forward from the camera by the trace distance
	FVector End = Start + FollowCamera->GetForwardVector() * TraceDistance;

	// Debug line (green) drawn in the world for 1 second to visualize the trace
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f);
#endif

	// Perform a line trace (raycast) in the visibility channel, using our parameters
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// If the trace hit something, get the actor we hit
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			// Tell the server we want to interact with this actor
			// (so the actual interaction logic is authority-controlled and replicated)
			Server_Interact(HitActor);
		}
	}
}
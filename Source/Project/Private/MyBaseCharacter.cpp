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

	/*We set the ClientMesh to our default mesh.*/
	ClientMesh = GetMesh();

	// Offset the mesh downward to align it correctly with the capsule
    ClientMesh->SetRelativeLocation(FVector(-5.0f, 0.0f, -90.f));
    // Rotate the mesh so it faces the proper direction relative to the capsule
	ClientMesh->SetRelativeRotation(FQuat(FRotator(0.0f, -90.0f, 0.0f)));

    /*We create another Skeletal Mesh, that the other players will see on the server.
	This mesh will only be used for shadow affects. Client won't see this mesh at all.*/
    ServerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ServerMesh"));
    /*We attach the ServerMesh to our Client Mesh.*/
    ServerMesh->SetupAttachment(ClientMesh);

    // Create a spring arm to control first-person camera pitch/yaw
    FPSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FPSpringArm"));
    // Attach the spring arm to the head/neck bone of the client mesh for natural motion
	FPSpringArm->SetupAttachment(ClientMesh, TEXT("neck_02"));
    // Remove arm length so the camera sits directly at the bone position
	FPSpringArm->TargetArmLength = 0.0f;
    // Allow the players mouse/controller input to rotate the spring arm
	FPSpringArm->bUsePawnControlRotation = true;
	// Small positional offset to place the camera correctly inside the head
    FPSpringArm->SetRelativeLocation(FVector(10.0f, 15.0f, 0.0f));

    // Create the first-person camera component
    FPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCamera"));
    // Attach the camera to the end of the spring arm for stable FP viewing
	FPCamera->SetupAttachment(FPSpringArm, TEXT("SpringEndpoint"));

    // Create a spring arm to control third-person camera positioning and rotation
    TPSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TPSpringArm"));
	// Attach the spring arm to the capsule so it follows the player�s body smoothly
    TPSpringArm->SetupAttachment(GetCapsuleComponent());
	// Set a TargetArmLength to keep the camera at a distance behind the player
    TPSpringArm->TargetArmLength = 250.0f;
	// Allow the players mouse/controller input to rotate the spring arm around the character
    TPSpringArm->bUsePawnControlRotation = true;
	// Apply a relative location and target offset to position the camera above and behind the player for better visibility.
    TPSpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    TPSpringArm->TargetOffset = (FVector(0.0, 25.0f, 40.0f));

    // Create the third-person camera component
    // Attach it to the end of the TPSpringArm for stable third-person viewing
    TPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPCamera"));
    TPCamera->SetupAttachment(TPSpringArm, TEXT("SpringEndPoint"));
	/* The height of the players viewpoint (camera) relative to the capsule bottom while crouching. */
	CrouchedEyeHeight = 52.0f;

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

	// Hide the server mesh on the local player (only remote clients use it)
    ServerMesh->SetHiddenInGame(true);
	// Allow the hidden server mesh to still cast shadows for world accuracy
    ServerMesh->bCastHiddenShadow = true;
	// Enable shadow casting on the server mesh for consistent lighting
    ServerMesh->CastShadow = true;

    // Ensure the client mesh is visible for first-person rendering
    // Disable shadows on the FP mesh so it doesnt cast odd artifacts
    // Hide the head bone so the FP camera is not obstructed
    GetMesh()->SetHiddenInGame(false);
    GetMesh()->CastShadow = false;

    /*
	Optional: 
	You can Toggle ThirdPerson instead if you would like ThirdPerson view.
	*/
	ActivateFirstPerson();

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
 * This function sets up how the players input (keyboard, mouse, controller, etc.)
 * is connected to the characters actions.
 *
 * In this example, were using the Enhanced Input system:
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

		EnhancedInputComponent->BindAction(ChangePerspectiveAction, ETriggerEvent::Started, this, &AMyBaseCharacter::ToggleView);

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
 * We take the Controllers rotation to figure out which direction is considered "forward."
 * We only care about the yaw so the character doesn�t tilt up/down when moving.
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

	// Start stamina drain timer
	if (MyStaminaComponent)
	{
		MyStaminaComponent->StartDraining();
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

	// Start stamina regeneration timer
	if (MyStaminaComponent)
	{
		MyStaminaComponent->StartRegeneration();
	}
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

/*
 * Toggles the character camera view between first-person and third-person.
 *
 * ToggleView(): If the character is currently in third-person view, this activates the first-person view.
 * Otherwise, it activates the third-person view.
 *
 * This is called when the view toggle input is triggered.
 */
void AMyBaseCharacter::ToggleView()
{
    if (bIsThirdPerson) 
    {
        ActivateFirstPerson(); 
    } else 
    { 
        ActivateThirdPerson(); 
    }
}

/*
 * Activates the first-person camera view for the character.
 *
 * Enables the first-person camera, disables the third-person camera,
 * and hides the character's head mesh to prevent visual clipping.
 *
 * Updates the view state to indicate the character is no longer in third-person.
 */
void AMyBaseCharacter::ActivateFirstPerson()
{
    FPCamera->SetActive(true);
    TPCamera->SetActive(false);
    GetMesh()->HideBoneByName("Head", EPhysBodyOp::PBO_None);
    bIsThirdPerson = false;
}

/*
 * Activates the third-person camera view for the character.
 *
 * Disables the first-person camera, enables the third-person camera,
 * and unhides the character's head mesh.
 *
 * Updates the view state to indicate the character is now in third-person.
 */
void AMyBaseCharacter::ActivateThirdPerson()
{
    FPCamera->SetActive(false);
    TPCamera->SetActive(true);
    GetMesh()->UnHideBoneByName("Head");
    bIsThirdPerson = true;
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

	/*Store a reference to an empty Active Camera. 
	This will be set based on our ThirdPerson boolean.*/
	UCameraComponent* ActiveCamera = nullptr;

	if (bIsThirdPerson) 
	{
		ActiveCamera = TPCamera;
	}
	else
	{
		ActiveCamera = FPCamera;
	}
	/*Create a new float to store the Interact Distance.*/
	float TraceDistance = InteractDistance;

	/*If we are thirdPerson then we add onto the interact distance from the Camera.
	This is done because ThirdPerson Camera sets further back then the FirstPersons Camera.*/
	if (bIsThirdPerson)
	{
		TraceDistance += TPSpringArm->TargetArmLength;
	}

	const FVector Start = ActiveCamera->GetComponentLocation();
	const FVector End = Start + ActiveCamera->GetForwardVector() * TraceDistance;

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
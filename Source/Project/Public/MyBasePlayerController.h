#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <EnhancedInputSubsystems.h>
#include "MyBasePlayerController.generated.h"

UCLASS()
class PROJECT_API AMyBasePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMyBasePlayerController();

    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerSpawnPlayer(APlayerController* PlayerController);

protected:
    /** Input Mapping Contexts */
    UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
    TArray<UInputMappingContext*> DefaultMappingContexts;

    /** Input mapping context setup */
    virtual void SetupInputComponent() override;

};
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractiveInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractiveInterface : public UInterface
{
    GENERATED_BODY()
};

class PROJECT_API IInteractiveInterface
{
    GENERATED_BODY()

public:
    // Do NOT make this static! This generates Execute_Interact automatically.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);
};
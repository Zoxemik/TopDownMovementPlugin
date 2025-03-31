// The source code, authored by Zoxemik in 2025
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TopDownController.generated.h"

UENUM(BlueprintType)
enum class EInputType : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    KeyMouse    UMETA(DisplayName = "Key/Mouse"),
    Gamepad     UMETA(DisplayName = "Gamepad"),
    Touch       UMETA(DisplayName = "Touch")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeySwitchSignature, EInputType, NewInputType);

UCLASS()
class TOPDOWNMOVEMENT_API ATopDownController : public APlayerController
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Input")
    FOnKeySwitchSignature OnKeySwitch;

    void SetupInputComponent() override;

protected:
    EInputType CurrentInputType = EInputType::Unknown;

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetCurrentInputType(EInputType NewInputType);

private:
    UFUNCTION(BlueprintCallable)
    void OnKeyDetect(FKey Key);

    UFUNCTION(BlueprintCallable)
    void OnTouchDetect(FKey Key);

    UFUNCTION(BlueprintCallable)
    void OnMouseMove(float Value);
};

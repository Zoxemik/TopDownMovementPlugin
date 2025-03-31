// The source code, authored by Zoxemik in 2025

#include "Core/TopDownController.h"

void ATopDownController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);
}

void ATopDownController::OnKeyDetect(FKey Key)
{
	if (Key.IsGamepadKey() && CurrentInputType != EInputType::Gamepad)
	{
		SetCurrentInputType(EInputType::Gamepad);
	}
}

void ATopDownController::OnTouchDetect(FKey Key)
{
	if (Key.IsTouch() && CurrentInputType != EInputType::Touch)
	{
		SetCurrentInputType(EInputType::Touch);
	}
}

void ATopDownController::OnMouseMove(float Value)
{
	if (Value != 0 && CurrentInputType != EInputType::KeyMouse)
	{
		SetCurrentInputType(EInputType::KeyMouse);
	}
}

void ATopDownController::SetCurrentInputType(EInputType NewInputType)
{
	if (CurrentInputType != NewInputType)
	{
		CurrentInputType = NewInputType;

		OnKeySwitch.Broadcast(NewInputType);
	}
}
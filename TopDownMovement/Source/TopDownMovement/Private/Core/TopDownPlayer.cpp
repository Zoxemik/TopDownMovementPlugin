// The source code, authored by Zoxemik in 2025

#include "Core/TopDownPlayer.h"
#include "Core/InputDataSetup.h"
#include "Core/TopDownController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"


ATopDownPlayer::ATopDownPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	// Collision Sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(Root);
	CollisionSphere->SetSphereRadius(180.f);
	CollisionSphere->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	// Cursor Plane (ground visual for mouse cursor)
	CursorPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorPlane"));
	CursorPlane->SetupAttachment(Root);
	CursorPlane->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	CursorPlane->SetWorldScale3D(FVector(2.f, 2.f, 1.f));
	CursorPlane->SetGenerateOverlapEvents(false);
	CursorPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CursorPlane->SetCastShadow(false);

	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = 1100.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetRelativeRotation(FRotator(-40.f, 0.f, 0.f));
	SpringArm->SocketOffset = FVector(-300.f, 0.f, 80.f);
	SpringArm->bDoCollisionTest = false;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Movement Component
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	MovementComponent->UpdatedComponent = RootComponent;
	MovementComponent->MaxSpeed = 1500.f;
	MovementComponent->Acceleration = 8000.f;
	MovementComponent->Deceleration = 6500.f;
	MovementComponent->TurningBoost = 8.0f;
	MovementComponent->bConstrainToPlane = true;
	MovementComponent->bSnapToPlaneAtStart = true;
	MovementComponent->ConstrainNormalToPlane(FVector(0.0f, 0.0f, 1.0f));

	ZoomDirection = 0.f;
	ZoomValue = 0.5f;
	ZoomSpeed = 0.01f;

	PullStartDistance = 9000.f;

	EdgeMoveDistance = 50.f;

	CurrentInputType = EInputType::Unknown;

	TargetHandle = FVector(0.0f, 0.0f, 0.0f);
}

void ATopDownPlayer::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATopDownPlayer::OverlapBegin);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ATopDownPlayer::OverlapEnd);

	PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(BaseInputMappingContext, 0);
			Subsystem->AddMappingContext(SelectMappingContext, 0);
		}
	}

	if (ATopDownController* TopDownPlayerController = Cast<ATopDownController>(GetController()))
	{
		TopDownPlayerController->OnKeySwitch.AddDynamic(this, &ATopDownPlayer::HandleInputTypeSwitched);
	}

	UpdateZoom();

	GetWorld()->GetTimerManager().SetTimer(
		MoveTrackingTimerHandle,
		this,
		&ATopDownPlayer::MoveTracking,
		0.01667f,
		true
	);
}

void ATopDownPlayer::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		if (OtherActor->IsA(AActor::StaticClass()))
		{
			HoverActor = OtherActor;
		}
	}
}

void ATopDownPlayer::OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == HoverActor)
	{
		HoverActor = nullptr;
	}
}

void ATopDownPlayer::HandleInputTypeSwitched(EInputType NewInputType)
{
	//UE_LOG(LogTemp, Warning, TEXT("Input type switched to: %s"), *UEnum::GetValueAsString(NewInputType));
	CurrentInputType = NewInputType;
}

void ATopDownPlayer::Move(const FInputActionValue& Value)
{
	const FVector2D DirectionValue = Value.Get<FVector2D>();
	
	if (Controller && (DirectionValue != FVector2D(0.f)))
	{
		FVector ForwardVector = GetActorForwardVector();
		AddMovementInput(ForwardVector, DirectionValue.Y);
		FVector RightVector = GetActorRightVector();
		AddMovementInput(RightVector, DirectionValue.X);
	}
}

void ATopDownPlayer::Spin(const FInputActionValue& Value)
{
	const float RotationValue = Value.Get<float>();
	AddActorLocalRotation(FRotator(0.0f, RotationValue, 0.0f));
}

void ATopDownPlayer::Zoom(const FInputActionValue& Value)
{
	ZoomDirection = Value.Get<float>();

	DepthOfField();
	UpdateZoom();
}

void ATopDownPlayer::DragMove()
{
	SingleTouchCheck(PlayerController);

	const FVector SpringArmForward = SpringArm->GetForwardVector();
	const FVector SpringArmUp = SpringArm->GetUpVector();
	const float ArmLength = SpringArm->TargetArmLength;
	const float OffsetX = SpringArm->SocketOffset.X;
	const float OffsetZ = SpringArm->SocketOffset.Z;

	const FVector BackwardOffset = SpringArmForward * (ArmLength - OffsetX) * -1.0f;
	const FVector VerticalOffset = SpringArmUp * OffsetZ;
	const FVector CameraOffset = SpringArm->GetComponentLocation() + BackwardOffset + VerticalOffset;

	const FVector RelativeToCamera = CameraOffset - Camera->GetComponentLocation();
	FVector2D ScreenPos;
	FVector Intersection;
	bool bProjectionSuccess = ProjectToGroundPlane(PlayerController, ScreenPos, Intersection);

	if (bProjectionSuccess)
	{
		const FVector WorldDelta = TargetHandle - Intersection - RelativeToCamera;
		AddActorWorldOffset(FVector(WorldDelta.X, WorldDelta.Y, 0.0f));
	}
	else
	{
		if (!PlayerController) { UE_LOG(LogTemp, Warning, TEXT("DragMove Main PlayerController was not initialized")) return; }

		if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			FModifyContextOptions Options;
			Options.bForceImmediately = true;
			Options.bNotifyUserSettings = false;
			Options.bIgnoreAllPressedKeysUntilRelease = true;

			Subsystem->RemoveMappingContext(DragMoveMappingContext, Options);
		}
	}
}

void ATopDownPlayer::SelectStarted()
{
	SingleTouchCheck(PlayerController);

	PositionCheck();

	if (CollisionOverlapCheck())
	{
		HandleSelection();
	}
	else
	{
		if (!PlayerController) { UE_LOG(LogTemp, Warning, TEXT("SelectStarted Main PlayerController was not initialized")) return; }

		if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DragMoveMappingContext, 0);
		}
	}
}

void ATopDownPlayer::SelectStopped()
{
	if (!PlayerController) { UE_LOG(LogTemp, Warning, TEXT("SelectStopped Main PlayerController was not initialized")) return; }

	if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		FModifyContextOptions Options;
		Options.bForceImmediately = true;
		Options.bNotifyUserSettings = false;
		Options.bIgnoreAllPressedKeysUntilRelease = true;

		Subsystem->RemoveMappingContext(DragMoveMappingContext, Options);
	}
}

void ATopDownPlayer::HandleSelection()
{

}

void ATopDownPlayer::MoveTracking()
{
	if (!PlayerController) { UE_LOG(LogTemp, Warning, TEXT("MoveTracking Main PlayerController was not initialized")) return; }

	FVector CurrentPosition = GetActorLocation();

	// Calculate how far the player is from the origin (0,0,0)
	// Start applying movement only when distance exceeds 'PullStartDistance' units
	// The further out past PullStartDistance the stronger the movement (max value not clamped)
	float ScaleValue = CurrentPosition.Size() - PullStartDistance;
	ScaleValue = ScaleValue / 5000.f;
	ScaleValue = FMath::Max(ScaleValue, 0.f);

	// Flip the direction to point *toward* the origin instead of away from it
	// Also ensure movement is flat by zeroing out the Z axis
	FVector CurrentDirection = CurrentPosition.GetSafeNormal(0.0001f);
	CurrentDirection = UKismetMathLibrary::Multiply_VectorInt(FVector(CurrentDirection.X, CurrentDirection.Y, 0.0), -1);

	AddMovementInput(CurrentDirection, ScaleValue);

	FVector Direction;
	float Strength;
	EdgeMove(PlayerController, Direction, Strength);

	AddMovementInput(Direction, Strength);

	FVector2D ScreenPos;
	FVector Intersection;

	bool bProjectionSuccess = ProjectToGroundPlane(PlayerController, ScreenPos, Intersection);
	FVector CollisionLocation = CollisionSphere->GetComponentLocation();
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	FVector InterpolatedLocation = UKismetMathLibrary::VInterpTo(CollisionLocation, FVector(CollisionLocation.X, CollisionLocation.Y, -500.f), DeltaSeconds, 12.f);
	FVector TouchInterpolatedLocation = UKismetMathLibrary::SelectVector(Intersection, InterpolatedLocation, bProjectionSuccess);

	if (CurrentInputType == EInputType::Touch)
	{
		CollisionSphere->SetWorldLocation(TouchInterpolatedLocation);
	}
	else
	{
		CollisionSphere->SetWorldLocation(Intersection + FVector(0.0f, 0.0f, 10.f));
	}

	UpdateCursorPosition();
}

void ATopDownPlayer::UpdateZoom()
{
	ZoomValue = FMath::Clamp(ZoomValue + ZoomDirection * ZoomSpeed, 0.f, 1.f);

	if (!ZoomCurve) return;

	float ZoomCurveValue = ZoomCurve->GetFloatValue(ZoomValue);

	SpringArm->TargetArmLength = UKismetMathLibrary::Lerp(800.f, 40000.f, ZoomCurveValue);
	SpringArm->SetRelativeRotation(FRotator(UKismetMathLibrary::Lerp(-40.f, -55.f, ZoomCurveValue), 0.0f, 0.0f));
	MovementComponent->MaxSpeed = UKismetMathLibrary::Lerp(1000.f, 6000.f, ZoomCurveValue);
	DepthOfField();
	Camera->SetFieldOfView(UKismetMathLibrary::Lerp(20.f, 15.f, ZoomCurveValue));

}

void ATopDownPlayer::UpdateCursorPosition()
{

	FTransform TargetTransform;
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	if (CurrentInputType == EInputType::Touch)
	{

		FVector TargetLocation = FVector(0.f, 0.f, -100.f);
		FVector2D ScreenPos;
		FVector Intersection;

		bool bProjectionSuccess = ProjectToGroundPlane(PlayerController, ScreenPos, Intersection);

		if (!bProjectionSuccess)
		{
			TargetLocation = Intersection;
		}

		TargetTransform = UKismetMathLibrary::MakeTransform(TargetLocation, FRotator::ZeroRotator, FVector(2.0f, 2.0f, 1.0f));
	}
	else
	{
		if (HoverActor)
		{
			FVector Origin;
			FVector BoxExtent;
			HoverActor->GetActorBounds(true, Origin, BoxExtent);

			float TimeScaled = DeltaSeconds * 5.f;
			float PulsateOffset = FMath::Sin(TimeScaled) * 0.25f;

			FVector2D BoxExtent2D(BoxExtent.X, BoxExtent.Y);
			float MaxAxisValue = BoxExtent.GetAbsMax() / 50.f;
			float ScaleFactor = MaxAxisValue + PulsateOffset + 1.5f;

			FVector ScaledLocation(Origin.X, Origin.Y, 20.f);
			FVector ScaleVector(ScaleFactor, ScaleFactor, 1.f);
			FTransform ScaledTransform = UKismetMathLibrary::MakeTransform(ScaledLocation, FRotator::ZeroRotator, ScaleVector);

			TargetTransform = ScaledTransform;

		}
		else
		{
			TargetTransform = UKismetMathLibrary::MakeTransform(CollisionSphere->GetComponentTransform().GetLocation(), CollisionSphere->GetComponentTransform().GetRotation().Rotator(), FVector(2.0f, 2.0f, 1.0f));
		}
	}

	FTransform CurrentTransform = CursorPlane->GetComponentTransform();
	CursorPlane->SetWorldTransform(UKismetMathLibrary::TInterpTo(CurrentTransform, TargetTransform, DeltaSeconds, 12.0f));
}

void ATopDownPlayer::PositionCheck()
{
	FVector2D ScreenPos;
	FVector Intersection;
	ProjectToGroundPlane(PlayerController, ScreenPos, Intersection);

	TargetHandle = Intersection;
	if (CurrentInputType == EInputType::Touch)
	{
		CollisionSphere->SetWorldLocation(TargetHandle);
	}
}

inline void ATopDownPlayer::DepthOfField()
{
	float FocalDistance = SpringArm->TargetArmLength;
	
	FPostProcessSettings& PostProcessSettings = Camera->PostProcessSettings;
	
	Camera->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
	Camera->PostProcessSettings.bOverride_DepthOfFieldSensorWidth = true;
	Camera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;

	PostProcessSettings.DepthOfFieldFstop = 3.0f;
	PostProcessSettings.DepthOfFieldSensorWidth = 150.0f;
	PostProcessSettings.DepthOfFieldFocalDistance = FocalDistance;
}

inline bool ATopDownPlayer::ProjectToGroundPlane(APlayerController* LocalPlayerController, FVector2D& OutScreenPos, FVector& OutIntersection)
{
	if (!LocalPlayerController) { UE_LOG(LogTemp, Warning, TEXT("ProjectToGroundPlane PlayerController was not initialized")) return false; }

	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);
	FVector2D ViewportSize = FVector2D(ViewportX, ViewportY);
	FVector2D ViewportCenter = ViewportSize * 0.5f;
	
	float MouseX, MouseY;
	bool bGotMousePos = PlayerController->GetMousePosition(MouseX, MouseY);
	FVector2D MousePosition = FVector2D(MouseX, MouseY);

	float TouchX, TouchY;
	bool bGotTouchPos;
	PlayerController->GetInputTouchState(ETouchIndex::Touch1, TouchX, TouchY, bGotTouchPos);
	FVector2D TouchPosition = FVector2D(TouchX, TouchY);

	OutScreenPos = ViewportCenter;

	if (CurrentInputType == EInputType::KeyMouse && bGotMousePos)
	{
		OutScreenPos = MousePosition;
	}
	else if (CurrentInputType == EInputType::Touch && bGotTouchPos)
	{
		OutScreenPos = TouchPosition;
	}

	FVector WorldOrigin, WorldDirection;
	if (!PlayerController->DeprojectScreenPositionToWorld(OutScreenPos.X, OutScreenPos.Y, WorldOrigin, WorldDirection))
	{
		return false;
	}

	FVector LineStart = WorldOrigin;
	FVector LineEnd = WorldOrigin + (WorldDirection * 1000000.f);
	FPlane Plane = UKismetMathLibrary::MakePlaneFromPointAndNormal(FVector::ZeroVector, FVector(0.0f, 0.0f, 1.0f));
	
	float TValue;
	FVector Intersection;
	UKismetMathLibrary::LinePlaneIntersection(LineStart, LineEnd, Plane, TValue, Intersection);
	
	if (CurrentInputType == EInputType::Touch)
	{
		OutIntersection = Intersection + FVector(0.0f, 0.0f, -500.f);
	}
	else
	{
		OutIntersection = Intersection;
	}

	switch (CurrentInputType)
	{
	case EInputType::Unknown:

		return false;

	case EInputType::KeyMouse:
		
		return bGotMousePos;

	case EInputType::Gamepad:

		return true;

	case EInputType::Touch:

		return bGotTouchPos;

	default:

		return false;
	}
}

inline void ATopDownPlayer::CursorDistFromCenter(APlayerController* LocalPlayerController, FVector2D CursorPos, FVector& Direction, float& Strenght)
{
	if (!LocalPlayerController) { UE_LOG(LogTemp, Warning, TEXT("CursorDistFromCenter PlayerController was not initialized")) return; }

	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);
	FVector2D ViewportCenter = FVector2D(ViewportX, ViewportY) * 0.5f;

	float AbsCursorPosX, AbsCursorPosY;
	AbsCursorPosX = UKismetMathLibrary::Abs(CursorPos.X);
	AbsCursorPosY = UKismetMathLibrary::Abs(CursorPos.Y);

	float EdgeMoveScale = 0.0f;
	if (CurrentInputType == EInputType::Touch || CurrentInputType == EInputType::Gamepad)
	{
		EdgeMoveScale = 2.0f;
	}
	else if (CurrentInputType == EInputType::KeyMouse)
	{
		EdgeMoveScale = 1.0f;
	}
	
	float ScaledEdgeMoveDistance = EdgeMoveDistance * EdgeMoveScale;

	ViewportCenter.X = ViewportCenter.X - ScaledEdgeMoveDistance;
	ViewportCenter.Y = ViewportCenter.Y - ScaledEdgeMoveDistance;

	ViewportCenter.X = AbsCursorPosX - ViewportCenter.X;
	ViewportCenter.Y = AbsCursorPosY - ViewportCenter.Y;
	
	ViewportCenter.X = FMath::Max(ViewportCenter.X, 0.0f);
	ViewportCenter.Y = FMath::Max(ViewportCenter.Y, 0.0f);

	ViewportCenter.X = ViewportCenter.X / EdgeMoveDistance;
	ViewportCenter.Y = ViewportCenter.Y / EdgeMoveDistance;
	
	ViewportCenter.X = ViewportCenter.X * UKismetMathLibrary::SignOfFloat(CursorPos.X);
	ViewportCenter.Y = ViewportCenter.Y * UKismetMathLibrary::SignOfFloat(CursorPos.Y) * -1;

	Direction = FVector(ViewportCenter.Y, ViewportCenter.X, 0.0f);
	Strenght = 1;
}

inline void ATopDownPlayer::EdgeMove(APlayerController* LocalPlayerController,FVector& Direction, float& Strenght)
{

	if (!LocalPlayerController) { UE_LOG(LogTemp, Warning, TEXT("EdgeMove PlayerController was not initialized")) return; }

	FVector2D ProjectionScreenPos;
	FVector ProjectionIntersection;
	
	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);
	FVector2D ViewportCenter = FVector2D(ViewportX, ViewportY) * 0.5f;

	bool bIsProjectionSuccess = ProjectToGroundPlane(LocalPlayerController, ProjectionScreenPos, ProjectionIntersection);

	FVector2D CursorOffsetFromCenter = ProjectionScreenPos - ViewportCenter;
	CursorDistFromCenter(LocalPlayerController, CursorOffsetFromCenter, Direction, Strenght);

	FTransform ActorTransform = GetActorTransform();

	Direction = UKismetMathLibrary::TransformDirection(ActorTransform, Direction);

}

inline bool ATopDownPlayer::SingleTouchCheck(APlayerController* LocalPlayerController)
{
	double TouchX, TouchY;
	bool bIsCurrentlyPressed;
	LocalPlayerController->GetInputTouchState(ETouchIndex::Touch2, TouchX, TouchY, bIsCurrentlyPressed);

	return (!bIsCurrentlyPressed);
}

inline bool ATopDownPlayer::CollisionOverlapCheck()
{
	TArray<AActor*> ActorArray;
	CollisionSphere->GetOverlappingActors(ActorArray);
	return ((ActorArray.Num() > 0) && (ActorArray[0] != nullptr));
}

void ATopDownPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputActions->MoveAction, ETriggerEvent::Triggered, this, &ATopDownPlayer::Move);
		EnhancedInputComponent->BindAction(InputActions->DragMoveAction, ETriggerEvent::Triggered, this, &ATopDownPlayer::DragMove);
		EnhancedInputComponent->BindAction(InputActions->SpinAction, ETriggerEvent::Triggered, this, &ATopDownPlayer::Spin);
		EnhancedInputComponent->BindAction(InputActions->ZoomAction, ETriggerEvent::Triggered, this, &ATopDownPlayer::Zoom);
		EnhancedInputComponent->BindAction(InputActions->SelectAction, ETriggerEvent::Started, this, &ATopDownPlayer::SelectStarted);
		EnhancedInputComponent->BindAction(InputActions->SelectAction, ETriggerEvent::Canceled, this, &ATopDownPlayer::SelectStopped);
		EnhancedInputComponent->BindAction(InputActions->SelectAction, ETriggerEvent::Completed, this, &ATopDownPlayer::SelectStopped);
	}
}
// The source code, authored by Zoxemik in 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "TopDownPlayer.generated.h"

class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UFloatingPawnMovement;
class UInputMappingContext;
class UInputDataSetup;
class UInputAction;

UCLASS()
class TOPDOWNMOVEMENT_API ATopDownPlayer : public APawn
{
	GENERATED_BODY()

public:
	ATopDownPlayer();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	/**
	* Called when another actor begins to overlap with this actor.
	* By default, checks if the overlapping object is an AActor and sets it as the HoverActor.
	* Should be customized depending on the specific needs of the project.
	*/
	UFUNCTION()
	void OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	* Called when another actor ends overlap with this actor.
	* By default, clears the HoverActor if it matches the actor that ended overlap.
	* Should be customized depending on the specific needs of the project.
	*/
	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleInputTypeSwitched(EInputType NewInputType);

	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Spin(const FInputActionValue& Value);

	UFUNCTION()
	void Zoom(const FInputActionValue& Value);

	UFUNCTION()
	void DragMove();

	UFUNCTION()
	void SelectStarted();

	UFUNCTION()
	void SelectStopped();

	/**
	* Handles the selection logic; implementation depends on the specific project requirements.
	*/
	UFUNCTION(BlueprintCallable)
	void HandleSelection();

	/**
	 * MoveTracking - Continuously updates player pawn movement.
	 * 1. Pulls pawn toward the world origin if it moves beyond a set distance (PullStartDistance).
	 * 2. Manages edge scrolling: moves the pawn when the cursor or touch is near viewport edges.
	 * 3. Updates the cursor position on-screen and aligns the collision detection sphere.
	 */
	UFUNCTION()
	void MoveTracking();

	/**
	 * UpdateZoom - Updates the zoom level of the camera, affecting arm length, camera angle,
	 * movement speed, depth of field, and FOV.
	 *
	 * The zoom is smoothly interpolated using a curve for more natural behavior.
	 */
	UFUNCTION()
	void UpdateZoom();

	/**
	 * UpdateCursorPosition - Updates the cursor plane's location and scale based on the current input type (touch vs. hover).
	 * - For touch input, attempts to project the screen touch location onto the ground plane.
	 * - For hover (e.g., mouse), checks the HoverActor bounds to apply a pulsating effect.
	 */
	UFUNCTION()
	void UpdateCursorPosition();

	/**
	* PositionCheck - Projects the current cursor/touch position onto the ground plane and updates the TargetHandle variable.
	*
	* This function sets the interaction location (TargetHandle) which is subsequently utilized in the DragMove function
	* to calculate pawn movement when dragging the camera or interacting with the game world.
	*
	* It also adjusts the CollisionSphere location directly when using touch input.
	*/
	UFUNCTION()
	void PositionCheck();

	/**
	 * DepthOfField - Configures depth of field settings on the player's camera using post-process overrides.
	 * Uses the SpringArm's length as the focal distance to dynamically adjust focus based on camera distance.
	 */
	UFUNCTION()
	inline void DepthOfField();

	/**
	 * ProjectToGroundPlane - Converts 2D cursor/touch positions into 3D game-world intersections.
	 * @param LocalPlayerController: The player's controller used for viewport and input calculations.
	 * @param OutScreenPos: Returns the 2D viewport position of the input.
	 * @param OutIntersection: Returns the intersection position on the ground plane.
	 * @return bool - Indicates successful projection based on input validity.
	 */
	UFUNCTION()
	inline bool ProjectToGroundPlane(APlayerController* LocalPlayerController, FVector2D& OutScreenPos, FVector& OutIntersection);

	/**
	 * CursorDistFromCenter - Computes how far the cursor is from the center of the viewport.
	 *
	 * @param LocalPlayerController: The player's controller used for viewport and input calculations.
	 * @param CursorPos: Current cursor position.
	 * @param Direction: Output direction vector relative to viewport center.
	 * @param Strength: Output strength scalar indicating how far from the center the cursor is.
	 */
	UFUNCTION()
	inline void CursorDistFromCenter(APlayerController* LocalPlayerController, FVector2D CursorPos, FVector& Direction, float& Strenght);

	/**
	 * EdgeMove - Calculates movement direction and strength based on the cursor’s offset from the screen center.
	 * Converts the local direction to world space relative to the player's transform.
	 *
	 * @param LocalPlayerController: The player's controller used for viewport and input calculations.
	 * @param Direction: Output world-space direction vector.
	 * @param Strength: Output movement strength scalar.
	 */
	UFUNCTION()
	inline void EdgeMove(APlayerController* LocalPlayerController, FVector& Direction, float& Strenght);

	/**
	 * SingleTouchCheck - Checks if a second touch input (Touch2) is currently not pressed on the given player controller.
	 * Returns true if the second touch is not pressed, and false if it is pressed.
	 *
	 * @param LocalPlayerController: The player's controller handling the touch inputs.
	 * @return bool indicating the presence of a single touch input.
	 */
	UFUNCTION()
	inline bool SingleTouchCheck(APlayerController* LocalPlayerController);

	/**
	* CollisionOverlapCheck - Checks if the CollisionSphere currently overlaps any actors.
	* Useful for determining selectable or interactable objects near the player.
	* 
	* Note: This function can be customized depending on your project's needs,
	* as it determines specifically what kind of interactions you're looking for.
	*
	* @return true if overlapping interactable actors are detected; false otherwise.
	*/
	UFUNCTION()
	inline bool CollisionOverlapCheck();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CursorPlane;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputDataSetup> InputActions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BaseInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DragMoveMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> SelectMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	TObjectPtr<UCurveFloat> ZoomCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	float ZoomSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	float PullStartDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	float EdgeMoveDistance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	TObjectPtr<AActor> HoverActor;
private:
	float ZoomDirection;
	float ZoomValue;

	EInputType CurrentInputType;
	FTimerHandle MoveTrackingTimerHandle;

	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defaults", meta = (AllowPrivateAccess = "true"))
	FVector TargetHandle;
};

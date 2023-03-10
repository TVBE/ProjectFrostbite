// Copyright (c) 2022-present Barrelhouse
// Written by Tim Verberne
// This source code is part of the project Frostbite

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterConfiguration.h"
#include "PlayerCharacterController.generated.h"

class UPlayerSubsystem;
class APlayerCharacter;
class APlayerCharacterState;
class UPlayerCharacterMovementComponent;
struct FTimerHandle;

/** The PlayerController for the PlayerCharacter. This class is responsible for handling all user input to the player Pawn. */
UCLASS(Blueprintable, ClassGroup=(PlayerCharacter))
class APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()
	
public:
	/** The character configuration to use for this player character. */
	UPROPERTY()
	UPlayerCharacterConfiguration* CharacterConfiguration;

	/** The state configuration to use for this player character. */
	UPROPERTY()
	UPlayerStateConfiguration* StateConfiguration;

protected:
	/** Pointer to the controlled pawn as a PlayerCharacter instance.*/
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacterController|Actors", Meta = (DisplayName = "Player Character"))
	APlayerCharacter* PlayerCharacter {nullptr};

	/** If true, the player is currently pressing the sprint button. */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacterController|Intention", Meta = (DisplayName = "Is Sprint Pending"))
	bool IsSprintPending {false};

	/** If true, the player is currently pressing the crouch button. */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacterController|Intention", Meta = (DisplayName = "Is Crouch Pending"))
	bool IsCrouchPending {false};

private:
	/** Pointer to the PlayerCharacterState instance for this controller. */
	UPROPERTY(BlueprintGetter = GetPlayerCharacterState, Category = "PlayerCharacterController|PlayerState", Meta = (DisplayName = "Player Character State"))
	APlayerCharacterState* PlayerCharacterState;
	
	/** When true, the player can receive user input for movement. */
	UPROPERTY(BlueprintGetter = GetCanProcessMovementInput, Category = "PlayerCharacterController", Meta = (DisplayName = "Can Process Movement Input"))
	bool CanProcessMovementInput {false};

	/** When true, the player can receive user input for camera rotation. */
	UPROPERTY(BlueprintGetter = GetCanProcessRotationInput, Category = "PlayerCharacterController", Meta = (DisplayName = "Can Process Rotation Input"))
	bool CanProcessRotationInput {false};

	/** Timer handle for the state update timer. */
	UPROPERTY()
	FTimerHandle StateTimer;

public:
	APlayerCharacterController();
	
	/** Returns whether the PlayerController has any movement input or not. */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacterController|Input", Meta = (DisplayName = "Has Any Movement Input"))
	bool GetHasMovementInput() const;

	/** Returns the current horizontal rotation input value from the PlayerController. */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacterController|Input", Meta = (DisplayName = "Get Horizontal Rotation Input"))
	float GetHorizontalRotationInput() const;

	/** Sets CanProcessMovementInput. This function can only be called by a PlayerSubsystem. */
	void SetCanProcessMovementInput(const UPlayerSubsystem* Subsystem, const bool Value);

	/** Sets CanProcessRotationInput. This function can only be called by a PlayerSubsystem. */
	void SetCanProcessRotationInput(const UPlayerSubsystem* Subsystem, const bool Value);

protected:
	
	/** Checks whether the player is currently looking at an interactable object. */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacterController", Meta = (DisplayName = "Can Interact"))
	bool CanInteract() const;
	
	/** Increases the PlayerMovementComponent maximum forward speed. */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterController", Meta = (DisplayName = "Start Sprinting"))
	void StartSprinting();
	
	/** Sets the PlayerMovementComponent maximum forward speed to its default value. */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterController", Meta = (DisplayName = "Stop Sprinting"))
	void StopSprinting();
	
	/** Set the PlayerMovementComponent to crouch. */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterController", Meta = (DisplayName = "Start Crouching"))
	void StartCrouching();
	
	/** Set the PlayerMovementComponent to stop crouching. */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterController", Meta = (DisplayName = "Sop Crouching"))
	void StopCrouching();
	
	/** Performs a collision query in front of the camera and returns the hit result. */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacterController", Meta = (DisplayName = "Get Camera Look At Query"))
	FHitResult GetCameraLookAtQuery() const;

	/** Updates the player character state. */
	UFUNCTION()
	void UpdatePlayerState();

private:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;
	virtual void InitPlayerState() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	/** Adjusts the character's horizontal orientation using a gamepad or mouse. */
	UFUNCTION()
	void HandleHorizontalRotation(float Value);

	/** Adjusts the character's vertical orientation using a gamepad or mouse. */
	UFUNCTION()
	void HandleVerticalRotation(float Value);

	/** Tries to move the character forward or backward depending on the input. */
	UFUNCTION()
	void HandleLongitudinalMovementInput(float Value);

	/** Tries to move the character forward or backward depending on the input. */
	UFUNCTION()
	void HandleLateralMovementInput(float Value);

	/** Handles the callback for when the player has pressed the Jump button */
	UFUNCTION()
	void HandleJumpActionPressed();

	/** Handles the callback for when the player has pressed the Sprint button. */
	UFUNCTION()
	void HandleSprintActionPressed();

	/** Handles the callback for when the player has released the Sprint button. */
	UFUNCTION()
	void HandleSprintActionReleased();

	/** Handles the callback for when the player has pressed the Crouch button. */
	UFUNCTION()
	void HandleCrouchActionPressed();

	/** Handles the callback for when the player has released the Crouch button. */
	UFUNCTION()
	void HandleCrouchActionReleased();

	/** Handles the callback for when the player has pressed the ToggleFlashlight button. */
	UFUNCTION()
	void HandleFlashlightActionPressed();


	/** Checks if the player character can currently sprint. */
	UFUNCTION()
	bool CanCharacterSprint() const;
	
	/** Checks if any player actions are currently pending and tries to complete them. */
	UFUNCTION()
	void UpdatePendingActions(const UPlayerCharacterMovementComponent* CharacterMovement);

	/** Checks if the player can continue with any actions they are currently performing. */
	UFUNCTION()
	void UpdateCurrentActions(const UPlayerCharacterMovementComponent* CharacterMovement);

public:
	/** Returns the player character state. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacterController|PlayerState", Meta = (DisplayName = "Player Character State"))
	FORCEINLINE APlayerCharacterState* GetPlayerCharacterState() const {return PlayerCharacterState; }
	
	/** Returns whether the player controller can process movement input. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacterController", Meta = (DisplayName = "Can Process Movement Input"))
	FORCEINLINE bool GetCanProcessMovementInput() const { return CanProcessMovementInput; }

	/** Returns whether the player controller can process rotation input. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacterController", Meta = (DisplayName = "Can Process Rotation Input"))
	FORCEINLINE bool GetCanProcessRotationInput() const { return CanProcessRotationInput; }
};

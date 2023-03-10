// Copyright (c) 2022-present Barrelhouse
// Written by Tim Verberne
// This source code is part of the project Frostbite

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UPlayerCharacterConfiguration;
class UPlayerStateConfiguration;
class APlayerCharacterController;
class UCameraComponent;
class USpotLightComponent;
class USpringArmComponent;
class UPlayerVfxComponent;
class UPlayerCameraController;
class UPlayerFlashlightComponent;
class UPlayerCharacterMovementComponent;
class UNiagaraComponent;
enum class EFoot : uint8;
enum class EPlayerLandingType : uint8;
struct FFootstepData;
struct FTimerHandle;

UCLASS(Abstract, Blueprintable, BlueprintType, NotPlaceable, ClassGroup = (PlayerCharacter))
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	// CONFIGURATION
	/** The character configuration data asset. */
	UPROPERTY(BlueprintGetter = GetCharacterConfiguration, EditAnywhere, Category = "PlayerCharacter|Configuration", Meta = (DisplayName = "Character Configuration", DisplayPriority = "0"))
	UPlayerCharacterConfiguration* CharacterConfiguration;

	/** The playerstate configuration data asset. */
	UPROPERTY(BlueprintGetter = GetStateConfiguration, EditAnywhere, Category = "PlayerCharacter|Configuration", Meta = (DisplayName = "State Configuration", DisplayPriority = "1"))
	UPlayerStateConfiguration* StateConfiguration;
	
	// COMPONENTS
	/** The camera for the player. */
	UPROPERTY(BlueprintGetter = GetCamera, EditAnywhere, Category = "PlayerCharacter|Camera", Meta = (DisplayName = "Camera"))
	UCameraComponent* Camera;
	
	/** The CameraController that handles first person camera behavior. */
	UPROPERTY(BlueprintGetter = GetCameraController, EditAnywhere, Category = "PlayerCharacter|Components", Meta = (DisplayName = "Camera Controller"))
	UPlayerCameraController* CameraController;
	
	/** The PlayerCharacterMovementComponent that handles the PlayerCharacter's movement. */
	UPROPERTY(BlueprintGetter = GetPlayerCharacterMovement, EditAnywhere, Category = "PlayerCharacter|Components", Meta = (DisplayName = "Player Character Movement Component"))
	UPlayerCharacterMovementComponent* PlayerCharacterMovement;
	
	/** The PlayerCharacterController that is currently controlling this PlayerCharacter. */
	UPROPERTY(BlueprintGetter = GetPlayerCharacterController, Category = "PlayerCharacter", Meta = (DisplayName = "Player Character Controller"))
	APlayerCharacterController* PlayerCharacterController;
	
	// VARIABLES
	/** If true, the character is currently turning in place. */
	UPROPERTY(BlueprintGetter = GetIsTurningInPlace, Category = "PlayerCharacter|Locomotion", Meta = (DisplayName = "Is Turning In Place"))
	bool IsTurningInPlace {false};

	/** The yaw delta value used for turn-in-place rotation. */
	UPROPERTY(BlueprintGetter = GetYawDelta, Category = "PlayerCharacter|Locomotion", Meta = (DisplayName = "Yaw Delta"))
	float YawDelta {0.f};

	// TIMERS
	/** The timer handle for the hard and heavy landing stun duration. */
	UPROPERTY()
	FTimerHandle FallStunTimer;

public:
	/** Sets default values for this character's properties. */
	APlayerCharacter();
	
	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;
	
	/** Called after the constructor but before BeginPlay. */
	virtual void PostInitProperties() override;

	/** Ss called after all of the actor's components have been created and initialized, but before the BeginPlay function is called. */
	virtual void PostInitializeComponents() override;

	/** Performs a collision query above the Pawn and returns the clearance. This will return -1.f if the query did not produce any hit results. */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacterController", Meta = (DisplayName = "Get Clearance Above Pawn"))
	float GetClearanceAbovePawn() const;

	/** Checks whether the player can currently jump. */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacter", Meta = (DisplayName = "Can Jump"))
	bool CanPerformJump() const;
	
	/** Checks whether the player can currently enter crouch. */
	bool CanCrouch() const override;
	
	/** Checks whether the player can stand up and stop crouching. */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacter", Meta = (DisplayName = "Can Stand Up"))
	bool CanStandUp() const;

protected:
	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Called after all default properties have been initialized. */
	virtual void OnConstruction(const FTransform& Transform) override;
	
	/** Called when the pawn is possessed by a controller. */
	virtual void PossessedBy(AController* NewController) override;

	/** Called when the pawn is ready to be destroyed or when the game ends. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Updates the character's rotation. */
	void UpdateRotation(const float& DeltaTime);

	/** Validates the configuration data assets. */
	void ValidateConfigurationAssets();

	/** Applies the configuration data assets to the character. */
	void ApplyConfigurationAssets();

private:
	/** Updates the character's yaw delta. */
	UFUNCTION()
	void UpdateYawDelta();
	
	/** Returns a scaled yaw delta value that is used for turn-in-place rotation.
	 *	@YawDelta The delta yaw rotation between the player's control rotation and the character's mesh rotation.
	 *	@DeltaTime The frame time in seconds.
	 *	@Factor A multiplier that affects the rotation speed.
	 *	@Clamp The maximum allowed YawDelta angle before the rotation speed should be clamped to prevent the camera from rotation too much in relation to the character's neck. //TODO: This doesn't work properly currently.
	 *	@Return A float value representing a rotator's yaw axis.
	 */
	static float CalculateTurnInPlaceRotation (const float YawDelta, const float DeltaTime, const float Factor, const float Clamp);

	/** Handles the landing callback from the player character movement component. */
	UFUNCTION()
	void HandleLanding(EPlayerLandingType Value);

	/** Handles the ending of a landing. */
	UFUNCTION()
	void HandleLandingEnd();

#if WITH_EDITOR
	/** Checks whether an object is properly initialized.
	 *	@Object The object to validate.
	 *	@Objectname The name of the object to be used in the log entry should the object not be properly initialized. 
	 */
	static void ValidateObject(const UObject* Object, const FString ObjectName);
#endif

public:
	/** Returns the Character configuration. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Configuration", Meta = (DisplayName = "Get Character Configuration"))
	FORCEINLINE UPlayerCharacterConfiguration* GetCharacterConfiguration() const {return CharacterConfiguration; }

	/** Returns the PlayerState configuration. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Configuration", Meta = (DisplayName = "Get State Configuration"))
	FORCEINLINE UPlayerStateConfiguration* GetStateConfiguration() const {return StateConfiguration; }
	
	/** Returns the PlayerCharacterController that is controlling this PlayerCharacter. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Locomotion", Meta = (DisplayName = "PlayerCharacterController"))
	FORCEINLINE APlayerCharacterController* GetPlayerCharacterController() const {return PlayerCharacterController; }
	
	/** Returns the camera component. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Components", Meta = (DisplayName = "Camera"))
	FORCEINLINE UCameraComponent* GetCamera() const {return Camera; }
	
	/** Returns the Player Camera Controller. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Components", Meta = (DisplayName = "Camera Controller"))
	FORCEINLINE UPlayerCameraController* GetCameraController() const {return CameraController; }
	
	/** Returns the PlayerCharacterMovementComponent. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Components", Meta = (DisplayName = "Player Character Movement Component"))
	FORCEINLINE UPlayerCharacterMovementComponent* GetPlayerCharacterMovement() const {return PlayerCharacterMovement; }
	
	/** Returns if the character is currently turning in place. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Locomotion", Meta = (DisplayName = "Is Turning In Place"))
	FORCEINLINE bool GetIsTurningInPlace() const {return IsTurningInPlace; }

	/** Returns the character yaw delta between the facing direction of the character and the camera. */
	UFUNCTION(BlueprintGetter, Category = "PlayerCharacter|Locomotion", Meta = (DisplayName = "Yaw Delta"))
	FORCEINLINE float GetYawDelta() const {return YawDelta; }
};





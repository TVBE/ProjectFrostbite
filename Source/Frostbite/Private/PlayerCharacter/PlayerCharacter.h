// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LandingIntensity.h"
#include "PlayerVFXController.h"
#include "PlayerCharacter.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:

protected:
	/** If true, the character is currently jumping. We assume the character is jumping if the character has not left the ground yet, but the jump action is triggered. */
	UPROPERTY(BlueprintReadOnly, Category = Locomotion, Meta = (DisplayName = "Is Jumping"))
	bool IsJumping;

private:
	/** The PlayerCharacterController that is currently controlling this PlayerCharacter. */
	UPROPERTY(BlueprintGetter = GetPlayerCharacterController, Category = Default, Meta = (DisplayName = "Player Character Controller"))
	class APlayerCharacterController* PlayerCharacterController;
	
	/** The camera for the player. */
	UPROPERTY(BlueprintGetter = GetCamera, EditAnywhere, Category = Camera, Meta = (DisplayName = "Camera"))
	class UCameraComponent* Camera;

	/** The flashlight for the player. */
	UPROPERTY(BlueprintGetter = GetFlashlight, EditAnywhere, Category = Flashlight, Meta = (DisplayName = "Flashlight"))
	class USpotLightComponent* Flashlight;

	/** The SpringArmComponent the flashlight is attached to. */
	UPROPERTY(BlueprintGetter = GetFlashlightSpringArm, EditAnywhere, Category = Flashlight, Meta = (DisplayName = "Flashlight Arm"))
	class USpringArmComponent* FlashlightSpringArm;

	/** The PlayerAudioController that handles player audio. */
	UPROPERTY(BlueprintGetter = GetAudioController, Category = Components, Meta = (DisplayName = "Player Audio Controller"))
	class UPlayerAudioController* AudioController;

	/** The PlayerVFXController that handles player VFX. */
	UPROPERTY(BlueprintGetter = GetVFXController, Category = Components, Meta = (DisplayName = "Player VFX Controller"))
	class UPlayerVFXController* VFXController;

	/** The CameraController that handles first person camera behavior. */
	UPROPERTY(BlueprintGetter = GetCameraController, Category = Components, Meta = (DisplayName = "Player Camera Controller"))
	class UPlayerCameraController* CameraController;

	/** The FlashlightController that handles the flashlight behavior. */
	UPROPERTY(BlueprintGetter = GetFlashlightController, Category = Components, Meta = (DisplayName = "Player Flashlight Controller"))
	class UPlayerFlashlightController* FlashlightController;

	/** The PlayerCharacterMovementComponent that handles the PlayerCharacter's movement. */
	UPROPERTY(BlueprintGetter = GetPlayerCharacterMovement, EditAnywhere, Category = Components, Meta = (DisplayName = "Player Character Movement Component"))
	class UPlayerCharacterMovementComponent* PlayerCharacterMovement;

	/** The AudioComponent for general player audio. */
	UPROPERTY(BlueprintGetter = GetBodyAudioComponent, EditAnywhere, Category = Components, Meta = (DisplayName = "Body Audio Component"))
	class UAudioComponent* BodyAudioComponent;

	/** The AudioComponent for the player's left foot. */
	UPROPERTY(BlueprintGetter = GetLeftFootAudioComponent, EditAnywhere, Category = Components, Meta = (DisplayName = "Left Foot Audio Component"))
	class UAudioComponent* LeftFootAudioComponent;

	/** The AudioComponent for the player's right foot.*/
	UPROPERTY(BlueprintGetter = GetRightFootAudioComponent, EditAnywhere, Category = Components, Meta = (DisplayName = "Right Foot Audio Component"))
	class UAudioComponent* RightFootAudioComponent;
	
	/** If true, the character is currently turning in place. */
	UPROPERTY(BlueprintGetter = GetIsTurningInPlace, Category = Locomotion, Meta = (DisplayName = "Is Turning In Place"))
	bool IsTurningInPlace {false};

	/** The yaw delta value used for turn-in-place rotation. */
	UPROPERTY(BlueprintGetter = GetYawDelta, Category = Locomotion, Meta = (DisplayName = "Yaw Delta"))
	float YawDelta {0.f};

public:
	// Sets default values for this character's properties
	APlayerCharacter();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PostInitProperties() override;


public:
	/** Returns the PlayerCharacterController that is controlling this PlayerCharacter. */
	UFUNCTION(BlueprintGetter, Category = Locomotion, Meta = (DisplayName = "PlayerCharacterController"))
	FORCEINLINE APlayerCharacterController* GetPlayerCharacterController() const {return PlayerCharacterController; }
	
	/** Returns the camera component. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Camera"))
	FORCEINLINE UCameraComponent* GetCamera() const {return Camera; }

	/** Returns the flashlight component. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Flashlight"))
	FORCEINLINE USpotLightComponent* GetFlashlight() const {return Flashlight; }

	/** Returns the flashlight SpringArmComponent. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Flashlight Spring Arm"))
	FORCEINLINE USpringArmComponent* GetFlashlightSpringArm() const {return FlashlightSpringArm; }

	/** Returns the Player Camera Controller. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Camera Controller"))
	FORCEINLINE UPlayerCameraController* GetCameraController() const {return CameraController; }

	/** Returns the Player Flashlight Controller. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Flashlight Controller"))
	FORCEINLINE UPlayerFlashlightController* GetFlashlightController() const {return FlashlightController; }
	
	/** Returns the Player Audio Controller. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Player Audio Controller"))
	FORCEINLINE UPlayerAudioController* GetAudioController() const {return AudioController; }

	/** Returns the Player VFX Controller*/
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Player VFX Controller"))
	FORCEINLINE UPlayerVFXController* GetVFXController() const {return VFXController; }

	/** Returns the PlayerCharacterMovementComponent. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Player Character Movement Component"))
	FORCEINLINE UPlayerCharacterMovementComponent* GetPlayerCharacterMovement() const {return PlayerCharacterMovement; }

	/** Returns the Body AudioComponent. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Body Audio Component"))
	FORCEINLINE UAudioComponent* GetBodyAudioComponent() const {return BodyAudioComponent; }

	/** Returns the Body AudioComponent. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Left Foot Audio Component"))
	FORCEINLINE UAudioComponent* GetLeftFootAudioComponent() const {return LeftFootAudioComponent; }

	/** Returns the Body AudioComponent. */
	UFUNCTION(BlueprintGetter, Category = Components, Meta = (DisplayName = "Right Foot Audio Component"))
	FORCEINLINE UAudioComponent* GetRightFootAudioComponent() const {return RightFootAudioComponent; }

	/** Returns if the character is currently turning in place. */
	UFUNCTION(BlueprintGetter, Category = Locomotion, Meta = (DisplayName = "Is Turning In Place"))
	FORCEINLINE bool GetIsTurningInPlace() const {return IsTurningInPlace; }

	/** Returns the character yaw delta between the facing direction of the character and the camera. */
	UFUNCTION(BlueprintGetter, Category = Locomotion, Meta = (DisplayName = "Yaw Delta"))
	FORCEINLINE float GetYawDelta() const {return YawDelta; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Updates the character's rotation. */
	void UpdateRotation(float DeltaTime);
	
	/** Specify whether the character is currently jumping. */
	UFUNCTION(BlueprintCallable, Category = Locomotion, Meta = (DisplayName = "Set IsJumping", CompactNodeTitle = "Is Jumping"))
	void SetIsJumping(bool Value);

private:
	/** Updates the character's yaw delta. */
	UFUNCTION()
	void UpdateYawDelta();
	
	/** Returns a scaled yaw delta value that is used for turn-in-place rotation. */
	static float CalculateTurnInPlaceRotation (const float YawDelta, const float DeltaTime, const float Factor, const float Clamp);

	/** Handles a controller change for the pawn. */
	UFUNCTION()
	void HandleControllerChange(APawn* Pawn, AController* OldController, AController* NewController);

#if WITH_EDITOR
	/** Checks whether a component is properly initialized. */
	static void ValidateObject(const UObject* Object, const FString ObjectName);
#endif
	
};




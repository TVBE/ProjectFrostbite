// Copyright (c) 2022-present Barrelhouse
// Written by Tim Verberne
// This source code is part of the project Frostbite

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacterConfiguration.h"
#include "Components/ActorComponent.h"
#include "UObject/WeakObjectPtr.h"
#include "PlayerCameraController.generated.h"

class UCameraComponent;
class APlayerCharacter;
class APlayerCharacterController;

/** UPlayerCameraController is an Actor Component responsible for managing the player camera's behavior, such as camera shakes and other effects.
 *	This class provides a simple and convenient way for designers to customize the camera's behavior and add special effects to the player's view. */
class APlayerCharacterController;
UCLASS(Blueprintable, BlueprintType, ClassGroup = (PlayerCharacter), Meta = (BlueprintSpawnableComponent) )
class UPlayerCameraController : public UActorComponent
{
	GENERATED_BODY()

private:
	// CONFIGURATION
	/** The configuration asset to use for this component. */
	UPROPERTY(EditAnywhere, Category = "Configuration", Meta = (DisplayName = "Configuration"))
	TSoftObjectPtr<UPlayerCameraConfiguration> ConfigurationAsset;

	/** Pointer to the configuration asset for this component. */
	UPROPERTY(BlueprintGetter = GetConfiguration, Category = "Configuration", Meta = (DisplayName = "Configuration"))
	UPlayerCameraConfiguration* Configuration;

	// VARIABLES
	/** Pointer to the PlayerCharacter. */
	UPROPERTY()
	APlayerCharacter* PlayerCharacter;

	/** Pointer to PlayerCharacterController. */
	UPROPERTY()
	APlayerCharacterController* PlayerCharacterController;
	
	/** When set to true, the player does not have full rotational control over the camera's orientation. */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerController|Animation", Meta = (DisplayName = "Lock Camera To Animation", AllowPrivateAccess = "true"))
	bool IsCameraLockedToAnimation {false};
	
	/** The default head socket rotation from the skeletal mesh of the PlayerCharacterPawn. */
	UPROPERTY()
	FTransform HeadSocketTransform {FTransform()};

	/** Interpolated head socket rotation. */
	UPROPERTY()
	FRotator InterpolatedHeadSocketRotation {FRotator()};

	/** The roll offset value of the camera shake rotation. */
	UPROPERTY()
	double CameraShakeRoll {0.0};

	/** The roll offset value of the camera lean rotation. */
	UPROPERTY()
	double CameraLeanRoll {0.0};

public:	
	// Sets default values for this component's properties
	UPlayerCameraController();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Fades the camera from black. */
	void FadeFromBlack(const float Duration);

protected:
	// Called when the game starts
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

private:
	/** Tries to get the owning pawn's player controller as PlayerCharacterController. */
	UFUNCTION()
	void HandleCharacterControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	/** Updates the camera relative location. */
	void UpdateCameraLocation(UCameraComponent& Camera);

	/** Updates the camera world rotation*/
	void UpdateCameraRotation(const UCameraComponent& Camera, const float DeltaTime);

	/** Returns a rotation offset for the camera to simulate the camera shaking while moving. */
	FRotator GetCameraSwayRotation();

	/** Returns a rotation offset for the camera when the player rotates while sprinting. Used to simulate leaning when running into bends. */
	FRotator GetCameraCentripetalRotation();

	/** Returns a scaled head socket delta rotation from the skeletal mesh of the PlayerCharacterPawn. */
	FRotator GetScaledHeadSocketDeltaRotation(const float DeltaTime);
	
	/** Updates the camera's field of view according to the Player's movement. */
	void UpdateCameraFieldOfView(UCameraComponent& Camera, const float DeltaTime);

	/** Updates the camera's vignette intensity according to the Player's movement.*/
	void UpdateCameraVignetteIntensity(UCameraComponent& Camera, const float DeltaTime);

	/** Updates the camera's depth of field according to whatever the player is looking at.*/
	void UpdateCameraDepthOfField(const UCameraComponent& Camera, const float DeltaTime);

	/** Performs a linetrace in the forward vector of the camera and returns the length of the trace. */
	float GetFocalDistance(const UCameraComponent& Camera) const;

public:
	/** Returns the Camera configuration. */
	UFUNCTION(BlueprintGetter, Category = "Configuration", Meta = (DisplayName = "Get Configuration"))
	FORCEINLINE UPlayerCameraConfiguration* GetConfiguration() const {return Configuration; }
};

// Copyright 2023 Barrelhouse

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerFlashlightController.generated.h"

class APlayerCharacter;
class UPlayerFlashlightConfiguration;
enum class EPlayerGroundMovementType : uint8;

/** UPlayerFlashlightController is an Actor Component responsible for controlling the player's flashlight. 
 *	This class provides a simple and convenient way for designers to customize the player's flashlight behavior.
 *	@Brief ActorComponent for controlling the player's flashlight.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (PlayerCharacter), Meta = (BlueprintSpawnableComponent))
class UPlayerFlashlightController : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	/** Pointer to the flashlight configuration of the player character this component is part of. */
	UPlayerFlashlightConfiguration* FlashlightConfiguration;
	
private:
	/** Pointer to the PlayerCharacter this component is part of. */
	UPROPERTY()
	APlayerCharacter* PlayerCharacter;
	
	/** Alpha value for blending the flashlight rotation based on movement. */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerFlashlightController", Meta = (DisplayName = "Movement Alpha", AllowPrivateAccess = "true"))
	float MovementAlpha {0.f};
	
public:	
	// Sets default values for this component's properties
	UPlayerFlashlightController();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Enables or disables the flashlight. */
	UFUNCTION(BlueprintCallable, Category = "PlayerFlashlightController", Meta = (DisplayName = "Set Flashlight Enabled"))
	void SetFlashlightEnabled(const bool Value);

	/** Returns whether the flashlight is enabled or not. */
	UFUNCTION(BlueprintPure, Category = "PlayerFlashlightController", Meta = (DisplayName = "Is Flashlight Enabled"))
	bool IsFlashlightEnabled() const;

	/** Updates the movement alpha value. */
	void UpdateMovementAlpha(const float DeltaTime);

	/** Calculates the flashlight focus rotation.
	 *	@Return The target rotation for the flashlight to focus on whatever surface the player is looking at.
	 */
	FRotator GetFlashlightFocusRotation() const;

	/** Calculates the flashlight sway rotation.
	 *	@Return A rotator that can be added to the world rotation of the flashlight to introduce flashlight sway.
	 */
	FRotator GetFlashlightSwayRotation() const;

	/** Returns the flashlight socket rotation with an offset depending on the movement type of the PlayerCharacter.
	 *	@Socket The socket to get the rotation from.
	 *	@MovementType The current ground movement type of the player.
	 *	@Return The rotation of the socket with an offset depending on the ground movement type.
	 */
	FRotator GetSocketRotationWithOffset(const FName Socket, const EPlayerGroundMovementType MovementType) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};

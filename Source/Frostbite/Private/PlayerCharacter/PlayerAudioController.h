// Copyright 2023 Barrelhouse

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerAudioController.generated.h"

class APlayerCharacter;

/** UPlayerAudioController is an Actor Component responsible for managing all audio specific to the player character. 
 *	This class provides a simple and convenient way for designers to customize the player's audio implementation.
 *	@Brief ActorComponent for managing player audio.
 */
UCLASS(Blueprintable, ClassGroup=(PlayerCharacter), meta=(BlueprintSpawnableComponent) )
class UPlayerAudioController : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** Sets default values for this component's properties. */
	UPlayerAudioController();

protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Initializes the component. Occurs at level startup or actor spawn. This is before BeginPlay. */
	virtual void InitializeComponent() override;

	/** Pointer to the PlayerCharacter that owns this component. */
	UPROPERTY(BlueprintReadOnly, Category = "AudioController", Meta = (DisplayName = "Player Character"))
	APlayerCharacter* PlayerCharacter;

public:	
	/** Called every frame. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	
};

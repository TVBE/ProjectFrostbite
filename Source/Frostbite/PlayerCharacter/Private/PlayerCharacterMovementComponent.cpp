// Copyright 2023 Tim Verberne

#include "PlayerCharacterMovementComponent.h"

void UPlayerCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UPlayerCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	OnLocomotionEvent.Broadcast(EPlayerLocomotionEvent::Jump);
	OnJump.Broadcast();
	return Super::DoJump(bReplayingMoves);
}

void UPlayerCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	if(Velocity.Z < -1000)
	{
		if(Velocity.Z < -1300)
		{
			OnLanding.Broadcast(EPlayerLandingType::Heavy);
		}
		else
		{
			OnLanding.Broadcast(EPlayerLandingType::Hard);
		}
	}
	else
	{
		OnLanding.Broadcast(EPlayerLandingType::Soft);
	}
	Super::ProcessLanded(Hit, remainingTime, Iterations);
}

/** Checks the current movement state and returns a corresponding enumeration value. */
EPlayerGroundMovementType UPlayerCharacterMovementComponent::GetGroundMovementType() const
{
	if(IsSprinting)
	{
		return EPlayerGroundMovementType::Sprinting;
	}
	if(IsMovingOnGround() && Velocity.SquaredLength() >= 25)
	{
		return EPlayerGroundMovementType::Walking;
	}
	return EPlayerGroundMovementType::Idle;
}

// Called by the player controller.
void UPlayerCharacterMovementComponent::SetIsSprinting(const bool Value, const APlayerController* Controller)
{
	if(!PawnOwner || IsSprinting == Value)
	{
		return;
	}
	if(PawnOwner->GetController() == Controller)
	{
		IsSprinting = Value;
	}
	if(Value)
	{
		OnLocomotionEvent.Broadcast(EPlayerLocomotionEvent::SprintStart);
	}
	else
	{
		OnLocomotionEvent.Broadcast(EPlayerLocomotionEvent::SprintEnd);
	}
}



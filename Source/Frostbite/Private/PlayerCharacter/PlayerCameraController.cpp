// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCameraController.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterController.h"
#include "PlayerCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Frostbite/Frostbite.h"

// Sets default values for this component's properties.
UPlayerCameraController::UPlayerCameraController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Initializes the component. Occurs at level startup or actor spawn. This is before BeginPlay.
void UPlayerCameraController::InitializeComponent()
{
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	if(PlayerCharacter != nullptr)
	{
		HeadSocketTransform = PlayerCharacter->GetMesh()->GetSocketTransform("head", RTS_Actor);
		PlayerCharacter->ReceiveControllerChangedDelegate.AddDynamic(this, &UPlayerCameraController::HandleCharacterControllerChanged);
		
	}
	else
	{
		const FString Class {GetOwner()->GetClass()->GetDisplayNameText().ToString()};
		UE_LOG(LogPlayerCameraController, Error, TEXT("PlayerCameraController expected pawn of type APlayerCharacter, is attached to Actor of type %s instead"), *Class);
	}
	const FString PawnInstance {GetOwner()->GetActorNameOrLabel()};
	UE_LOG(LogPlayerCameraController, VeryVerbose, TEXT("CameraController initialized for %s."), *PawnInstance);
	Super::InitializeComponent();
}

// Called after the pawn's controller has changed.
void UPlayerCameraController::HandleCharacterControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	APlayerCharacterController* Controller {Cast<APlayerCharacterController>(NewController)};
	if(Controller != nullptr)
	{
		PlayerCharacterController = Controller;
	}
}

// Called when the game starts.
void UPlayerCameraController::BeginPlay()
{
	Super::BeginPlay();
	if(PlayerCharacter)
	{
		if(PlayerCharacter->GetCameraConfiguration())
		{
			CameraConfigurationData = PlayerCharacter->GetCameraConfiguration()->PlayerCameraConfigurationData;
		}
		if(PlayerCharacter->GetCamera())
		{
			PlayerCharacter->GetCamera()->SetFieldOfView(CameraConfigurationData.DefaultFOV);
		}
	}
}


// Called every frame.
void UPlayerCameraController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(PlayerCharacter && PlayerCharacterController)
	{
		UpdateCameraRotation(); // Even with camera sway and centripetal rotation disabled, we need to call this function every frame to update the actual orientation of the camera.
		UpdateCameraLocation();
		if(CameraConfigurationData.IsDynamicFOVEnabled)
		{
			UpdateCameraFieldOfView();
		}
		if(CameraConfigurationData.IsDynamicDOFEnabled)
		{
			UpdateCameraDepthOfField(DeltaTime);
		}
	}
}

// Called by TickComponent.
void UPlayerCameraController::UpdateCameraLocation()
{
	/* Get an alpha value based on the pitch of the camera. We do not want the camera to explicitly follow the head socket if the body of the player isn't visible (e.g. looking down),
	 as this could be perceived as annoying by the user. */ 
	const double PitchAlpha
	{FMath::GetMappedRangeValueClamped(FVector2d(-30.0, -55.0), FVector2d(0.0, 1.0), PlayerCharacter->GetCamera()->GetComponentRotation().Pitch)};
	
	// Get the delta position of the current head socket location in relation to the default location. This allows us to introduce some socket-bound headbobbing with scalable intensity.
	const FVector SocketLocation
	{FVector(0, 0,(PlayerCharacter->GetMesh()->GetSocketTransform("head", RTS_Actor).GetLocation() - HeadSocketTransform.GetLocation()).Z * 0.5)};
	
	FVector Result {FVector()};
	// If the player is looking forward or up, we don't need to perform any additional calculations and can set the relative location to the CameraConfiguration's default value.
	if(PitchAlpha == 0.0)
	{
		Result = CameraConfigurationData.CameraOffset + (SocketLocation * !PlayerCharacter->GetIsTurningInPlace());
		
	}
	else
	{
		// Get the target location if the player is not looking down.
		const FVector UprightCameraLocation {CameraConfigurationData.CameraOffset + (SocketLocation * !PlayerCharacter->GetIsTurningInPlace())};
		
		// Calculate the target location if the player is looking down.
		const FVector DownwardCameraLocation {PlayerCharacter->GetMesh()->GetSocketTransform("head", RTS_Actor).GetLocation() + FVector(CameraConfigurationData.CameraOffset.X * 0.625, 0, 0)
		- FVector(0, 0, (PlayerCharacter->GetVelocity().X * 0.02))}; // We lower the camera slightly when the character is moving forward to simulate the body leaning forward.
		
		// Interpolate between the two target locations depending on PitchAlpha.
		Result = FMath::Lerp(UprightCameraLocation, DownwardCameraLocation, PitchAlpha); //NOTE: In UE 5.1 using FMath::Lerp() with two FVectors can cause semantic errors, but the code will compile and run just fine.
	}
	// Rotate the result with the base aim rotation.
	const FRotator ControlRotation {FRotator(0, PlayerCharacterController->GetControlRotation().Yaw, 0)};
	Result = ControlRotation.RotateVector(Result);
	
	// Add the world location of the pawn to the result.
	Result = Result + PlayerCharacter->GetActorLocation();
	
	PlayerCharacter->GetCamera()->SetWorldLocation(Result);
}

// Called by TickComponent.
void UPlayerCameraController::UpdateCameraRotation()
{
	const FRotator Sway {CameraConfigurationData.IsCameraSwayEnabled ? GetCameraSwayRotation() : FRotator()};
	const FRotator CentripetalRotation {CameraConfigurationData.IsCentripetalRotationEnabled ? GetCameraCentripetalRotation() : FRotator()};
	FRotator SocketRotation {FRotator()};
	if(!PlayerCharacter->GetIsTurningInPlace())
	{
		SocketRotation = GetScaledHeadSocketDeltaRotation();
	}
	PlayerCharacter->GetCamera()->SetWorldRotation(Sway + CentripetalRotation + SocketRotation + PlayerCharacter->GetControlRotation());
}

// Called by UpdateCameraRotation.
FRotator UPlayerCameraController::GetCameraSwayRotation()
{
	// Get the current ground movement type from the PlayerController.
	if(!PlayerCharacter->GetPlayerCharacterMovement()) {return FRotator();}
	
	const EPlayerGroundMovementType MovementType {PlayerCharacter->GetPlayerCharacterMovement()->GetGroundMovementType()};
	// Get a oscillation multiplier value according to the ground movement type.
	float IntensityMultiplier {0.0};
	switch(MovementType)
	{
	case 0: IntensityMultiplier = 0.1; // Idle
		break;
	case 1: IntensityMultiplier = 0.3; // Walking
		break;
	case 2: IntensityMultiplier = 1.65; // Sprinting
		break;
	default: IntensityMultiplier = 0.3; // Miscellaneous
		break;
	}
	// Get a mapped deviation value that scales the shake intensity and speed. Used to introduce some cyclical pseudo-random variance.
	const double Deviation {FMath::GetMappedRangeValueClamped(FVector2d(-1.0, 1.00), FVector2d(0.75, 1.5),
					UKismetMathLibrary::Cos(UGameplayStatics::GetTimeSeconds(GetWorld()) * 2.4))};
	
	// Calculate the target shake rotation.
	float Intensity {CameraConfigurationData.CameraShakeIntensity};
	const double TargetRollOffset {UKismetMathLibrary::Cos(UGameplayStatics::GetTimeSeconds(GetWorld()) * Deviation) * IntensityMultiplier * Deviation * CameraConfigurationData.CameraShakeIntensity};
	
	// Interpolate between the current camera roll and the target camera roll.
	CameraShakeRoll = FMath::FInterpTo(CameraShakeRoll, TargetRollOffset, GetWorld()->GetDeltaSeconds(), 3.0);
	
	// Return a rotator with the camera roll offset.
	return FRotator(0, 0, CameraShakeRoll);
}

// Called by UpdateCameraRotation.
FRotator UPlayerCameraController::GetCameraCentripetalRotation()
{
	FRotator Rotation {FRotator()};
	double TargetRoll {0.0};
	if(PlayerCharacter->GetPlayerCharacterMovement() && PlayerCharacter->GetPlayerCharacterMovement()->GetIsSprinting())
	{
		// When the player is moving laterally while sprinting, we want the camera to lean into that direction.
		const float LateralVelocityMultiplier {0.002353f * CameraConfigurationData.VelocityCentripetalRotation};
		const FVector WorldVelocity {PlayerCharacter->GetMovementComponent()->Velocity};
		const FVector LocalVelocity {PlayerCharacter->GetActorTransform().InverseTransformVector(WorldVelocity)};
		const double LateralVelocityRoll {LocalVelocity.Y * LateralVelocityMultiplier};
		
		// When the player is rotating horizontally while sprinting, we want the camera to lean into that direction.
		const float HorizontalRotationRoll{FMath::Clamp(PlayerCharacterController->GetHorizontalRotationInput() * CameraConfigurationData.RotationCentripetalRotation,
					-CameraConfigurationData.MaxCentripetalRotation, CameraConfigurationData.MaxCentripetalRotation)};

		TargetRoll = LateralVelocityRoll + HorizontalRotationRoll;
	}
	// Interpolate the roll value.
	CameraLeanRoll = FMath::FInterpTo(CameraLeanRoll, TargetRoll, GetWorld()->GetDeltaSeconds(), 4.f);
	Rotation = (FRotator(0, 0, CameraLeanRoll));
	return Rotation;
}

FRotator UPlayerCameraController::GetScaledHeadSocketDeltaRotation()
{
	// Get the current ground movement type from the PlayerController.
	if(!PlayerCharacter->GetPlayerCharacterMovement()) {return FRotator();};
	const EPlayerGroundMovementType MovementType {PlayerCharacter->GetPlayerCharacterMovement()->GetGroundMovementType()};
	// Get a oscillation multiplier value according to the ground movement type.
	float IntensityMultiplier {0.0};
	if(!PlayerCharacter->GetMovementComponent()->IsFalling())
	{
		switch(MovementType)
		{
		case 2: IntensityMultiplier = 1.25; // Sprinting
			break;
		default: IntensityMultiplier = 0.5; // Miscellaneous
			break;
		}
	}
	// Get the delta head socket rotation.
	FRotator TargetHeadSocketRotation {(PlayerCharacter->GetMesh()->GetSocketTransform("head", RTS_Actor).GetRotation()
		- HeadSocketTransform.GetRotation()) * IntensityMultiplier};

	// Apply scalars
	TargetHeadSocketRotation = FRotator(TargetHeadSocketRotation.Pitch, (TargetHeadSocketRotation.Yaw * 0), (TargetHeadSocketRotation.Roll * 1.5));

	// Interpolate the rotation value to smooth out jerky rotation changes.
	if(const UWorld* World {GetWorld()})
	{
		InterpolatedHeadSocketRotation = FMath::RInterpTo(InterpolatedHeadSocketRotation, TargetHeadSocketRotation, World->GetDeltaSeconds(), 4);
	}
	return InterpolatedHeadSocketRotation;
}

// Called by TickComponent.
void UPlayerCameraController::UpdateCameraFieldOfView()
{
	FPlayerCharacterConfigurationData Configuration {FPlayerCharacterConfigurationData()};
	if(PlayerCharacter->GetCharacterConfiguration())
	{
		Configuration = PlayerCharacter->GetCharacterConfiguration()->PlayerCharacterConfigurationData;
	}

	float TargetFOV {CameraConfigurationData.DefaultFOV};
	const FVector WorldVelocity {PlayerCharacter->GetMovementComponent()->Velocity};
	const FVector LocalVelocity {PlayerCharacter->GetActorTransform().InverseTransformVector(WorldVelocity)};
	if (LocalVelocity.X > Configuration.WalkSpeed * 1.1)
	{
		TargetFOV = FMath::GetMappedRangeValueClamped(FVector2D(Configuration.WalkSpeed * 1.1, Configuration.SprintSpeed),
					FVector2D(CameraConfigurationData.DefaultFOV, CameraConfigurationData.SprintFOV), LocalVelocity.X);
	} 

	PlayerCharacter->GetCamera()->FieldOfView = FMath::FInterpTo(PlayerCharacter->GetCamera()->FieldOfView, TargetFOV, GetWorld()->GetDeltaSeconds(),2.f );
}

void UPlayerCameraController::UpdateCameraDepthOfField(float DeltaTime)
{
	float FocalDistance {GetFocalDistance()};
	FocalDistance = FMath::Clamp(FocalDistance, CameraConfigurationData.MinimumFocalDistance, CameraConfigurationData.MaximumFocalDistance);
	
	const float BlurFocus {static_cast<float>(FMath::GetMappedRangeValueClamped
		(FVector2D(CameraConfigurationData.MinimumFocalDistance, CameraConfigurationData.MaximumFocalDistance),
			FVector2D(CameraConfigurationData.MacroBlurFocus,CameraConfigurationData.LongShotBlurFocus),FocalDistance))};
	
	const float BlurAmount {static_cast<float>(FMath::GetMappedRangeValueClamped
		(FVector2D(CameraConfigurationData.MinimumFocalDistance, CameraConfigurationData.MaximumFocalDistance),
			FVector2D(CameraConfigurationData.MacroBlurAmount,CameraConfigurationData.LongShotBlurAmount),FocalDistance))};

	PlayerCharacter->GetCamera()->PostProcessSettings.DepthOfFieldSkyFocusDistance =
		FMath::FInterpTo(PlayerCharacter->GetCamera()->PostProcessSettings.DepthOfFieldSkyFocusDistance, FocalDistance, DeltaTime, CameraConfigurationData.DynamicDofSpeed);
	
	PlayerCharacter->GetCamera()->PostProcessSettings.DepthOfFieldDepthBlurAmount = 
		FMath::FInterpTo(PlayerCharacter->GetCamera()->PostProcessSettings.DepthOfFieldDepthBlurAmount, BlurFocus, DeltaTime, CameraConfigurationData.DynamicDofSpeed);
	
	PlayerCharacter->GetCamera()->PostProcessSettings.DepthOfFieldDepthBlurRadius =
		FMath::FInterpTo(PlayerCharacter->GetCamera()->PostProcessSettings.DepthOfFieldDepthBlurRadius, BlurAmount, DeltaTime, CameraConfigurationData.DynamicDofSpeed);
}

float UPlayerCameraController::GetFocalDistance()
{
	
	if (!PlayerCharacter || !PlayerCharacter->GetCamera())
	{
		return 0.0f;
	}

	UCameraComponent* Camera {PlayerCharacter->GetCamera()};
	FVector CameraLocation = Camera->GetComponentLocation();
	FVector ForwardVector = Camera->GetForwardVector();

	constexpr float TraceLength {50000.0f};
	ForwardVector *= TraceLength;
	FVector End = CameraLocation + ForwardVector;

	FHitResult HitResult;
	if (Camera->GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, End, ECollisionChannel::ECC_Visibility))
	{
		return (HitResult.Location - CameraLocation).Size();
	}
	return TraceLength;
}






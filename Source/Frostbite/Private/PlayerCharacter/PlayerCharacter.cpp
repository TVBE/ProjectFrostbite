// Copyright 2023 Barrelhouse


#include "PlayerCharacter.h"

#include "PlayerAudioController.h"
#include "PlayerCameraController.h"
#include "PlayerCharacterController.h"
#include "PlayerCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "MetasoundSource.h"
#include "PlayerVfxController.h"
#include "Core/FrostbiteGameMode.h"
#include "Core/PlayerSubsystem.h"
#include "Math/Vector.h"
#include "Core/LogCategories.h"


/** The PlayerCharacter's initialization follows these stages:
 *	1) Constructor: Creates the actor and sets its default properties. We cannot access default property values at this time.
 *	2) PostInitProperties(): Called after construction to perform additional initialization that requires access to default property values.
 *	3) OnConstruction(): Called after all default property values have been fully initialized, but before any of the components are initialized.
 *	4) PostInitializeComponents(): Called after initializing the components, which allows them to register with other systems and set up data structures.
 *	5) BeginPlay(): Called when the actor is ready to be used in the game world.
 */

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	/** Construct Camera. */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(this->RootComponent);
	Camera->SetRelativeLocation(FVector(22.0, 0.0, 75.0));
	Camera->FieldOfView = 90.0;
	Camera->bUsePawnControlRotation = false;

#define FOOTSOCKET_L "foot_l_Socket"
#define FOOTSOCKET_R "foot_r_Socket"
#define BODYSOCKET "spine_04"
	
	/** Construct Audio Component. */
	BodyAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Body Audio Component"), true);
	BodyAudioComponent->SetupAttachment(GetMesh(), BODYSOCKET);
	BodyAudioComponent->bAutoActivate = false;
	BodyAudioComponent->bEditableWhenInherited = false;
	
	static ConstructorHelpers::FObjectFinder<UMetaSoundSource> MainSourceMetasound(TEXT("/Script/MetasoundEngine.MetaSoundSource'/Game/Game/Audio/Sources/Player/Main/MSS_Player_Main.MSS_Player_Main'")); // TODO: We need to replace this with TSoftObjectPtrs.
	if(MainSourceMetasound.Succeeded())
	{
		BodyAudioComponent->SetSound(MainSourceMetasound.Object);
	}
	
	/** Construct Particle System Components. */
	LeftFootParticleEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Left Foot Particle Emitter"), true);
	LeftFootParticleEmitter->SetupAttachment(GetMesh(), FOOTSOCKET_L);
	LeftFootParticleEmitter->bAutoActivate = false;
	LeftFootParticleEmitter->bEditableWhenInherited = false;

	RightFootParticleEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Right Foot Particle Emitter"),true);
	RightFootParticleEmitter->SetupAttachment(GetMesh(), FOOTSOCKET_R);
	RightFootParticleEmitter->bAutoActivate = false;
	RightFootParticleEmitter->bEditableWhenInherited = false;
	
	/** Construct Camera Controller. */
	CameraController = CreateDefaultSubobject<UPlayerCameraController>(TEXT("Camera Controller"));
	CameraController->bEditableWhenInherited = false;
	
	/** Construct Audio Controller, we want to use the Blueprint derived class for this so that designers can easily script behavior for the audio controller. */
	static ConstructorHelpers::FClassFinder<UPlayerAudioController> AudioControllerClass(TEXT("/Script/Engine.Blueprint'/Game/Game/Actors/PlayerCharacter/Blueprints/Components/BPC_PlayerAudioController.BPC_PlayerAudioController_C'"));
	if(AudioControllerClass.Succeeded())
	{
		AudioController = Cast<UPlayerAudioController>(CreateDefaultSubobject(TEXT("Audio Controller"), AudioControllerClass.Class, AudioControllerClass.Class, true, true ));
	}
	else
	{
		/** Construct the base class if the Blueprint derived class cannot be found. */
		AudioController = CreateDefaultSubobject<UPlayerAudioController>(TEXT("Audio Controller")); 
	}
	AudioController->bEditableWhenInherited = false;
	
	/** Construct VFX Controller, we want to use the Blueprint derived class for this so that designers can easily script behavior for the VFX controller. */
	static ConstructorHelpers::FClassFinder<UPlayerVfxController> VfxControllerClass(TEXT("/Script/Engine.Blueprint'/Game/Game/Actors/PlayerCharacter/Blueprints/Components/BPC_PlayerVfxController.BPC_PlayerVfxController_C'"));
	if(VfxControllerClass.Succeeded())
	{
		VfxController = Cast<UPlayerVfxController>(CreateDefaultSubobject(TEXT("VFX Controller"), VfxControllerClass.Class, VfxControllerClass.Class, true, true ));
	}
	else
	{
		/** Construct the base class if the Blueprint derived class cannot be found. */
		VfxController = CreateDefaultSubobject<UPlayerVfxController>(TEXT("VFX Controller"));
	}
	VfxController->bEditableWhenInherited = false;
}

/** Called after the constructor but before the components are initialized. */
void APlayerCharacter::PostInitProperties()
{
	ValidateConfigurationAssets();
	
	if(UPlayerCharacterMovementComponent* PlayerCharacterMovementComponent {Cast<UPlayerCharacterMovementComponent>(GetCharacterMovement())})
	{
		PlayerCharacterMovement = PlayerCharacterMovementComponent;
	}
	else if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, FLT_MAX, FColor::Red, "PlayerCharacter failed to initialize PlayerCharacterMovementComponent.");
	}

	/** Set components to call their virtual InitializeComponent functions. */
	CameraController->bWantsInitializeComponent = true;
	// FlashlightController->bWantsInitializeComponent = true;
	AudioController->bWantsInitializeComponent = true;
	VfxController->bWantsInitializeComponent = true;
	
	Super::PostInitProperties();
}

/** Called after all default property values have been fully initialized, but before any of the components are initialized. */
void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	/** Registers this player character to the player character subsystem. */
	if(const UWorld* World {GetWorld()})
	{
		if(UPlayerSubsystem* PlayerSubsystem {World->GetSubsystem<UPlayerSubsystem>()})
		{
			PlayerSubsystem->RegisterPlayerCharacter(this);
		}
	}
	
	Super::OnConstruction(Transform);
}

/** Called after InitializeComponents. */
void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	ApplyConfigurationAssets();

	/** Subscribe to the OnLanding event of the player character movement component. */
	if(PlayerCharacterMovement)
	{
		PlayerCharacterMovement->OnLanding.AddDynamic(this, &APlayerCharacter::HandleLanding);
	}
}

/** Called when the game starts or when spawned. */
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	/** Notify the GameMode that the character has Begun Play. */
	if(GetWorld() && GetWorld()->GetAuthGameMode())
	{
		if(AFrostbiteGameMode* GameMode {Cast<AFrostbiteGameMode>(GetWorld()->GetAuthGameMode())})
		{
			GameMode->NotifyPlayerCharacterBeginPlay(this);
		}
	}

#if WITH_EDITOR
		/** Check if all components have been succesfully initialized. */
		ValidateObject(CameraController, "CameraController");
		ValidateObject(AudioController, "AudioController");
		ValidateObject(VfxController, "VfxController");
		ValidateObject(BodyAudioComponent, "BodyAudioComponent");
#endif
}

/** Called when the controller is changed. */
void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if(NewController)
	{
		PlayerCharacterController = Cast<APlayerCharacterController>(NewController);
		if(APlayerController* PlayerController {Cast<APlayerController>(NewController)})
		{
			StateConfiguration->ApplyToPlayerController(PlayerController);
		}
	}
}

/** Called every frame. */
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateYawDelta();
	UpdateRotation(DeltaTime);
}

void APlayerCharacter::UpdateYawDelta()
{
	double Delta {GetBaseAimRotation().Yaw - GetActorRotation().Yaw};
	Delta = FRotator::NormalizeAxis(Delta);
	YawDelta = Delta;
}

void APlayerCharacter::UpdateRotation(const float& DeltaTime)
{
	const UCharacterMovementComponent* MovementComponent {GetCharacterMovement()};
	if(MovementComponent && ((MovementComponent->IsMovingOnGround() && abs(GetVelocity().X) > 1) || MovementComponent->IsFalling()))
	{
		if(GetController())
		{
			SetActorRotation(FRotator(0, GetController()->GetControlRotation().Yaw, 0));
		}
		IsTurningInPlace = false;
	}
	else
	{
		constexpr float YawDeltaThreshold {30.0f};
		
		if(IsTurningInPlace)
		{
			AddActorWorldRotation(FRotator(0, CalculateTurnInPlaceRotation(YawDelta, DeltaTime, 4.f, 45.0f), 0));
		}
		if(FMath::IsNearlyEqual(YawDelta, 0, 0.5f))
		{
			IsTurningInPlace = false;
		}
		else if(abs(YawDelta) > YawDeltaThreshold)
		{
			IsTurningInPlace = true;
		}
	}
}

float APlayerCharacter::CalculateTurnInPlaceRotation(const float YawDelta, const float DeltaTime, const float Factor, const float Clamp)
{
	float Rotation {YawDelta * Factor * DeltaTime};
	if(abs(YawDelta) >= Clamp)
	{
		float RotationOvershoot {abs(YawDelta) - Clamp};
		RotationOvershoot = (YawDelta >= 0.0) ? RotationOvershoot : -RotationOvershoot;
		Rotation += RotationOvershoot;
	}
	return Rotation;
}

#if WITH_EDITOR
void APlayerCharacter::ValidateObject(const UObject* Object, const FString ObjectName)
{
	if(!Object)
	{
		UE_LOG(LogPlayerCharacter, Error, TEXT("%s was not properly initialized during the construction of the PlayerCharacter."), *ObjectName); \
	}
}
#endif

void APlayerCharacter::ValidateConfigurationAssets()
{
	/** If the configuration properties are not properly serialized, construct a default instance instead. */
	if(!CharacterConfiguration)
	{
		CharacterConfiguration = NewObject<UPlayerCharacterConfiguration>();
		if(GIsEditor && FApp::IsGame())
		{
			UE_LOG(LogPlayerCharacter, Warning, TEXT("No Character Configuration was selected for player character. Using default settings instead."))
		}
	}
	if(!StateConfiguration)
	{
		StateConfiguration = NewObject<UPlayerStateConfiguration>();
		if(GIsEditor && FApp::IsGame())
		{
			UE_LOG(LogPlayerCharacter, Warning, TEXT("No PlayerState Configuration was selected for player character. Using default settings instead."))
		}
	}
}

void APlayerCharacter::HandleLanding(EPlayerLandingType Value)
{
	float StunDuration {0.0f};
	switch(Value)
	{
	case EPlayerLandingType::Soft:
		return;
		
	case EPlayerLandingType::Hard:
		StunDuration = 1.85f;
		break;
	case EPlayerLandingType::Heavy:
		StunDuration = 4.5f;
		break;
	}

	if(const UWorld* World {GetWorld()})
	{
		if(UPlayerSubsystem* Subsystem {World->GetSubsystem<UPlayerSubsystem>()})
		{
			Subsystem->SetPlayerMovementInputLock(true);
			Subsystem->SetPlayerRotationInputLock(true);
		}
		GetWorld()->GetTimerManager().SetTimer(FallStunTimer, this, &APlayerCharacter::HandleLandingEnd, StunDuration, false);
	}
	GetCharacterMovement()->StopMovementImmediately();
	
	
}

void APlayerCharacter::HandleLandingEnd()
{
	if(PlayerCharacterController)
	{
		if(const UWorld* World {GetWorld()})
		{
			if(UPlayerSubsystem* Subsystem {World->GetSubsystem<UPlayerSubsystem>()})
			{
				Subsystem->SetPlayerMovementInputLock(false);
				Subsystem->SetPlayerRotationInputLock(false);
			}
		}
	}
}

void APlayerCharacter::ApplyConfigurationAssets()
{
	if(CharacterConfiguration)
	{
		CharacterConfiguration->ApplyToPlayerCharacter(this);
	}
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(const UWorld* World {GetWorld()})
	{
		if(UPlayerSubsystem* Subsystem {World->GetSubsystem<UPlayerSubsystem>()})
		{
			Subsystem->UnregisterPlayerCharacter(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}





















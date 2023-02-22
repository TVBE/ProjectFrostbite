// Copyright 2023 Barrelhouse


#include "ExteriorWindAudioComponent.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"

// Sets default values for this component's properties
UExteriorWindAudioComponent::UExteriorWindAudioComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	
	PrimaryComponentTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<UMetaSoundSource> MetaSoundSourceObject(TEXT("/Script/MetasoundEngine.MetaSoundSource'/Game/Game/Audio/Sources/Environment/Wind/MSS_ProceduralWind_Quad.MSS_ProceduralWind_Quad'"));
	if(MetaSoundSourceObject.Succeeded())
	{
		MetaSoundSource = MetaSoundSourceObject.Object;
	}

	if(GetOwner())
	{
		// AudioComponent = GetOwner()->AddComponentByClass<UAudioComponent>(UAudioComponent::StaticClass());
	}
	
}


// Called when the game starts
void UExteriorWindAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UExteriorWindAudioComponent::InitializeComponent()
{
	Super::InitializeComponent();
}


// Called every frame
void UExteriorWindAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


// Fill out your copyright notice in the Description page of Project Settings.

#include "LightSwitchCodeOnly.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
ALightSwitchCodeOnly::ALightSwitchCodeOnly()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    DesiredIntensity = 3000.0f;

    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Point Light"));
    PointLight->Intensity = DesiredIntensity;
    RootComponent = PointLight;

    LightSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Light Sphere"));
    LightSphere->InitSphereRadius(250.0f);
    LightSphere->SetupAttachment(RootComponent);

    LightSphere->OnComponentBeginOverlap.AddDynamic(this, &ALightSwitchCodeOnly::OnOverlapBegin);       // set up a notification for when this component overlaps something
    LightSphere->OnComponentEndOverlap.AddDynamic(this, &ALightSwitchCodeOnly::OnOverlapEnd);       // set up a notification for when this component overlaps something
}

// Called when the game starts or when spawned
void ALightSwitchCodeOnly::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALightSwitchCodeOnly::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALightSwitchCodeOnly::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        ToggleLight();
    }
}

void ALightSwitchCodeOnly::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        ToggleLight();
    }
}

void ALightSwitchCodeOnly::ToggleLight()
{
    PointLight->ToggleVisibility();
}




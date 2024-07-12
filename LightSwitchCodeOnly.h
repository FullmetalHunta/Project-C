// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightSwitchCodeOnly.generated.h"

UCLASS()
class JORDANSIMS_API ALightSwitchCodeOnly : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALightSwitchCodeOnly();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	/** point light component */
	UPROPERTY(VisibleAnywhere, Category = "Switch Components")
		class UPointLightComponent* PointLight;

	/** sphere component */
	UPROPERTY(VisibleAnywhere, Category = "Switch Components")
		class USphereComponent* LightSphere;


	/** called when something enters the sphere component */
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** called when something leaves the sphere component */
	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Toggles the light component's visibility*/
	UFUNCTION()
		void ToggleLight();

	/** the desired intensity for the light */
	UPROPERTY(VisibleAnywhere, Category = "Switch Variables")
		float DesiredIntensity;
};

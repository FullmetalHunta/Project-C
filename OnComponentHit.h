// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "OnComponentHit.generated.h"

class UBoxComponent;

UCLASS()
class JORDANSIMS_API AOnComponentHit : public AActor
{
		GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
		AOnComponentHit();

protected:
	virtual void BeginPlay() override; 

public:
	virtual void Tick(float DeltaTime) override; //called every frame//

	UPROPERTY(VisibleAnywhere)
			class UBoxComponent* MyComp; //calls UBoxComp//

	UFUNCTION()
			void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit); //declares void when register hit//

};

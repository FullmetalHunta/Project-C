// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraDirector.generated.h"

UCLASS()
class JORDANSIMS_API ACameraDirector : public AActor
{
	GENERATED_BODY()
		

public:	
	// Sets default values for this actor's properties
	ACameraDirector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//This is where unreal allows the two cameras I created to be used for the switching. In this case it's the sphere and the camera actor.
	UPROPERTY(EditAnywhere)
		AActor* CameraOne;

	UPROPERTY(EditAnywhere)
		AActor* CameraTwo;

	void cameraGo();

	float TimeToNextCameraChange;
	bool cameraChange;
};

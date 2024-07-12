// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI_Bot_Controller.generated.h"

/**
 * 
 */
UCLASS()
class JORDANSIMS_API AAI_Bot_Controller : public AAIController
{
	GENERATED_BODY()

public:
	AAI_Bot_Controller();

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* Pawn) override; //It is ran when it is plugged into the character.

	virtual void Tick(float DeltaSeconds) override;

	virtual FRotator GetControlRotation() const override; //it allows for the AI to always point to where the character is facing.

	UFUNCTION()
	void OnPawnDetected(const TArray<AActor*>& DetectedPawns);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float AISightRadius = 500.0f; //How far the AI  can see the character.

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float AISightAge = 5.0f; //How long the AI has sees the character.

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float AILoseSightRadius = AISightRadius + 50.0f; //Amount in which the AI loses its view on the character.

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float AIFieldofView = 90.0f; //This is how long the radius of the AI's field of view is.

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		class UAISenseConfig_Sight* SightConfig;







	
};

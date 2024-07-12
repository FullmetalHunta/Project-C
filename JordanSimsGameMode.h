// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JordanSimsCharacter.h"
#include "JordanSimsGameMode.generated.h"


UENUM()
enum class EGamePlayState
{
	EPlaying,
	EGameOver,
	EUnknown
};
UCLASS(minimalapi)
class AJordanSimsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJordanSimsGameMode();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	AJordanSimsCharacter* MyCharacter;

	UFUNCTION(BlueprintPure, Category = "Health")
		EGamePlayState GetCurrentState() const;

	void SetCurrentState(EGamePlayState NewState);

private:

	EGamePlayState CurrentState;

	void HandleNewState(EGamePlayState NewState); //makes it so the game can function with the damage//

};




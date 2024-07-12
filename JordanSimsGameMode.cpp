// Copyright Epic Games, Inc. All Rights Reserved.

#include "JordanSimsGameMode.h"
#include "JordanSimsHUD.h"
#include "Kismet/GameplayStatics.h"
#include "JordanSimsCharacter.h"
#include "UObject/ConstructorHelpers.h"


AJordanSimsGameMode::AJordanSimsGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AJordanSimsHUD::StaticClass();
}

void AJordanSimsGameMode::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentState(EGamePlayState::EPlaying);

	MyCharacter = Cast<AJordanSimsCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void AJordanSimsGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MyCharacter)
	{
		if (FMath::IsNearlyZero(MyCharacter->GetHealth(), 0.001f)) //It makes it so the character has a few seconds before taking damage again//
		{
			SetCurrentState(EGamePlayState::EGameOver);
		}
	}
}

EGamePlayState AJordanSimsGameMode::GetCurrentState() const
{
	return CurrentState;
}

void AJordanSimsGameMode::SetCurrentState(EGamePlayState NewState)
{
	CurrentState = NewState;
	HandleNewState(CurrentState);
}

void AJordanSimsGameMode::HandleNewState(EGamePlayState NewState)
{
	switch (NewState)
	{
	case EGamePlayState::EPlaying:
		{
			// do nothing
		}
		break;

	case EGamePlayState::EGameOver:
	{
		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
	}
	break;

	default:
	case EGamePlayState::EUnknown:
	{
		// do nothing
	}
	break;
	}
}
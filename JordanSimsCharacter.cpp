// Copyright Epic Games, Inc. All Rights Reserved.

#include "JordanSimsCharacter.h"
#include "JordanSimsProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Engine.h" 
#include "CameraDirector.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"



DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AJordanSimsCharacter


AJordanSimsCharacter::AJordanSimsCharacter()

{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	JumpHeight = 600.f;
}

void AJordanSimsCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	FullHealth = 1000.0f;
	Health = FullHealth;
	HealthPercentage = 1.0f;
	bool CanBeDamaged = true;

	FullMagic = 100.0f;
	Magic = FullMagic;
	MagicPercentage = 1.0f;
	PreviousMagic = MagicPercentage;
	MagicValue = 0.0f;
	bCanUseMagic = true;

	if (MagicCurve)
	{
		FOnTimelineFloat TimelineCallback;
		FOnTimelineEventStatic TimelineFinishedCallback;

		TimelineCallback.BindUFunction(this, FName("SetMagicValue"));
		TimelineFinishedCallback.BindUFunction(this, FName("SetMagicState"));
		MyTimeline.AddInterpFloat(MagicCurve, TimelineCallback);
		MyTimeline.SetTimelineFinishedFunc(TimelineFinishedCallback);
	}



	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

void AJordanSimsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MyTimeline.TickTimeline(DeltaTime); 
}
//////////////////////////////////////////////////////////////////////////
// Input

void AJordanSimsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{


	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AJordanSimsCharacter::DoubleJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AJordanSimsCharacter::OnFire);

	//Bind Camera event//
	PlayerInputComponent->BindAction("CameraToggle", IE_Released, this, &AJordanSimsCharacter::OnCameraSwitch);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);
	
	
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AJordanSimsCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AJordanSimsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AJordanSimsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AJordanSimsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AJordanSimsCharacter::LookUpAtRate);

}

void AJordanSimsCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != nullptr && !FMath::IsNearlyZero(Magic, 0.001f) && bCanUseMagic)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AJordanSimsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AJordanSimsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}

		// try and play the sound if specified
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation != nullptr)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}

		MyTimeline.Stop();
		GetWorldTimerManager().ClearTimer(MagicTimerHandle);
		SetMagicChange(-20.0f);
		GetWorldTimerManager().SetTimer(MagicTimerHandle, this, &AJordanSimsCharacter::UpdateMagic, 5.0f, false); //makes it so the magic meter goes down//
	}	
}
void AJordanSimsCharacter::OnCameraSwitch()
{
	cameraToggle();
}

void AJordanSimsCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AJordanSimsCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AJordanSimsCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AJordanSimsCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}


void AJordanSimsCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AJordanSimsCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AJordanSimsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AJordanSimsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AJordanSimsCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AJordanSimsCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AJordanSimsCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AJordanSimsCharacter::TouchUpdate);
		return true;
	}

	return false;
}
void AJordanSimsCharacter::Landed(const FHitResult& Hit)
{
	DoubleJumpCounter = 0;
}
void AJordanSimsCharacter::DoubleJump()
{
	if (DoubleJumpCounter <= 1)
	{
		ACharacter::LaunchCharacter(FVector(0, 0, JumpHeight), false, true);
		DoubleJumpCounter++;
	}
}
void AJordanSimsCharacter::cameraToggle()
{
	UE_LOG(LogTemp, Warning, TEXT("Pressed C Button: Activated by character pawn."));
	ACameraDirector* CameraDirectorPointer = Cast<ACameraDirector>(CameraDirector);

	CameraDirectorPointer->cameraGo();
	
}

float AJordanSimsCharacter::GetHealth()
{
	return HealthPercentage;
}

float AJordanSimsCharacter::GetMagic()
{
	return MagicPercentage;
}

FText AJordanSimsCharacter::GetHealthIntText()
{
	int32 HP = FMath::RoundHalfFromZero(HealthPercentage * 100);
	FString HPS = FString::FromInt(HP);
	FString HealthHUD = HPS + FString(TEXT("%"));
	FText HPTEXT = FText::FromString(HealthHUD);
	return HPTEXT;
}

FText AJordanSimsCharacter::GetMagicIntText()
{
	int32 MP = FMath::RoundHalfFromZero(MagicPercentage * 100);
	FString MPS = FString::FromInt(MP);
	FString FullMPS = FString::FromInt(FullMagic);
	FString MagicHUD = MPS + FString(TEXT("/") + FullMPS);
	FText MagicTEXT = FText::FromString(MagicHUD);
	return MagicTEXT;
}

void AJordanSimsCharacter::SetDamageState()
{
	bool CanBeDamaged = true;
}

void AJordanSimsCharacter::DamageTimer()
{
	GetWorldTimerManager().SetTimer(MemberTimerHandle, this, &AJordanSimsCharacter::SetDamageState, 2.0f, false); //this is also where it determines how long the invinsible frames last//
}

void AJordanSimsCharacter::SetMagicValue()
{
	TimelineValue = MyTimeline.GetPlaybackPosition();
	CurveFloatValue = PreviousMagic + MagicValue * MagicCurve->GetFloatValue(TimelineValue);
	Magic = CurveFloatValue * FullHealth;
	Magic = FMath::Clamp(Magic, 0.0f, FullMagic);
	MagicPercentage = CurveFloatValue;
	MagicPercentage = FMath::Clamp(MagicPercentage, 0.0f, 1.0f);
}

void AJordanSimsCharacter::SetMagicState()
{
	bCanUseMagic = true;
	MagicValue = 0.0;
	if (GunDefaultMaterial)
	{
		FP_Gun->SetMaterial(0, GunDefaultMaterial);
	}
}

bool AJordanSimsCharacter::PlayFlash()
{
	if (redFlash)
	{
		redFlash = false;
		return true;
	}
	return false;
}

float AJordanSimsCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) //makes it so the character can actually get damaged//
{
	bool CanBeDamaged = false;
	redFlash = true;
	UpdateHealth(-DamageAmount);
	DamageTimer();
	return DamageAmount;
}

void AJordanSimsCharacter::UpdateHealth(float HealthChange)
{
	Health += HealthChange;
	Health = FMath::Clamp(Health, 0.0f, FullHealth);
	HealthPercentage = Health/FullHealth;
}

void AJordanSimsCharacter::UpdateMagic()
{
	PreviousMagic = MagicPercentage;
	MagicPercentage = Magic/FullMagic;
	MagicValue = 1.0f;
	MyTimeline.PlayFromStart();
}

void AJordanSimsCharacter::SetMagicChange(float MagicChange)
{
	bCanUseMagic = false;
	PreviousMagic = MagicPercentage;
	MagicValue = MagicChange/FullMagic;
	if (GunOverheatMaterial)
	{
		FP_Gun->SetMaterial(0, GunOverheatMaterial);
	}

	MyTimeline.PlayFromStart();
}
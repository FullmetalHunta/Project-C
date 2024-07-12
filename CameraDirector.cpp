


#include "CameraDirector.h"
#include "Kismet/GameplayStatics.h" //allows for more information to be used for the game.
// Sets default values
ACameraDirector::ACameraDirector()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACameraDirector::BeginPlay()
{
	Super::BeginPlay();
    PrimaryActorTick.bCanEverTick = true;

    cameraChange = false;

    UE_LOG(LogTemp, Warning, TEXT("Camera Director BeginPlay"));
}

// Called every frame
void ACameraDirector::Tick(float DeltaTime) {

    Super::Tick(DeltaTime);
    //Here is where it dictates when the camera will switch to the other one.
    const float TimeBetweenCameraChanges = 2.0f;
    const float SmoothBlendTime = 0.75f;
    TimeToNextCameraChange -= DeltaTime;

    if (cameraChange)
    {
        if (TimeToNextCameraChange <= 0.0f) {
            TimeToNextCameraChange += TimeBetweenCameraChanges;

            APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);

            if (OurPlayerController) {


                if (CameraTwo && (OurPlayerController->GetViewTarget() == CameraOne)) {
                    OurPlayerController->SetViewTargetWithBlend(CameraTwo, SmoothBlendTime);
                }
                else if (CameraOne) {
                    OurPlayerController->SetViewTargetWithBlend(CameraOne, SmoothBlendTime);
                }

            }
        }
        else {
            cameraChange = false;
            TimeToNextCameraChange = 0.0f;
        }
    }
}
    
    
       
            

void ACameraDirector::cameraGo()
{
    cameraChange = true;
    TimeToNextCameraChange = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("cameraGO function activated!"));
}
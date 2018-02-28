// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CTFProjectCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

//////////////////////////////////////////////////////////////////////////
// ACTFProjectCharacter

ACTFProjectCharacter::ACTFProjectCharacter()
{
   // Set size for collision capsule
   GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

   // set our turn rates for input
   BaseTurnRate = 45.f;
   BaseLookUpRate = 45.f;

   // Don't rotate when the controller rotates. Let that just affect the camera.
   bUseControllerRotationPitch = false;
   bUseControllerRotationYaw = false;
   bUseControllerRotationRoll = false;

   // Configure character movement
   GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...   
   GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
   GetCharacterMovement()->JumpZVelocity = 600.f;
   GetCharacterMovement()->AirControl = 0.2f;

   // Create a camera boom (pulls in towards the player if there is a collision)
   CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
   CameraBoom->SetupAttachment(RootComponent);
   CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character   
   CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

   // Create a follow camera
   FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
   FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
   FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

   
   ConstructorHelpers::FObjectFinder<UBlueprint> WallAsset(TEXT("Blueprint'/Game/ThirdPersonCPP/Blueprints/Placeable/Wall.Wall'"));
   Wall = (UClass*)WallAsset.Object->GeneratedClass;

   ConstructorHelpers::FObjectFinder<UBlueprint> CubeAsset(TEXT("Blueprint'/Game/ThirdPersonCPP/Blueprints/Placeable/Cube.Cube'"));
   Cube = (UClass*)CubeAsset.Object->GeneratedClass;

   Reach = 1500.f;

   numWalls = 10;
   numCubes = 10;

   // Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
   // are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACTFProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
   // Set up gameplay key bindings
   check(PlayerInputComponent);
   PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
   PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

   PlayerInputComponent->BindAxis("MoveForward", this, &ACTFProjectCharacter::MoveForward);
   PlayerInputComponent->BindAxis("MoveRight", this, &ACTFProjectCharacter::MoveRight);

   // We have 2 versions of the rotation bindings to handle different kinds of devices differently
   // "turn" handles devices that provide an absolute delta, such as a mouse.
   // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
   PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
   PlayerInputComponent->BindAxis("TurnRate", this, &ACTFProjectCharacter::TurnAtRate);
   PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
   PlayerInputComponent->BindAxis("LookUpRate", this, &ACTFProjectCharacter::LookUpAtRate);

   // handle touch devices
   PlayerInputComponent->BindTouch(IE_Pressed, this, &ACTFProjectCharacter::TouchStarted);
   PlayerInputComponent->BindTouch(IE_Released, this, &ACTFProjectCharacter::TouchStopped);

   // VR headset functionality
   PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACTFProjectCharacter::OnResetVR);

   PlayerInputComponent->BindAction("PlaceWall", IE_Pressed, this, &ACTFProjectCharacter::PlaceWall);
   PlayerInputComponent->BindAction("PlaceCube", IE_Pressed, this, &ACTFProjectCharacter::PlaceCube);
}

void ACTFProjectCharacter::PlaceWall()
{
	if (numWalls > 0) {
		FVector SpawnLocation = GetSpawnLocation();
		if (SpawnLocation.IsZero()) {

		}
		else {
			APlaceable* MyPlaceable = GetWorld()->SpawnActor<APlaceable>(Wall, SpawnLocation, FRotator(0, Controller->GetControlRotation().Yaw, 0), *(new FActorSpawnParameters));
			numWalls--;
		}
	}
}

void ACTFProjectCharacter::PlaceCube()
{
	if (numCubes > 0) {
		FVector SpawnLocation = GetSpawnLocation();
		if (SpawnLocation.IsZero()) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, SpawnLocation.ToString());
		}
		else {
			APlaceable* MyPlaceable = GetWorld()->SpawnActor<APlaceable>(Cube, SpawnLocation, FRotator(0, Controller->GetControlRotation().Yaw, 0), *(new FActorSpawnParameters));
			numCubes--;
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, SpawnLocation.ToString());
		}
	}
}

FVector ACTFProjectCharacter::GetSpawnLocation()
{
	FHitResult LinetraceHit;
   FVector StartTrace = FollowCamera->GetComponentLocation();
   FVector EndTrace = (FollowCamera->GetForwardVector() * Reach) + StartTrace;

   FCollisionQueryParams CQP;
   CQP.AddIgnoredActor(this);

   GetWorld()->LineTraceSingleByChannel(LinetraceHit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldDynamic, CQP);

   AActor* PotentialObject = Cast<AActor>(LinetraceHit.GetActor());

   if (PotentialObject != NULL) {
	  return LinetraceHit.ImpactPoint - 20.0f * FollowCamera->GetForwardVector();
   }
   else {
	   return FVector(0.f);
   }
   
}

void ACTFProjectCharacter::OnResetVR()
{
   UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACTFProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
      Jump();
}

void ACTFProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
      StopJumping();
}

void ACTFProjectCharacter::TurnAtRate(float Rate)
{
   // calculate delta for this frame from the rate information
   AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACTFProjectCharacter::LookUpAtRate(float Rate)
{
   // calculate delta for this frame from the rate information
   AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACTFProjectCharacter::MoveForward(float Value)
{
   if ((Controller != NULL) && (Value != 0.0f))
   {
      // find out which way is forward
      const FRotator Rotation = Controller->GetControlRotation();
      const FRotator YawRotation(0, Rotation.Yaw, 0);

      // get forward vector
      const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
      AddMovementInput(Direction, Value);
   }
}

void ACTFProjectCharacter::MoveRight(float Value)
{
   if ( (Controller != NULL) && (Value != 0.0f) )
   {
      // find out which way is right
      const FRotator Rotation = Controller->GetControlRotation();
      const FRotator YawRotation(0, Rotation.Yaw, 0);
   
      // get right vector 
      const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
      // add movement in that direction
      AddMovementInput(Direction, Value);
   }
}

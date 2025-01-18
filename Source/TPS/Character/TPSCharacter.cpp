// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Kismet\GameplayStatics.h"
#include "Kismet\KismetMathLibrary.h"
#include "Engine/World.h"

ATPSCharacter::ATPSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATPSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	MovementTick(DeltaSeconds);
}

void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);
	
	NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATPSCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATPSCharacter::InputAxisY);
}

void ATPSCharacter::InputAxisX(float Value)
{
	AxisX = Value;
}

void ATPSCharacter::InputAxisY(float Value)
{
	AxisY = Value;
}

void ATPSCharacter::MovementTick(float DeltaTime)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);

	if (MovementState == EMovementState::SprintRun_State)
	{
		FVector MyRotationVector = FVector(AxisX, AxisY, 0.0f);
		FRotator MyRotator = MyRotationVector.ToOrientationRotator();
		SetActorRotation(FQuat(MyRotator));
	}

	APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (MyController)
	{
		FHitResult ResultHit;
		MyController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);
		
		float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
		SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));
	}
}

void ATPSCharacter::CharacterUpdate()
{
	float ResSpeed = 600.0f;
	switch (MovementState)
	{
	case EMovementState::Aim_State:
		ResSpeed = MovementSpeedInfo.AimSpeedNormal;
		break;
	case EMovementState::AimWalk_State:
		ResSpeed = MovementSpeedInfo.AimSpeedWalk;
		break;
	case EMovementState::Walk_State:
		ResSpeed = MovementSpeedInfo.WalkSpeedNormal;
		break;
	case EMovementState::Run_State:
		ResSpeed = MovementSpeedInfo.RunSpeedNormal;
		break;
	case EMovementState::SprintRun_State:
		ResSpeed = MovementSpeedInfo.SprintRunSpeedRun;
		break;
	default:
		break;
	}

	GetCharacterMovement()->MaxWalkSpeed = ResSpeed;
}

void ATPSCharacter::ChangeMovementeState(EMovementState NewMovementState)
{
	// Ничего не нажато
	if (!bWalkEnabled && !bSprintRunEnabled && !bAimEnabled)
	{
		MovementState = EMovementState::Run_State;
	}
	else
	{
		// Нажат спринт
		if (bSprintRunEnabled)
		{
			bWalkEnabled = false;
			bAimEnabled = false;
			MovementState = EMovementState::SprintRun_State;
		}
		// Прицелились и ходим
		if (bWalkEnabled && !bSprintRunEnabled && bAimEnabled)
		{
			MovementState = EMovementState::AimWalk_State;
		}
		else
		{
			// Только хотьба
			if (bWalkEnabled && !bSprintRunEnabled && !bAimEnabled)
			{
				MovementState = EMovementState::Walk_State;
			}
			else
			{
				//Только прицеливание
				if (!bWalkEnabled && !bSprintRunEnabled && bAimEnabled)
				{
					MovementState = EMovementState::Aim_State;
				}
			}
		}
	}
	CharacterUpdate();
}

bool ATPSCharacter::IsForwardMove()
{
	FVector FnVector = GetActorForwardVector();
	FVector FlVector = GetLastMovementInputVector();

	return FVector::DotProduct(FnVector, FlVector) > 0.9f;
}

/*
void ATPSCharacter::InputMouseWheel(float DeltaTime)
{
	// Зум колесика мыши. Код не работает Разобраться!!!
	if ((Height - (HeightMouse * SpeedZoom) > MinHeightZoom - 1) && (Height - (HeightMouse * SpeedZoom) < MaxHeightZoom + 1))
	{
		Height = Height - (HeightMouse * SpeedZoom);
		CameraBoom->TargetArmLength = UKismetMathLibrary::FInterpTo(CameraBoom->TargetArmLength, Height, DeltaTime, 1);
	}
}
*/
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
//#include "HeadMoutedDisplayFunctionLibrary.h"
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

	CameraDistance = 800;
	MinCameraDistance = CameraDistance - 500;
	MaxCameraDistance = CameraDistance + 500;
	CameraSlideSpeed = 50;

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

	if (CurrentCursor)
	{
		APlayerController* MyPc = Cast<APlayerController>(GetController());
		if (MyPc)
		{
			FHitResult TraceHitResult;
			MyPc->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFv = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFv.Rotation();

			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitWeapon();

	if (CursorMaterial)
	{
		{
			CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
		}
	}

}

void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);
	
	NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATPSCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATPSCharacter::InputAxisY);
	NewInputComponent->BindAxis(TEXT("MouseWheel"), this, &ATPSCharacter::InputMouseWheel);
	
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATPSCharacter::InputsAttackPressed);
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATPSCharacter::InputsAttackReleased);
	
}

void ATPSCharacter::InputAxisX(float Value)
{
	AxisX = Value;
}

void ATPSCharacter::InputAxisY(float Value)
{
	AxisY = Value;
}

void ATPSCharacter::InputsAttackPressed()
{
	AttackCharEvent(true);
}

void ATPSCharacter::InputsAttackReleased()
{
	AttackCharEvent(false);
}

void ATPSCharacter::MovementTick(float DeltaTime)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);
	CameraSlideTick(DeltaTime);

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
		//MyController->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, ResultHit);
		
		float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
		SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));
	}
}

void ATPSCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* MyWeapon = nullptr;
	MyWeapon = GetCurrentWeapon();
	if (MyWeapon)
	{
		//ToDo Check melee or range
		MyWeapon->SetWeaponSteteFire(bIsFiring);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
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
	
	//Weapon state update
	AWeaponDefault* MyWeapon = GetCurrentWeapon();
	if (MyWeapon)
	{
		MyWeapon->UpdateStateWeapon(MovementState);
	}

	
}

AWeaponDefault* ATPSCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATPSCharacter::InitWeapon() //ToDo Init by row  by table
{
	if (InitWeaponClass)
	{
		FVector SpawnLocation = FVector(0);
		FRotator SpawnRotation = FRotator(0);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = GetInstigator();

		AWeaponDefault* MyWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(InitWeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
		if (MyWeapon)
		{
			FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
			MyWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
			CurrentWeapon = MyWeapon;

			MyWeapon->UpdateStateWeapon(MovementState);
		}
	}
}


bool ATPSCharacter::IsForwardMove()
{
	FVector FnVector = GetActorForwardVector();
	FVector FlVector = GetLastMovementInputVector();

	return FVector::DotProduct(FnVector, FlVector) > 0.9f;
}

UDecalComponent* ATPSCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}


void ATPSCharacter::CameraSlideTick(float DeltaTime)
{
	// Зум колесика мыши. 
	if (CameraBoom == nullptr) return; // Если по какой-то причине отсутствует указатель на ArmLengthComponent  -- скипаем логику
	if (CameraDistance == CameraBoom->TargetArmLength) return; // Если желаемая высота не изменилась -- скипаем логику
	CameraDistance = FMath::Clamp(CameraDistance, MinCameraDistance, MaxCameraDistance);

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, CameraDistance, DeltaTime, 5.0f);
	
}

void ATPSCharacter::InputMouseWheel(float Value)
{
	if (Value != 0.0f) CameraDistance -= Value * CameraSlideSpeed;

}


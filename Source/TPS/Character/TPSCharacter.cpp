// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayTypes.h"
#include "Materials/Material.h"
#include "Kismet\GameplayStatics.h"
#include "Kismet\KismetMathLibrary.h"
#include "TPS/Game/TPSGameInstance.h"
#include "Engine/World.h"

ATPSCharacter::ATPSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
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
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
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

	InitWeapon(InitWeaponName);

	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
	}
}

void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);
	
	NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATPSCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATPSCharacter::InputAxisY);
	NewInputComponent->BindAxis(TEXT("MouseWheel"), this, &ATPSCharacter::InputMouseWheel);

	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATPSCharacter::InputAttackPressed);
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATPSCharacter::InputAttackReleased);
	NewInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &ATPSCharacter::TryReloadWeapon);

}

void ATPSCharacter::InputAxisX(float Value)
{
	AxisX = Value;
}

void ATPSCharacter::InputAxisY(float Value)
{
	AxisY = Value;
}

void ATPSCharacter::InputAttackPressed()
{
	AttackCharEvent(true);
}

void ATPSCharacter::InputAttackReleased()
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
		if (!MyRotationVector.IsNearlyZero())							// �������a �� ������� ������
		{
			FRotator MyRotator = MyRotationVector.ToOrientationRotator();
			SetActorRotation(FQuat(MyRotator));
		}
		//FRotator MyRotator = MyRotationVector.ToOrientationRotator();
		//SetActorRotation(FQuat(MyRotator));
	}
	else
	{
		APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (MyController)
		{
			FHitResult ResultHit;
			MyController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);
			//MyController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);

			float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
			SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));

			if (CurrentWeapon)
			{
				FVector Displacement = FVector(0);
				switch (MovementState)
				{
				case EMovementState::Aim_State:
					Displacement = FVector(0.0f, 0.0f, 160.0f);
					CurrentWeapon->ShouldReduseDespersion = true;
					break;
				case EMovementState::AimWalk_State:
					Displacement = FVector(0.0f, 0.0f, 160.0f);
					CurrentWeapon->ShouldReduseDespersion = true;
					break;
				case EMovementState::Walk_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduseDespersion = false;
					break;
				case EMovementState::Run_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduseDespersion = false;
					break;
				case EMovementState::SprintRun_State:
					break;
				default:
					break;
				}

				//aim cursor like 3d widget?
				CurrentWeapon->ShootEndLocation = ResultHit.Location + Displacement;
			}
		}
	}

	
	
}

void ATPSCharacter::AttackCharEvent(bool bIsFiring)
{
	
	AWeaponDefault* MyWeapon = nullptr;
	MyWeapon = GetCurrentWeapon();
	if (MyWeapon)
	{
	
		//ToDo Check melee or range
		MyWeapon->SetWeaponStateFire(bIsFiring);
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

void ATPSCharacter::ChangeMovementState()
{
	
	// ������ �� ������
	if (!bWalkEnabled && !bSprintRunEnabled && !bAimEnabled)
	{
		MovementState = EMovementState::Run_State;
	}
	else
	{
		// ����� ������
		if (bSprintRunEnabled)
		{
			bWalkEnabled = false;
			bAimEnabled = false;
			MovementState = EMovementState::SprintRun_State;
		}
		// ����������� � �����
		if (bWalkEnabled && !bSprintRunEnabled && bAimEnabled)
		{
			MovementState = EMovementState::AimWalk_State;
		}
		else
		{
			// ������ ������
			if (bWalkEnabled && !bSprintRunEnabled && !bAimEnabled)
			{
				MovementState = EMovementState::Walk_State;
			}
			else
			{
				//������ ������������
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

void ATPSCharacter::InitWeapon(FName IdWeaponName)
{
	UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetGameInstance());
	FWeaponInfo MyWeaponInfo;
	if (MyGI)
	{
		if (MyGI->GetWeaponInfoByName(IdWeaponName, MyWeaponInfo))
		{
			FVector SpawnLocation = FVector(0);
			FRotator SpawnRotation = FRotator(0);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();

			AWeaponDefault* MyWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(MyWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
			if (MyWeapon)
			{
				FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
				MyWeapon->AttachToComponent(GetMesh(), Rule, FName("Weapon_R"));
				CurrentWeapon = MyWeapon;

				MyWeapon->WeaponSetting = MyWeaponInfo;
			//	MyWeapon->WeaponReloading = false;
				MyWeapon->WeaponInfo.Round = MyWeaponInfo.MaxRound;
				//Remove !! Debug
				MyWeapon->ReloadTime = MyWeaponInfo.ReloadTime;
				MyWeapon->UpdateStateWeapon(MovementState);

				// ����������� ��� �������
				UE_LOG(LogTemp, Warning, TEXT("Weapon AnimCharReload: %s"), *GetNameSafe(MyWeaponInfo.AnimCharReload));

				MyWeapon->OnWeaponReloadStart.AddDynamic(this, &ATPSCharacter::WeaponReloadStart);
				MyWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATPSCharacter::WeaponReloadEnd);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ATPSChatacter::InitWeapon - Weapon not found on table -NULL"));
	}
}

void ATPSCharacter::TryReloadWeapon()
{
		if (CurrentWeapon)    // ���������� ������� ��������.
		{
			if (CurrentWeapon->GetWeaponRound() <= CurrentWeapon->WeaponSetting.MaxRound)
			{
				CurrentWeapon->InitReload();
			}
		}
	
}

void ATPSCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	WeaponReloadStart_BP(Anim);
}

void ATPSCharacter::WeaponReloadEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("WeaponReloadEnd called!"));
	WeaponReloadEnd_BP();
}

void ATPSCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	//in BP
}

void ATPSCharacter::WeaponReloadEnd_BP_Implementation()
{
	//in BP
}

UDecalComponent* ATPSCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}

bool ATPSCharacter::IsForwardMove()
{
	FVector FnVector = GetActorForwardVector();
	FVector FlVector = GetLastMovementInputVector();

	return FVector::DotProduct(FnVector, FlVector) > 0.9f;
}


void ATPSCharacter::CameraSlideTick(float DeltaTime)
{
	// ��� �������� ����. 
	if (CameraBoom == nullptr) return; // ���� �� �����-�� ������� ����������� ��������� �� ArmLengthComponent  -- ������� ������
	if (CameraDistance == CameraBoom->TargetArmLength) return; // ���� �������� ������ �� ���������� -- ������� ������
	CameraDistance = FMath::Clamp(CameraDistance, MinCameraDistance, MaxCameraDistance);

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, CameraDistance, DeltaTime, 5.0f);
	
}

void ATPSCharacter::InputMouseWheel(float Value)
{
	if (Value != 0.0f) CameraDistance -= Value * CameraSlideSpeed;

}


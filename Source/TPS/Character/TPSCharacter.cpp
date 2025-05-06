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
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/TPSInventoryComponent.h"
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

	
	InventoryComponent = CreateDefaultSubobject<UTPSInventoryComponent>(TEXT("InventoryComponent"));
	
		if (InventoryComponent)
		{
			InventoryComponent->OnSwitchWeapon.AddDynamic(this, &ATPSCharacter::InitWeapon);
		}

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

	InitWeapon(InitWeaponName, FAdditionalWeaponInfo(), 0);

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

	NewInputComponent->BindAction(TEXT("SwitchNextWeapon"), EInputEvent::IE_Pressed, this, &ATPSCharacter::TrySwitchNextWeapon);
	NewInputComponent->BindAction(TEXT("SwitchPreviosWeapon"), EInputEvent::IE_Pressed, this, &ATPSCharacter::TrySwitchPreviosWeapon);

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
		if (!MyRotationVector.IsNearlyZero())							// проверкa на нулевой вектор
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
					CurrentWeapon->ShouldReduceDispersion = true;
					break;
				case EMovementState::AimWalk_State:
					Displacement = FVector(0.0f, 0.0f, 160.0f);
					CurrentWeapon->ShouldReduceDispersion = true;
					break;
				case EMovementState::Walk_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduceDispersion = false;
					break;
				case EMovementState::Run_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduceDispersion = false;
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

void ATPSCharacter::InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo WeaponAdditionalInfo, int32 NewCurrentIndexWeapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetGameInstance());
	FWeaponInfo MyWeaponInfo;
	if (MyGI)
	{
		if(MyGI->GetWeaponInfoByName(IdWeaponName, MyWeaponInfo))
			if (MyWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* MyWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(MyWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (MyWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					MyWeapon->AttachToComponent(GetMesh(), Rule, FName("Weapon_R"));
					CurrentWeapon = MyWeapon;

					MyWeapon->WeaponSetting = MyWeaponInfo;
					//MyWeapon->AdditionalWeaponInfo.Round = MyWeaponInfo.MaxRound;
					//	MyWeapon->WeaponReloading = false;
					//MyWeapon->WeaponInfo.Round = MyWeaponInfo.MaxRound;
					//Remove !! Debug
					MyWeapon->ReloadTime = MyWeaponInfo.ReloadTime;
					MyWeapon->UpdateStateWeapon(MovementState);

					MyWeapon->AdditionalWeaponInfo = WeaponAdditionalInfo;
					CurrentIndexWeapon = NewCurrentIndexWeapon;//fix

					//Not Forget remove delegate on change/drop weapon
					MyWeapon->OnWeaponReloadStart.AddDynamic(this, &ATPSCharacter::WeaponReloadStart);
					MyWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATPSCharacter::WeaponReloadEnd);

					MyWeapon->OnWeaponFireStart.AddDynamic(this, &ATPSCharacter::WeaponFireStart);

					// after switch try reload weapon if needed
					if (CurrentWeapon->GetWeaponRound() <= 0 && CurrentWeapon->CheckCanWeaponReload())
						CurrentWeapon->InitReload();

					if (InventoryComponent)
						InventoryComponent->OnWeaponAmmoAviable.Broadcast(MyWeapon->WeaponSetting.WeaponType);
				}

			}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ATPSChatacter::InitWeapon - Weapon not found on table -NULL"));
	}
}

void ATPSCharacter::RemoveCurrentWeapon()
{

}

void ATPSCharacter::TryReloadWeapon()
{
		if (CurrentWeapon && !CurrentWeapon->WeaponReloading)    // Fix Reload
		{
			if (CurrentWeapon->GetWeaponRound() < CurrentWeapon->WeaponSetting.MaxRound && CurrentWeapon->CheckCanWeaponReload())
			{
				CurrentWeapon->InitReload();
			}
		}
	
}

void ATPSCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	WeaponReloadStart_BP(Anim);
}

void ATPSCharacter::WeaponReloadEnd(bool bIsSuccess, int32 AmmoTake)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->AmmoSlotChangeValue(CurrentWeapon->WeaponSetting.WeaponType, AmmoTake);
		InventoryComponent->SetAdditionalInfoWeapon(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	}
	WeaponReloadEnd_BP(bIsSuccess);
}

void ATPSCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	//in BP
}

void ATPSCharacter::WeaponReloadEnd_BP_Implementation(bool bIsSuccess)
{
	//in BP
}

void ATPSCharacter::WeaponFireStart(UAnimMontage* Anim)
{
	if (InventoryComponent && CurrentWeapon)
		InventoryComponent->SetAdditionalInfoWeapon(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	WeaponFireStart_BP(Anim);
}

void ATPSCharacter::WeaponFireStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

UDecalComponent* ATPSCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}

//ToDO in one func TrySwitchPreviosWeapon && TrySwicthNextWeapon
//need Timer to Switch with Anim, this method stupid i must know switch success for second logic inventory
//now we not have not success switch/ if 1 weapon switch to self

void ATPSCharacter::TrySwitchNextWeapon()
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		//We have more then one weapon go switch
		int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndex(CurrentIndexWeapon + 1, OldIndex, OldInfo, true))
			{
			}
		}
	}
}

void ATPSCharacter::TrySwitchPreviosWeapon()
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		//We have more then one weapon go switch
		int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		if (InventoryComponent)
		{
			//InventoryComponent->SetAdditionalInfoWeapon(OldIndex, GetCurrentWeapon()->AdditionalWeaponInfo);
			if (InventoryComponent->SwitchWeaponToIndex(CurrentIndexWeapon - 1, OldIndex, OldInfo, false))
			{
			}
		}
	}
}

bool ATPSCharacter::IsForwardMove()
{
	FVector FnVector = GetActorForwardVector();
	FVector FlVector = GetLastMovementInputVector();

	return FVector::DotProduct(FnVector, FlVector) > 0.9f;
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


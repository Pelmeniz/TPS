#include "WeaponDefault.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

//Sets default values
AWeaponDefault::AWeaponDefault()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	if (!SceneComponent)																								/*Проверка на наличие компонента*/
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create SceneComponent!"));
		return;
	}
	RootComponent = SceneComponent;

	SkeletalMeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshWeapon->SetGenerateOverlapEvents(false);
	SkeletalMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshWeapon->SetupAttachment(RootComponent);

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(RootComponent);
}

//Called when the game starts or when spawned
void AWeaponDefault::BeginPlay()
{
	Super::BeginPlay();
	WeaponInit();
}

//Called every frame
void AWeaponDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FireTick(DeltaTime);
	ReloadTick(DeltaTime);
	DispersionTick(DeltaTime);
}

void AWeaponDefault::FireTick(float DeltaTime)
{
	
	if (GetWeaponRound() > 0)
	{
		if (WeaponFiring)
		{
			if (FireTimer < 0.0f)
			{
				if (!WeaponReloading)
				{
					Fire();
				}
			}
			else
			{
				FireTimer -= DeltaTime;
			}
		}
		else
		{
			if (!WeaponReloading)
			{
				InitReload();
			}

		}
	}
	if (GetWeaponRound() <= 0)											// Проверка на случай, если GetWeaponRound() вернет 0 или меньше. Для исключения неправильного поведения оружия.
	{
		InitReload();
		return;
	}
}

void AWeaponDefault::ReloadTick(float DeltaTime)
{
	if (WeaponReloading)
	{
		if (ReloadTimer < 0.0f)
		{
			FinishReload();
		}
		else
		{
			ReloadTimer -= DeltaTime;
		}
	}
}

void AWeaponDefault::DispersionTick(float DeltaTime)
{
	if (!WeaponReloading)
	{
		if (!WeaponFiring)
		{
			if (ShouldReduseDespersion)
			{
				CurrentDispersion = CurrentDispersion - CurrentDispersionReduction;
			}
			else
			{
				CurrentDispersion = CurrentDispersion + CurrentDispersionReduction;
			}
		}
		if (CurrentDispersion < CurrentDispersionMin)
		{
			CurrentDispersion = CurrentDispersionMin;
		}
		else
		{
			if (CurrentDispersion > CurrentDispersionMax)
			{
				CurrentDispersion = CurrentDispersionMax;
			}
		}
	}
	#if WITH_EDITORONLY_DATA
	if (ShowDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dispersion: MAX = %f. MIN = %f. Current = %f"), CurrentDispersionMax, CurrentDispersionMin, CurrentDispersion);
	}
	#endif
}

void AWeaponDefault::WeaponInit()
{
	if (SkeletalMeshWeapon && !SkeletalMeshWeapon->GetSkinnedAsset())   // Изменено под движок UE5 так как в нем используется новый способ.
	{
		SkeletalMeshWeapon->DestroyComponent(true);
	}

	if (StaticMeshWeapon && !StaticMeshWeapon->GetStaticMesh())
	{
		StaticMeshWeapon->DestroyComponent();
	}
	UpdateStateWeapon(EMovementState::Run_State);
}

void AWeaponDefault::SetWeaponStateFire(bool bIsFire)
{
	if (CheckWeaponCanFire())
	{
		WeaponFiring = bIsFire;
	}
	else
	{
		WeaponFiring = false;
		FireTimer = 0.01f;
	}

}

bool AWeaponDefault::CheckWeaponCanFire()
{
	return !BlockFire;
}


FProjectileInfo AWeaponDefault::GetProjectile()
{
	return WeaponSetting.ProjectileSetting;
}



void AWeaponDefault::Fire()
{
	FireTimer = WeaponSetting.RateOfFire;
	WeaponInfo.Round = WeaponInfo.Round - 1;
	ChangeDispersionByShot();

	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());

	int8 NumberProjectile = GetNumberProjectileByShot();

	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();

		FVector EndLocation;
		for (int8 i = 0; i < NumberProjectile; i++)
		{
			EndLocation = GetFireEndLocation();

			FVector Dir = EndLocation - SpawnLocation;
			Dir.Normalize();

			FMatrix MyMatrix(Dir, FVector(0, 0, 0), FVector(0, 0, 0), FVector::ZeroVector);
			SpawnRotation = MyMatrix.Rotator();

			if (ProjectileInfo.Projectile)
			{
				// Projectile Init ballistic fire

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AProjectileDefault* MyProjectile = Cast<AProjectileDefault>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (MyProjectile)
				{
					//ToDo Init Projectile settings by id in table row(or keep in weapon table)
					MyProjectile->InitProjectile(WeaponSetting.ProjectileSetting);
				}
			}
			else
			{
				//ToDo Projectile null Init trace fire
				// GetWorld()->LineTraceSingleByChanel();	
			}
		}
		
	}
}


void AWeaponDefault::UpdateStateWeapon(EMovementState NewMovementState)
{
	//ToDo Distersion
	BlockFire = false;

	switch (NewMovementState)
	{
	case EMovementState::Aim_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimReduction;
		break;
	case EMovementState::AimWalk_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimReduction;
		break;
	case EMovementState::Walk_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimReduction;
		break;
	case EMovementState::Run_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Run_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Run_StateDispersionAimReduction;
		break;
	case EMovementState::SprintRun_State:
		BlockFire = true;
		SetWeaponStateFire(false);
		break;
	default:
		break;
	}
}

void AWeaponDefault::ChangeDispersionByShot()
{
	CurrentDispersion = CurrentDispersion + CurrentDispersionRecoil;
}

float AWeaponDefault::GetCurrentDispersion() const
{
	float Result = CurrentDispersion;
	return Result;
}

FVector AWeaponDefault::ApplyDispersionToShoot(FVector DirectionShoot) const
{
	return FMath::VRandCone(DirectionShoot, GetCurrentDispersion() * PI / 180.0f);
}

FVector AWeaponDefault::GetFireEndLocation() const
{
	if (!ShootLocation)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWeaponDefault::GetFireEndLocation - ShootLocation is null!"));
		return FVector::ZeroVector; // Возвращаем нулевой вектор в случае ошибки
	}

	bool bShootDirection = false;
	FVector EndLocation = FVector(0.0f);
	static const float MaxFireDistance = 20000.0f;  // Замена числового выражения в вычислениях.

	FVector TmpV = (ShootLocation->GetComponentLocation() - ShootEndLocation);

	if (TmpV.Size() > SizeVectorToChangeShootDirectionLogic)
	{
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot((ShootLocation->GetComponentLocation() - ShootEndLocation).GetSafeNormal()) * -MaxFireDistance;
		if (ShowDebug)
		{
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), -(ShootLocation->GetComponentLocation() - ShootEndLocation), 
				WeaponSetting.DistanceTrace, GetCurrentDispersion() * PI / 180.0f, GetCurrentDispersion() * PI / 180.0f, 
				32, FColor::Emerald, false, 0.1f, (uint8)'\000', 1.0f);
		}
		else
		{
			EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(ShootLocation->GetForwardVector()) * MaxFireDistance;
			if (ShowDebug)
			{
				DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), ShootLocation->GetForwardVector(), WeaponSetting.DistanceTrace, GetCurrentDispersion() * PI / 180.0f,
					GetCurrentDispersion() * PI / 180.0f, 32, FColor::Emerald, false, 0.1f, (uint8)'\000', 1.0f);
			}
		}
#if WITH_EDITORONLY_DATA
		if (ShowDebug)
		{
			//direction weapon look
			DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector() * 500.0f, FColor::Cyan, false, 5.f, (uint8)'\000', 0.5f);
			//direction projectile must fly
			DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), ShootEndLocation, FColor::Red, false, 5.f, (uint8)'\000', 0.5f);
			//Direction Projectile Current fly
			DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), EndLocation, FColor::Black, false, 5.f, (uint8)'\000', 0.5f);

			//DrawDebugSphere(GetWorld(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector()*SizeVectorToChangeShootDirectionLogic, 10.f, 8, FColor::Red, false, 4.0f);
		}
#endif
		
	}
	return EndLocation;
}

int8 AWeaponDefault::GetNumberProjectileByShot() const
{
	return WeaponSetting.NumberProjectileByShot;
}

int32 AWeaponDefault::GetWeaponRound()
{
	return WeaponInfo.Round;
}

void AWeaponDefault::InitReload()
{
	WeaponReloading = true;

	ReloadTimer = WeaponSetting.ReloadTime;

	//ToDo Anim reload
	if (WeaponSetting.AnimCharReload)
	{
		//OnWeaponReloadStart.Broadcast(WeaponSetting.AnimCharReload);
		if (OnWeaponReloadStart.IsBound())
		{
			OnWeaponReloadStart.Broadcast(WeaponSetting.AnimCharReload);                    // Проверка на наличие слушателей
		}
	}
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;
	WeaponInfo.Round = WeaponSetting.MaxRound;

	OnWeaponReloadEnd.Broadcast();
}
#include "ProjectileDefault_Grenade.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"

void AProjectileDefault_Grenade::BeginPlay()
{
    Super::BeginPlay();

    // Убедимся, что коллизия реагирует правильно
    if (BulletCollisionSphere)
    {
        if (BulletCollisionSphere && IsValid(this))
        {
            BulletCollisionSphere->OnComponentHit.RemoveDynamic(this, &AProjectileDefault_Grenade::BulletCollisionSphereHit);
            BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileDefault_Grenade::BulletCollisionSphereHit);
        }
        BulletCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void AProjectileDefault_Grenade::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (TimerEnabled)
    {
        TimerExplose(DeltaTime);
    }
}

AProjectileDefault_Grenade::AProjectileDefault_Grenade()
{
    GrenadeMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("GrenadeMovement"));
    GrenadeMovement->UpdatedComponent = BulletCollisionSphere;
    GrenadeMovement->InitialSpeed = 1500.f;
    GrenadeMovement->MaxSpeed = 2000.f;
    GrenadeMovement->bRotationFollowsVelocity = true;
    GrenadeMovement->ProjectileGravityScale = 1.5f;
}

void AProjectileDefault_Grenade::TimerExplose(float DeltaTime)
{
    if (!TimerEnabled) return;

    if (TimerToExplose >= TimeToExplose)
    {
        Explose();
    }
    else
    {
        TimerToExplose += DeltaTime;

        // Альтернативный способ визуализации
        float TimeLeft = TimeToExplose - TimerToExplose;
        if (TimeLeft <= 2.0f)
        {
            // Создаем временный эффект для индикации
            if (GetWorld() && (int32)(TimeLeft * 10) % 5 == 0)
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                  ProjectileSetting.ProjectileTrailFx, // Добавьте этот эффект в настройки
                    GetActorLocation(),
                    FRotator::ZeroRotator,
                    true
                );
            }
        }
    }
}

void AProjectileDefault_Grenade::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Не вызываем родительский метод, чтобы избежать двойной обработки
    if (!bExplosed)
    {
        Explose(); // Взрыв при столкновении
    }
}

void AProjectileDefault_Grenade::ImpactProjectile()
{
    // Активируем таймер взрыва при "мягком" попадании
    if (!TimerEnabled && !bExplosed)
    {
        TimerEnabled = true;

        // Останавливаем гранату при падении на землю
        if (GrenadeMovement)
        {
            GrenadeMovement->Velocity = FVector::ZeroVector;
            GrenadeMovement->SetActive(false);
        }
    }
}

UProjectileMovementComponent* AProjectileDefault_Grenade::GetProjectileMovement() const
{
    return ProjectileMovement;
}

/*

// В конструкторе инициализируйте компонент:
AProjectileDefault_Grenade::AProjectileDefault_Grenade()
{
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = RootComponent;
    ProjectileMovement->InitialSpeed = 1500.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = true;
}

*/

void AProjectileDefault_Grenade::Explose()
{
    if (!IsValid(this)) return;
    if (bExplosed) return;
    bExplosed = true;
    TimerEnabled = false;

    // Эффекты взрыва
    if (ProjectileSetting.ExploseFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ProjectileSetting.ExploseFX,
            GetActorTransform(),
            true,
            EPSCPoolMethod::AutoRelease
        );
    }

    if (ProjectileSetting.ExploseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            ProjectileSetting.ExploseSound,
            GetActorLocation(),
            1.0f,
            FMath::RandRange(0.9f, 1.1f)
        );
    }

    // Наносим урон
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this); // Игнорируем саму гранату

    UGameplayStatics::ApplyRadialDamageWithFalloff(
        GetWorld(),
        ProjectileSetting.ExploseMaxDamage,
        ProjectileSetting.ExploseMaxDamage * 0.2f,
        GetActorLocation(),
        ProjectileSetting.ProjectileMaxRadiusDamage,
        ProjectileSetting.ProjectileMinRadiusDamage,
        ProjectileSetting.ExplodeFalloffCoef,
        nullptr,
        IgnoredActors,
        this,
        GetInstigatorController(),
        ECollisionChannel::ECC_Visibility
    );

    // Отладочная визуализация
#if ENABLE_DRAW_DEBUG
    if (ProjectileSetting.Projectile)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(),
            ProjectileSetting.ProjectileMaxRadiusDamage, 12,
            FColor::Red, false, 2.0f);
        DrawDebugSphere(GetWorld(), GetActorLocation(),
            ProjectileSetting.ProjectileMinRadiusDamage, 12,
            FColor::Orange, false, 2.0f);
    }
#endif

    // Уничтожаем с небольшой задержкой
    SetLifeSpan(0.1f);
}
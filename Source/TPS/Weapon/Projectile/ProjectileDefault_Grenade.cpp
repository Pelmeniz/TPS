#include "ProjectileDefault_Grenade.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"

void AProjectileDefault_Grenade::BeginPlay()
{
    Super::BeginPlay();

    // ��������, ��� �������� ��������� ���������
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

        // �������������� ������ ������������
        float TimeLeft = TimeToExplose - TimerToExplose;
        if (TimeLeft <= 2.0f)
        {
            // ������� ��������� ������ ��� ���������
            if (GetWorld() && (int32)(TimeLeft * 10) % 5 == 0)
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                  ProjectileSetting.ProjectileTrailFx, // �������� ���� ������ � ���������
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
    // �� �������� ������������ �����, ����� �������� ������� ���������
    if (!bExplosed)
    {
        Explose(); // ����� ��� ������������
    }
}

void AProjectileDefault_Grenade::ImpactProjectile()
{
    // ���������� ������ ������ ��� "������" ���������
    if (!TimerEnabled && !bExplosed)
    {
        TimerEnabled = true;

        // ������������� ������� ��� ������� �� �����
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

// � ������������ ��������������� ���������:
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

    // ������� ������
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

    // ������� ����
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this); // ���������� ���� �������

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

    // ���������� ������������
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

    // ���������� � ��������� ���������
    SetLifeSpan(0.1f);
}
#pragma once

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/ProjectileDefault.h"
#include "ProjectileDefault_Grenade.generated.h"

/**
 *
 */
UCLASS()
class TPS_API AProjectileDefault_Grenade : public AProjectileDefault
{
	GENERATED_BODY()


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "Grenade")
	bool bExplosed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* GrenadeMovement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;


public:

	AProjectileDefault_Grenade();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	class UProjectileMovementComponent* GetProjectileMovement() const;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void TimerExplose(float DeltaTime);

	virtual void BulletCollisionSphereHit(class UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void ImpactProjectile() override;

	void Explose();

	bool TimerEnabled = false;
	float TimerToExplose = 0.0f;
	float TimeToExplose = 5.0f;
};

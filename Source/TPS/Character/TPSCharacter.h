// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/EngineTypes.h"
#include "TPS\FuncLibrary\Type.h"
#include "Engine/World.h"
#include "TPS/Weapon/WeaponDefault.h"
#include "TPSCharacter.generated.h"


USTRUCT(BlueprintType)
struct FCharacterSpeedInfo
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class ATPSCharacter : public ACharacter
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

public:
	ATPSCharacter();


	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	

	virtual void SetupPlayerInputComponent(class UInputComponent* NewInputComponent) override;


	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
	//Cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);


	//Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraDistance; // Расстояние от камеры до персонажа

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinCameraDistance; // Минимальное расстояние камеры

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxCameraDistance; // Ммаксимальное расстояние камеры

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraSlideSpeed; // Минимальное расстояние камеры


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovementState MovementState = EMovementState::Run_State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FCharacterSpeed MovementSpeedInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bSprintRunEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bWalkEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bAimEnabled = false;
	
	//Weapon
	AWeaponDefault* CurrentWaepon = nullptr;

	//For Demo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	TSubclassOf<AWeaponDefault> InitWeaponClass = nullptr;
	
	UDecalComponent* CurrentCursor = nullptr;

	// Inputs
	UFUNCTION()
	void InputAxisX(float Value);
	UFUNCTION()
	void InputAxisY(float Value);
	
	UFUNCTION()
	void InputsAttackPressed();
	UFUNCTION()
	void InputsAttackReleased();
	

	float AxisX = 0.0f;
	float AxisY = 0.0f;

	// Tick Function
	UFUNCTION()
	void MovementTick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void CharacterUpdate();
	UFUNCTION(BlueprintCallable)
	void ChangeMovementeState(EMovementState NewMovementState);
	
	
	// Зум колесика мыши. 
	UFUNCTION(BlueprintCallable)
	void CameraSlideTick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void InputMouseWheel(float Value);
	
	UFUNCTION(BlueprintCallable)
	bool IsForwardMove();
	
	UFUNCTION(BlueprintCallable)
	AWeaponDefault* GetCurrentWeapon();
	void InitWeapon();
	

	UFUNCTION(BlueprintCallable)
	UDecalComponent* GetCursorToWorld();
};


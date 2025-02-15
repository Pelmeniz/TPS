// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPS\FuncLibrary\Type.h"
#include "TPSCharacter.generated.h"



UCLASS(Blueprintable)
class ATPSCharacter : public ACharacter
{
	GENERATED_BODY()

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

	UFUNCTION()
	void InputAxisX(float Value);
	UFUNCTION()
	void InputAxisY(float Value);

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
};


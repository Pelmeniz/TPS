#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TPS/FuncLibrary/Type.h"
#include "Engine/DataTable.h"
#include "TPS/Weapon/WeaponDefault.h"
#include "TPSGameInstance.generated.h"

UCLASS()
class TPS_API UTPSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	
	//table
	UPROPERTY()
	UDataTable* WeaponDataTable = nullptr;
	UFUNCTION(BlueprintCallable)
	bool GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo);
};
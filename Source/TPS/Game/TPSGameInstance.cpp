#include "TPS/Game/TPSGameInstance.h"

bool UTPSGameInstance::GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo)
{
	bool bIsFind = false;
	FWeaponInfo* WeaponInfoRow;

	if (WeaponDataTable)
	{
		WeaponInfoRow = WeaponDataTable->FindRow<FWeaponInfo>(NameWeapon, "", false);
		if (WeaponInfoRow)
		{
			bIsFind = true;
			OutInfo = *WeaponInfoRow;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTPSGameInstance::GetWeaponInfoByName - WeaponTable -NULL"));
	}
	return bIsFind;
}

bool UTPSGameInstance::GetDropItemInfoByName(FName NameItem, FDropItem& OutInfo)
{
	bool bIsFind = false;
	if (DropItemInfoTable)
	{
		FDropItem* DropItemInfoRow;
		TArray<FName>RowNames = DropItemInfoTable->GetRowNames();

		int8 i = 0;
		while (i < RowNames.Num() && !bIsFind)
		{
			DropItemInfoRow = DropItemInfoTable->FindRow<FDropItem>(RowNames[i], "");
			if (DropItemInfoRow->WeaponInfo.NameItem == NameItem)
			{
				OutInfo = (*DropItemInfoRow);
				bIsFind = true;
			}
			i++;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTPSGameInstance::GetDropItemInfoByName - DropItemInfoTable -NULL"));
	}
	return bIsFind;
}

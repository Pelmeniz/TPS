// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TPSInventoryComponent.h"
#include "Game/TPSGameInstance.h"
#pragma optimize ("", off)

// Sets default values for this component's properties
UTPSInventoryComponent::UTPSInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTPSInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	// Find init WeaponSlots and first init Weapon
	for (int8 i = 0; i < WeaponSlots.Num(); i++)
	{
		UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());
		if (MyGI)
		{
			if (!WeaponSlots[i].NameItem.IsNone())
			{
				FWeaponInfo Info;
				if (MyGI->GetWeaponInfoByName(WeaponSlots[i].NameItem, Info))
					WeaponSlots[i].AdditionalInfo.Round = Info.MaxRound;
				else
				{
					//WeaponSlots.RemoveAt(i);
					//i--;
				}
			}
		}
	}
	MaxSlotsWeapon = WeaponSlots.Num();

	if (WeaponSlots.IsValidIndex(0))
	{
		if (WeaponSlots[0].NameItem.IsNone())
			OnSwitchWeapon.Broadcast(WeaponSlots[0].NameItem, WeaponSlots[0].AdditionalInfo, 0);
	}
	
}


// Called every frame
void UTPSInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UTPSInventoryComponent::SwitchWeaponToIndex(int32 ChangeToIndex, int32 OldIndex, FAdditionalWeaponInfo OldInfo, bool bIsForward)
{
	bool bIsSuccess = false;
	int8 CorrectIndex = ChangeToIndex;
	if (ChangeToIndex > WeaponSlots.Num() - 1)
		CorrectIndex = 0;
	else
		if (ChangeToIndex < 0)
			CorrectIndex = WeaponSlots.Num() - 1;

	FName NewIdWeapon;
	FAdditionalWeaponInfo NewAdditionalInfo;
	int32 NewCurrentIndex = 0;

	if (WeaponSlots.IsValidIndex(CorrectIndex))
	{
		if (!WeaponSlots[CorrectIndex].NameItem.IsNone())
		{
			if (WeaponSlots[CorrectIndex].AdditionalInfo.Round > 0)
			{
				//good weapon have ammo start change
				bIsSuccess = true;
			}
			else
			{
				UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());
				if (MyGI)
				{
					//check ammoSlots for this weapon
					FWeaponInfo MyInfo;
					MyGI->GetWeaponInfoByName(WeaponSlots[CorrectIndex].NameItem, MyInfo);

					bool bIsFind = false;
					int8 j = 0;
					while (j < AmmoSlots.Num() && !bIsFind)
					{
						if (AmmoSlots[j].WeaponType == MyInfo.WeaponType && AmmoSlots[j].Cout > 0)
						{
							//good weapon have ammo start change
							bIsSuccess = true;
							bIsFind = true;
						}
						j++;
					}
				}
			}
			if (bIsSuccess)
			{
				NewCurrentIndex = CorrectIndex;
				NewIdWeapon = WeaponSlots[CorrectIndex].NameItem;
				NewAdditionalInfo = WeaponSlots[CorrectIndex].AdditionalInfo;
			}
		}
	}

	if (!bIsSuccess)
	{
		if (bIsForward)
		{
			int8 Iteration = 0;
			int8 Seconditeration = 0;
			while (Iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				Iteration++;
				int8 TmpIndex = ChangeToIndex + Iteration;
				if (WeaponSlots.IsValidIndex(TmpIndex))
				{
					if (!WeaponSlots[TmpIndex].NameItem.IsNone())
					{
						if (WeaponSlots[TmpIndex].AdditionalInfo.Round > 0)
						{
							//WeaponGood
							bIsSuccess = true;
							NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
							NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
							NewCurrentIndex = TmpIndex;
						}
						else
						{
							FWeaponInfo MyInfo;
							UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());

							MyGI->GetWeaponInfoByName(WeaponSlots[TmpIndex].NameItem, MyInfo);

							bool bIsFind = false;
							int8 j = 0;
							while (j < AmmoSlots.Num() && !bIsFind)
							{
								if (AmmoSlots[j].WeaponType == MyInfo.WeaponType && AmmoSlots[j].Cout > 0)
								{
									//WeaponGood
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
									NewCurrentIndex = TmpIndex;
									bIsFind = true;
								}
								j++;
							}
						}
					}
				}
				else
				{
					//go to end of right of array weapon slots
					if (OldIndex != Seconditeration)
					{
						if (WeaponSlots.IsValidIndex(Seconditeration))
						{
							if (!WeaponSlots[Seconditeration].NameItem.IsNone())
							{
								if (WeaponSlots[Seconditeration].AdditionalInfo.Round > 0)
								{
									//WeaponGood
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[Seconditeration].NameItem;
									NewAdditionalInfo = WeaponSlots[Seconditeration].AdditionalInfo;
									NewCurrentIndex = Seconditeration;
								}
								else
								{
									FWeaponInfo MyInfo;
									UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());

									MyGI->GetWeaponInfoByName(WeaponSlots[Seconditeration].NameItem, MyInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == MyInfo.WeaponType && AmmoSlots[j].Cout > 0)
										{
											//WeaponGood
											bIsSuccess = true;
											NewIdWeapon = WeaponSlots[Seconditeration].NameItem;
											NewAdditionalInfo = WeaponSlots[Seconditeration].AdditionalInfo;
											NewCurrentIndex = Seconditeration;
											bIsFind = true;
										}
										j++;
									}
								}
							}
						}
					}
					else
					{
						//go to same weapon when start
						if (WeaponSlots.IsValidIndex(Seconditeration))
						{
							if (!WeaponSlots[Seconditeration].NameItem.IsNone())
							{
								if (WeaponSlots[Seconditeration].AdditionalInfo.Round > 0)
								{
									//WeaponGood, it same weapon do nothing
								}
								else
								{
									FWeaponInfo MyInfo;
									UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());

									MyGI->GetWeaponInfoByName(WeaponSlots[Seconditeration].NameItem, MyInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == MyInfo.WeaponType)
										{
											if (AmmoSlots[j].Cout > 0)
											{
												//WeaponGood, it same weapon do nothing
											}
											else
											{
												//Not find weapon with amm need init Pistol with infinity ammo
												UE_LOG(LogTemp, Error, TEXT("UTPSInventoryComponent::SwitchWeaponToIndex - Init PISTOL - NEED"));
											}
										}
										j++;
									}
								}
							}
						}
					}
					Seconditeration++;
				}
			}
		}
		else
		{
			int8 Iteration = 0;
			int8 Seconditeration = WeaponSlots.Num() - 1;
			while (Iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				Iteration++;
				int8 TmpIndex = ChangeToIndex - Iteration;
				if (WeaponSlots.IsValidIndex(TmpIndex))
				{
					if (!WeaponSlots[TmpIndex].NameItem.IsNone())
					{
						if (WeaponSlots[TmpIndex].AdditionalInfo.Round > 0)
						{
							//WeaponGood
							bIsSuccess = true;
							NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
							NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
							NewCurrentIndex = TmpIndex;
						}
						else
						{
							FWeaponInfo MyInfo;
							UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());

							MyGI->GetWeaponInfoByName(WeaponSlots[TmpIndex].NameItem, MyInfo);

							bool bIsFind = false;
							int8 j = 0;
							while (j < AmmoSlots.Num() && !bIsFind)
							{
								if (AmmoSlots[j].WeaponType == MyInfo.WeaponType && AmmoSlots[j].Cout > 0)
								{
									//WeaponGood
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[TmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[TmpIndex].AdditionalInfo;
									NewCurrentIndex = TmpIndex;
									bIsFind = true;
								}
								j++;
							}
						}
					}
				}
				else
				{
					//go to end of LEFT of array weapon slots
					if (OldIndex != Seconditeration)
					{
						if (WeaponSlots.IsValidIndex(Seconditeration))
						{
							if (!WeaponSlots[Seconditeration].NameItem.IsNone())
							{
								if (WeaponSlots[Seconditeration].AdditionalInfo.Round > 0)
								{
									//WeaponGood
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[Seconditeration].NameItem;
									NewAdditionalInfo = WeaponSlots[Seconditeration].AdditionalInfo;
									NewCurrentIndex = Seconditeration;
								}
								else
								{
									FWeaponInfo MyInfo;
									UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());

									MyGI->GetWeaponInfoByName(WeaponSlots[Seconditeration].NameItem, MyInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == MyInfo.WeaponType && AmmoSlots[j].Cout > 0)
										{
											//WeaponGood
											bIsSuccess = true;
											NewIdWeapon = WeaponSlots[Seconditeration].NameItem;
											NewAdditionalInfo = WeaponSlots[Seconditeration].AdditionalInfo;
											NewCurrentIndex = Seconditeration;
											bIsFind = true;
										}
										j++;
									}
								}
							}
						}
					}
					else
					{
						//go to same weapon when start
						if (WeaponSlots.IsValidIndex(Seconditeration))
						{
							if (!WeaponSlots[Seconditeration].NameItem.IsNone())
							{
								if (WeaponSlots[Seconditeration].AdditionalInfo.Round > 0)
								{
									//WeaponGood, it same weapon do nothing
								}
								else
								{
									FWeaponInfo MyInfo;
									UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());

									MyGI->GetWeaponInfoByName(WeaponSlots[Seconditeration].NameItem, MyInfo);

									bool bIsFind = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFind)
									{
										if (AmmoSlots[j].WeaponType == MyInfo.WeaponType)
										{
											if (AmmoSlots[j].Cout > 0)
											{
												//WeaponGood, it same weapon do nothing
											}
											else
											{
												//Not find weapon with amm need init Pistol with infinity ammo
												UE_LOG(LogTemp, Error, TEXT("UTPSInventoryComponent::SwitchWeaponToIndex - Init PISTOL - NEED"));
											}
										}
										j++;
									}
								}
							}
						}
					}
					Seconditeration--;
				}
			}
		}
	}

	if (bIsSuccess)
	{
		SetAdditionalInfoWeapon(OldIndex, OldInfo);
		OnSwitchWeapon.Broadcast(NewIdWeapon, NewAdditionalInfo, NewCurrentIndex);
		//OnWeaponAmmoAviable.Broadcast()
	}


	return bIsSuccess;
}

FAdditionalWeaponInfo UTPSInventoryComponent::GetAdditionalInfoWeapon(int32 IndexWeapon)
{
	FAdditionalWeaponInfo Result;
	if (WeaponSlots.IsValidIndex(IndexWeapon))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (/*WeaponSlots[i].IndexSlot */i == IndexWeapon)
			{
				Result = WeaponSlots[i].AdditionalInfo;
				bIsFind = true;
			}
			i++;
		}
		if (!bIsFind)
			UE_LOG(LogTemp, Warning, TEXT("UTPSInventoryComponent::SetAdditionalInfoWeapon - No Found Weapon with index - %d"), IndexWeapon);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UTPSInventoryComponent::SetAdditionalInfoWeapon - Not Correct index Weapon - %d"), IndexWeapon);

	return Result;
}

int32 UTPSInventoryComponent::GetWeaponIndexSlotByName(FName IdWeaponName)
{
	int32 result = -1;
	int8 i = 0;
	bool bIsFind = false;
	while (i < WeaponSlots.Num() && !bIsFind)
	{
		if (WeaponSlots[i].NameItem == IdWeaponName)
		{
			bIsFind = true;
			result = i/*WeaponSlots[i].IndexSlot*/;
		}
		i++;
	}
	return result;
}

FName UTPSInventoryComponent::GetWeaponNameBySlotIndex(int32 IndexSlot)
{
	FName Result;

	if (WeaponSlots.IsValidIndex(IndexSlot))
	{
		Result = WeaponSlots[IndexSlot].NameItem;
	}
	return Result;
}

void UTPSInventoryComponent::SetAdditionalInfoWeapon(int32 IndexWeapon, FAdditionalWeaponInfo NewInfo)
{
	if (WeaponSlots.IsValidIndex(IndexWeapon))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (/*WeaponSlots[i].IndexSlot*/i == IndexWeapon)
			{
				WeaponSlots[i].AdditionalInfo = NewInfo;
				bIsFind = true;

				OnWeaponAdditionalInfoChange.Broadcast(IndexWeapon, NewInfo);
			}
			i++;
		}
		if (!bIsFind)
			UE_LOG(LogTemp, Warning, TEXT("UTPSInventoryComponent::SetAdditionalInfoWeapon - No Found Weapon with index - %d"), IndexWeapon);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UTPSInventoryComponent::SetAdditionalInfoWeapon - Not Correct index Weapon - %d"), IndexWeapon);
}

void UTPSInventoryComponent::AmmoSlotChangeValue(EWeaponType TypeWeapon, int32 CoutChangeAmmo)
{
	bool bIsFind = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bIsFind)
	{
		if (AmmoSlots[i].WeaponType == TypeWeapon)
		{
			AmmoSlots[i].Cout += CoutChangeAmmo;
			if (AmmoSlots[i].Cout > AmmoSlots[i].MaxCout)
				AmmoSlots[i].Cout = AmmoSlots[i].MaxCout;

			OnAmmoChange.Broadcast(AmmoSlots[i].WeaponType, AmmoSlots[i].Cout);

			bIsFind = true;
		}
		i++;
	}
}

bool UTPSInventoryComponent::CheckAmmoForWeapon(EWeaponType TypeWeapon, int8& AviableAmmoForWeapon)
{
	AviableAmmoForWeapon = 0;
	bool bIsFind = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bIsFind)
	{
		if (AmmoSlots[i].WeaponType == TypeWeapon)
		{
			bIsFind = true;
			AviableAmmoForWeapon = AmmoSlots[i].Cout;
			if (AmmoSlots[i].Cout > 0)
			{
				//OnWeaponAmmoAviable.Broadcast(TypeWeapon);//remove not here, only when pickUp ammo this type, or swithc weapon
				return true;
			}

		}

		i++;
	}

	OnWeaponAmmoEmpty.Broadcast(TypeWeapon);//visual empty ammo slot

	return false;
}

bool UTPSInventoryComponent::CheckCanTakeAmmo(EWeaponType AmmoType)
{
	bool bResult = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bResult)
	{
		if (AmmoSlots[i].WeaponType == AmmoType && AmmoSlots[i].Cout < AmmoSlots[i].MaxCout)
			bResult = true;
		i++;
	}
	return bResult;
}

bool UTPSInventoryComponent::CheckCanTakeWeapon(int32& FreeSlot)
{
	bool bIsFreeSlot = false;
	int8 i = 0;
	while (i < WeaponSlots.Num() && !bIsFreeSlot)
	{
		if (WeaponSlots[i].NameItem.IsNone())
		{
			bIsFreeSlot = true;
			FreeSlot = i;
		}
		i++;
	}
	return bIsFreeSlot;
}

bool UTPSInventoryComponent::SwitchWeaponToInventory(FWeaponSlot NewWeapon, int32 IndexSlot, int32 CurrentIndexWeaponChar, FDropItem& DropItemInfo)
{
	bool bResult = false;

	if (WeaponSlots.IsValidIndex(IndexSlot) && GetDropItemInfoFromInventory(IndexSlot, DropItemInfo))
	{
		WeaponSlots[IndexSlot] = NewWeapon;

		SwitchWeaponToIndex(CurrentIndexWeaponChar, -1, NewWeapon.AdditionalInfo, true);

		OnUpdateWeaponSlots.Broadcast(IndexSlot, NewWeapon);
		bResult = true;
	}
	return bResult;
}

bool UTPSInventoryComponent::TryGetWeaponToInventory(FWeaponSlot NewWeapon)
{
	int32 IndexSlot = -1;
	if (CheckCanTakeWeapon(IndexSlot))
	{
		if (WeaponSlots.IsValidIndex(IndexSlot))
		{
			WeaponSlots[IndexSlot] = NewWeapon;

			OnUpdateWeaponSlots.Broadcast(IndexSlot, NewWeapon);
			return true;
		}
	}
	return false;
}

bool UTPSInventoryComponent::GetDropItemInfoFromInventory(int32 IndexSlot, FDropItem& DropItemInfo)
{
	bool bResult = false;

	FName DropItemName = GetWeaponNameBySlotIndex(IndexSlot);

	UTPSGameInstance* MyGI = Cast<UTPSGameInstance>(GetWorld()->GetGameInstance());
	if (MyGI)
	{
		bResult = MyGI->GetDropItemInfoByName(DropItemName, DropItemInfo);
		if (WeaponSlots.IsValidIndex(IndexSlot))
		{
			DropItemInfo.WeaponInfo.AdditionalInfo = WeaponSlots[IndexSlot].AdditionalInfo;
		}
	}

	return bResult;
}

#pragma optimize ("", on)














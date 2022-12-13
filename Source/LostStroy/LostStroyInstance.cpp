// Fill out your copyright notice in the Description page of Project Settings.


#include "LostStroyInstance.h"

FST_Weapon* ULostStroyInstance::GetWeaponRowData(FName name)
{
	return WeaponTable->FindRow<FST_Weapon>(name, TEXT(""));
}

FName ULostStroyInstance::GetWeaponRandomRowName()
{
	TArray<FName> names = WeaponTable->GetRowNames();
	FName WeaponName = names[FMath::RandRange(0, names.Num() - 1)];

	return WeaponName;
}
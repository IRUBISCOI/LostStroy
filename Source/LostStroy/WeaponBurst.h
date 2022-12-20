// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "WeaponBurst.generated.h"

/**
 * 
 */
UCLASS()
class LOSTSTROY_API AWeaponBurst : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void PressTrigger_Implementation(bool isPressed) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BurstShooting();

	FTimerHandle th_BurstShooting;	
};

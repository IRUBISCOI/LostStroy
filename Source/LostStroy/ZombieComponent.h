// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZombieComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_TwoParams(FDele_Multi_UpdateHp_TwoParams, float, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FDele_Multi_UpdateMag_OneParam, int);

UCLASS( ClassGroup=(Custom), Blueprintable , meta=(BlueprintSpawnableComponent) )
class LOSTSTROY_API UZombieComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZombieComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurHp, VisibleAnywhere)
	float CurHp;

	UPROPERTY(ReplicatedUsing = OnRep_MaxHp, VisibleAnywhere)
	float MaxHp;
		
public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurHp() const { return CurHp; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHp() const { return MaxHp; }

public:
	UFUNCTION()
	void OnRep_CurHp();

	UFUNCTION()
	void OnRep_MaxHp();

	UFUNCTION(BlueprintCallable)
	void AddDamage(float Damage);

	FDele_Multi_UpdateHp_TwoParams Fuc_Dele_UpdateHp_TwoParams;
	FDele_Multi_UpdateMag_OneParam Fuc_Dele_UpdateMag_OneParam;
};

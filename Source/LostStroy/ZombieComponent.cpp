// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UZombieComponent::UZombieComponent()
{
	CurHp = 100.0f;
	MaxHp = 100.0f;
}


// Called when the game starts
void UZombieComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UZombieComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UZombieComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UZombieComponent, CurHp);
	DOREPLIFETIME(UZombieComponent, MaxHp);
}

void UZombieComponent::OnRep_CurHp()
{
	if (Fuc_Dele_UpdateHp_TwoParams.IsBound())
		Fuc_Dele_UpdateHp_TwoParams.Broadcast(CurHp, MaxHp);
}

void UZombieComponent::OnRep_MaxHp()
{
}

void UZombieComponent::AddDamage(float Damage)
{
	CurHp = CurHp - Damage;
	CurHp = FMath::Clamp(CurHp, 0.0f, MaxHp);

	OnRep_CurHp();
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "LostChildState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameHUD.h"

void ALostChildState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALostChildState, CurHp);
	DOREPLIFETIME(ALostChildState, MaxHp);
	DOREPLIFETIME(ALostChildState, Mag);
}

ALostChildState::ALostChildState()
{
	CurHp = 100.0f;
	MaxHp = 100.0f;
}

void ALostChildState::OnRep_CurHp()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("OnRep_CurHp = %f"), CurHp));

	if (Fuc_Dele_UpdateHp_TwoParams.IsBound())
		Fuc_Dele_UpdateHp_TwoParams.Broadcast(CurHp, MaxHp);
}

void ALostChildState::OnRep_MaxHp()
{
}

void ALostChildState::OnRep_Mag()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("OnRep_Mag = %f"), Mag));

	if (Fuc_Dele_UpdateMag_OneParam.IsBound())
		Fuc_Dele_UpdateMag_OneParam.Broadcast(Mag);
}

void ALostChildState::AddDamage(float Damage)
{
	CurHp = CurHp - Damage;
	CurHp = FMath::Clamp(CurHp, 0.0f, MaxHp);

	OnRep_CurHp();
}

void ALostChildState::AddHeal(float Heal)
{
	CurHp = CurHp + Heal;
	CurHp = FMath::Clamp(CurHp, 0.0f, MaxHp);

	OnRep_CurHp();
}

void ALostChildState::AddMag()
{
	Mag = Mag + 1;

	OnRep_Mag();
}

bool ALostChildState::UseMag()
{
	bool result = IsCanUseMag();

	if (result)
	{
		Mag = Mag - 1;

		OnRep_Mag();
	}

	return result;
}

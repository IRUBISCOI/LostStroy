// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUD.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "LostChildState.h"

void AGameHUD::OnUpdateMyHp_Implementation(float CurrentHp, float MaxHp)
{
}

void AGameHUD::OnUpdateMyAmmo_Implementation(int Ammo)
{
}

void AGameHUD::OnUpdateMyMag_Implementation(int Mag)
{
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	check(HudWidgetClass);

	HudWidget = CreateWidget<UUserWidget>(GetWorld(), HudWidgetClass);
	HudWidget->AddToViewport();

	BindPlayerState();
}

void AGameHUD::BindPlayerState()
{
	//APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APlayerController* pc = GetWorld()->GetFirstPlayerController();

	if (IsValid(pc) && pc->PlayerState != nullptr)
	{
		ALostChildState* ps = Cast<ALostChildState>(pc->PlayerState);
		if (IsValid(ps))
		{
			ps->Fuc_Dele_UpdateHp_TwoParams.AddUFunction(this, FName("OnUpdateMyHp"));
			OnUpdateMyHp(ps->GetCurHp(), ps->GetMaxHp());

			ps->Fuc_Dele_UpdateMag_OneParam.AddUFunction(this, FName("OnUpdateMyMag"));
			OnUpdateMyMag(ps->GetMag());
		}

		return;
	}

	FTimerManager& timerManager = GetWorld()->GetTimerManager();
	timerManager.SetTimer(th_BindPlayerState, this, &AGameHUD::BindPlayerState, 0.1f, false);
}



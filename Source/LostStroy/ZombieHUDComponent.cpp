// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ZombieComponent.h"
#include "GameFramework/Actor.h"

void UZombieHUDComponent::OnUpdateMyHp_Implementation(float CurrentHp, float MaxHp)
{

}

// Called when the game starts
void UZombieHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	check(HudWidgetClass);

	HudWidget = CreateWidget<UUserWidget>(GetWorld(), HudWidgetClass);
	HudWidget->AddToViewport();

	BindPlayerState();	
}

void UZombieHUDComponent::BindPlayerState()
{
	
}


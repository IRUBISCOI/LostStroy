// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZombieHUDComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LOSTSTROY_API UZombieHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> HudWidgetClass;

	UPROPERTY(BlueprintReadWrite)
	UUserWidget* HudWidget;

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnUpdateMyHp(float CurrentHp, float MaxHp);

	void OnUpdateMyHp_Implementation(float CurrentHp, float MaxHp);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void BindPlayerState();

	FTimerHandle th_BindPlayerState;		
};

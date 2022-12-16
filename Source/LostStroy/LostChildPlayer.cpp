// Fill out your copyright notice in the Description page of Project Settings.


#include "LostChildPlayer.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "WeaponInterface.h"
#include "NameTagInterface.h"
#include "Weapon.h"
#include "LostChildState.h"
#include "ZombieComponent.h"

// Sets default values
ALostChildPlayer::ALostChildPlayer()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	ConstructorHelpers::FObjectFinder<UAnimMontage> montage(TEXT("AnimMontage'/Game/RifleAnimsetPro/Animations/InPlace/Rifle_ShootOnce_Montage.Rifle_ShootOnce_Montage'"));

	AnimMontage = montage.Object;

	IsRagdoll = false;
}

// Called when the game starts or when spawned
void ALostChildPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	BindPlayerState();
}

// Called every frame
void ALostChildPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() == true)
	{
		ControlPitch = GetControlRotation().Pitch;
	}

	if (IsRagdoll)
	{
		SetActorLocation(GetMesh()->GetSocketLocation("spine_01") + FVector(0.0f, 0.0f, 60.0f));
	}
}

float ALostChildPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("TakeDamage Damage=%f EventInstigator=%s"), DamageAmount, *EventInstigator->GetName()));

	ALostChildState* ps = Cast<ALostChildState>(GetPlayerState());
	if (ps)
	{
		ps->AddDamage(DamageAmount);
	}

	UZombieComponent* ac = Cast<UZombieComponent>(GetComponentByClass(UZombieComponent::StaticClass()));
	if (ac)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Damage Complete")));
		ac->AddDamage(DamageAmount);

	}

	return 0.0f;
}

void ALostChildPlayer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALostChildPlayer, ControlPitch);
}

// Called to bind functionality to input
void ALostChildPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ALostChildPlayer::Dash);

	PlayerInputComponent->BindAxis("Stage1_MoveForward", this, &ALostChildPlayer::MoveForward);
	PlayerInputComponent->BindAxis("Stage1_MoveRight", this, &ALostChildPlayer::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ALostChildPlayer::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ALostChildPlayer::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ALostChildPlayer::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ALostChildPlayer::LookUpAtRate);

	// Shoot
	PlayerInputComponent->BindAction("Stage1_Trigger", IE_Pressed, this, &ALostChildPlayer::PressTrigger);
	PlayerInputComponent->BindAction("Stage1_Trigger", IE_Released, this, &ALostChildPlayer::ReleaseTrigger);

	// Reload
	PlayerInputComponent->BindAction("Stage1_Reload", IE_Pressed, this, &ALostChildPlayer::PressReload);

	// PickUp
	PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &ALostChildPlayer::PressPickUp);

	// DropWeapon
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &ALostChildPlayer::PressDropWeapon);

	// MagTest
	PlayerInputComponent->BindAction("MagTest", IE_Pressed, this, &ALostChildPlayer::PressMagTest);
}

AActor* ALostChildPlayer::SetEquipWeapon(AActor* Weapon)
{
	if (IsValid(EquipWeapon))
	{
		DisableOwnerWeapon();
	}

	ResPickUp(Weapon);

	EnableOwnerWeapon(Weapon);

	return EquipWeapon;
}

void ALostChildPlayer::OnNotifyShoot()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_NotifyShoot(EquipWeapon);
	}
}

void ALostChildPlayer::OnNotifyReload()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_NotifyReload(EquipWeapon);
	}
}

void ALostChildPlayer::OnUpdateHp_Implementation(float CurrentHp, float MaxHp)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,	FString::Printf(TEXT("OnUpdateHp CurrentHp : %f"), CurrentHp));

	if (CurrentHp <= 0)
	{
		DoRagdoll();
	}
}

void ALostChildPlayer::DoRagdoll_Implementation()
{
	IsRagdoll = true;

	GetMesh()->SetSimulatePhysics(true);
}

void ALostChildPlayer::DoGetup()
{
	IsRagdoll = false;

	GetMesh()->SetSimulatePhysics(false);

	GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	FVector loc = { 0.0f, 0.0f, -97.0f };
	FRotator Rot = { 0.0f, 270.0f, 0.0f };
	GetMesh()->SetRelativeLocationAndRotation(loc, Rot);
}

void ALostChildPlayer::AddHeal_Implementation(float Heal)
{
	ALostChildState* ps = Cast<ALostChildState>(GetPlayerState());
	if (ps)
	{
		ps->AddHeal(Heal);
	}
}

void ALostChildPlayer::AddMag_Implementation()
{
	ALostChildState* ps = Cast<ALostChildState>(GetPlayerState());
	if (ps)
	{
		ps->AddMag();
	}
}

void ALostChildPlayer::ReqPressTrigger_Implementation(bool isPressed)
{
	ResPressTrigger(isPressed);
}

void ALostChildPlayer::ResPressTrigger_Implementation(bool isPressed)
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_PressTrigger(EquipWeapon, isPressed);
	}
}

void ALostChildPlayer::ReqPressReload_Implementation()
{
	ResPressReload();
}

void ALostChildPlayer::ResPressReload_Implementation()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_PressReload(EquipWeapon);
	}
}

void ALostChildPlayer::ReqPickUp_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,	FString::Printf(TEXT("PressPickUp")));

	AActor* nearestActor = GetNearestWeapon();
	if (nearestActor)
	{
		SetEquipWeapon(nearestActor);
	}
}

void ALostChildPlayer::ResPickUp_Implementation(AActor* weapon)
{
	if (IsValid(EquipWeapon))
	{
		IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);
		if (InterfaceObj)
		{
			InterfaceObj->Execute_DetachWeapon(EquipWeapon, this);
		}
	}

	AttachWeapon(weapon);
}

void ALostChildPlayer::ReqDropWeapon_Implementation()
{
	DisableOwnerWeapon();
	ResDropWeapon();
}

void ALostChildPlayer::ResDropWeapon_Implementation()
{
	DetachWeapon(EquipWeapon);
}

void ALostChildPlayer::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ALostChildPlayer::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ALostChildPlayer::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ALostChildPlayer::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ALostChildPlayer::Dash()
{
}

void ALostChildPlayer::PressTrigger()
{
	ReqPressTrigger(true);
}

void ALostChildPlayer::ReleaseTrigger()
{
	ReqPressTrigger(false);
}

void ALostChildPlayer::BindPlayerState()
{
	ALostChildState* ps = Cast<ALostChildState>(GetPlayerState());
	UZombieComponent* ac = Cast<UZombieComponent>(GetComponentByClass(UZombieComponent::StaticClass()));

	if (IsValid(ps) | IsValid(ac))
	{
		if(IsValid(ps))
		{
			ps->Fuc_Dele_UpdateHp_TwoParams.AddUFunction(this, FName("OnUpdateHp"));
			OnUpdateHp(ps->GetCurHp(), ps->GetMaxHp());
		}

		if(IsValid(ac))
		{
			ac->Fuc_Dele_UpdateHp_TwoParams.AddUFunction(this, FName("OnUpdateHp"));
			OnUpdateHp(ac->GetCurHp(), ac->GetMaxHp());
		}
		return;
	}

	FTimerManager& timerManager = GetWorld()->GetTimerManager();
	timerManager.SetTimer(th_BindPlayerState, this, &ALostChildPlayer::BindPlayerState, 0.1f, false);
}

void ALostChildPlayer::PressReload()
{
	ReqPressReload();
}

void ALostChildPlayer::PressPickUp()
{
	ReqPickUp();
}

void ALostChildPlayer::PressDropWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,	FString::Printf(TEXT("PressDropWeapon")));

	ReqDropWeapon();
}

void ALostChildPlayer::PressMagTest()
{
	ALostChildState* ps = Cast<ALostChildState>(GetPlayerState());
	if (ps)
	{
		ps->AddMag();
	}
}

void ALostChildPlayer::OnRep_EquipWeapon()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_AttachWeapon(EquipWeapon, this);
	}
}

AActor* ALostChildPlayer::GetNearestWeapon()
{
	float nearestDist = 9999999.0f;
	AActor* nearestActor = nullptr;
	TArray<AActor*> actors;
	GetCapsuleComponent()->GetOverlappingActors(actors, AWeapon::StaticClass());

	for (AActor* weapon : actors)
	{
		IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(weapon);

		if (InterfaceObj == nullptr)
			continue;

		float dist = FVector::Distance(GetActorLocation(), weapon->GetActorLocation());

		if (dist > nearestDist)
			continue;

		nearestDist = dist;
		nearestActor = weapon;
	}

	return nearestActor;
}

void ALostChildPlayer::EnableOwnerWeapon(AActor* actor)
{
	actor->SetOwner(GetController());
}

void ALostChildPlayer::DisableOwnerWeapon()
{
	if (IsValid(EquipWeapon))
		EquipWeapon->SetOwner(nullptr);
}

void ALostChildPlayer::AttachWeapon(AActor* weapon)
{
	EquipWeapon = weapon;

	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(weapon);
	if (InterfaceObj)
	{
		InterfaceObj->Execute_AttachWeapon(weapon, this);
	}

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ALostChildPlayer::DetachWeapon(AActor* weapon)
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_DetachWeapon(EquipWeapon, this);
	}

	EquipWeapon = nullptr;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

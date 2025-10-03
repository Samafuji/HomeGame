// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/Components/CharacterStatusComponent.h"
#include "Home/DailyCycleSubsystem.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UCharacterStatusComponent::UCharacterStatusComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCharacterStatusComponent::BeginPlay()
{
	Super::BeginPlay();

    RecomputeHeart();

    if (UWorld* W = GetWorld())
    {
        if (auto* Cycle = W->GetSubsystem<UDailyCycleSubsystem>())
        {
            Cycle->OnRecoveryTick.AddDynamic(this, &UCharacterStatusComponent::HandleRecoveryTick);
            Cycle->OnDailyReset.AddDynamic(this, &UCharacterStatusComponent::HandleDailyReset);
        }
    }
}

void UCharacterStatusComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* W = GetWorld())
    {
        if (auto* Cycle = W->GetSubsystem<UDailyCycleSubsystem>())
        {
            Cycle->OnRecoveryTick.RemoveDynamic(this, &UCharacterStatusComponent::HandleRecoveryTick);
            Cycle->OnDailyReset.RemoveDynamic(this, &UCharacterStatusComponent::HandleDailyReset);
        }
    }
    Super::EndPlay(EndPlayReason);
}

// íºê⁄ëÄçÏ
void UCharacterStatusComponent::AddAffection(int32 Delta) { Affection = Clamp0100(Affection + Delta); RecomputeHeart(); Broadcast(); }
void UCharacterStatusComponent::AddMood(int32 Delta) { Mood = Clamp0100(Mood + Delta);      RecomputeHeart(); Broadcast(); }
void UCharacterStatusComponent::AddEnergy(int32 Delta) { Energy = Clamp0100(Energy + Delta);    RecomputeHeart(); Broadcast(); }
void UCharacterStatusComponent::AddFocus(int32 Delta) { Focus = Clamp0100(Focus + Delta);     RecomputeHeart(); Broadcast(); }
void UCharacterStatusComponent::AddCoin(int32 Delta) { Coin = FMath::Max(0, Coin + Delta);  Broadcast(); }


void UCharacterStatusComponent::ApplyDelta(int32 dAff, int32 dMood, int32 dEn, int32 dFo, int32 dCoin)
{
    Affection = Clamp0100(Affection + dAff);
    Mood = Clamp0100(Mood + dMood);
    Energy = Clamp0100(Energy + dEn);
    Focus = Clamp0100(Focus + dFo);
    Coin = FMath::Max(0, Coin + dCoin);
    RecomputeHeart(); Broadcast();
}

// â°ífÉCÉxÉìÉgÇÃéÛÇØå˚ 
void UCharacterStatusComponent::HandleRecoveryTick(int32 HeartCap)
{
    if (Heart < HeartCap)
    {
        Heart = FMath::Min(Heart + 1, HeartCap);
        Broadcast();
    }
}

void UCharacterStatusComponent::HandleDailyReset()
{
    // ïKóvÇ…âûÇ∂Çƒì˙éüèàóù åyó î≈åJâzÇ»Ç«ÇÕï ëwÇ≈é¿é{Ç∑ÇÈëzíË
    // Ç±Ç±Ç≈ÇÕí ímÇÃÇ›
    Broadcast();
}

void UCharacterStatusComponent::RecomputeHeart()
{
    const float w = FMath::Max(0.0001f, HeartWeights.Affection + HeartWeights.Mood + HeartWeights.Energy + HeartWeights.Focus);
    const float v = HeartWeights.Affection * Affection + HeartWeights.Mood * Mood + HeartWeights.Energy * Energy + HeartWeights.Focus * Focus;
    Heart = FMath::Clamp(FMath::RoundToInt(v / w), 0, 100);
}

FCharacterStatusSnapshot UCharacterStatusComponent::MakeSnapshot() const
{
    FCharacterStatusSnapshot S;
    S.Affection = Affection; S.Mood = Mood; S.Energy = Energy; S.Focus = Focus; S.Coin = Coin; S.Heart = Heart;
    return S;
}

void UCharacterStatusComponent::LoadSnapshot(const FCharacterStatusSnapshot& In)
{
    Affection = Clamp0100(In.Affection);
    Mood = Clamp0100(In.Mood);
    Energy = Clamp0100(In.Energy);
    Focus = Clamp0100(In.Focus);
    Coin = FMath::Max(0, In.Coin);
    Heart = Clamp0100(In.Heart);
    Broadcast();
}

void UCharacterStatusComponent::Broadcast()
{
    OnStatusChanged.Broadcast();
}

// Called every frame
void UCharacterStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


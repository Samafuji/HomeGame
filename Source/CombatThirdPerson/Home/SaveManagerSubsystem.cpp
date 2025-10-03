// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/SaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Home/TaskSaveGame.h"
#include "CharacterInteraction/Character/HomeCharacter.h"
#include "Home/Components/CharacterStatusComponent.h"
#include "EngineUtils.h"


bool USaveManagerSubsystem::SaveAll(UWorld* World)
{
    if (!World) return false;
    UTaskSaveGame* SG = Cast<UTaskSaveGame>(UGameplayStatics::CreateSaveGameObject(UTaskSaveGame::StaticClass()));
    if (!SG) return false;

    for (TActorIterator<AHomeCharacter> It(World); It; ++It)
    {
        AHomeCharacter* HC = *It;
        if (!IsValid(HC) || !HC->Status) continue;

        const FName Key = (HC->CharacterId.IsNone() ? HC->GetFName() : HC->CharacterId);

        const auto Snap = HC->Status->MakeSnapshot();
        FCharacterStatusSG Out;
        Out.Affection = Snap.Affection; Out.Mood = Snap.Mood; Out.Energy = Snap.Energy;
        Out.Focus = Snap.Focus; Out.Coin = Snap.Coin; Out.Heart = Snap.Heart;

        SG->CharacterStatuses.Add(Key, Out);
    }

    return UGameplayStatics::SaveGameToSlot(SG, SlotName, UserIndex);
}

bool USaveManagerSubsystem::LoadAll(UWorld* World)
{
    if (!World) return false;
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex)) return false;

    UTaskSaveGame* SG = Cast<UTaskSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    if (!SG) return false;

    for (TActorIterator<AHomeCharacter> It(World); It; ++It)
    {
        AHomeCharacter* HC = *It;
        if (!IsValid(HC) || !HC->Status) continue;

        const FName Key = (HC->CharacterId.IsNone() ? HC->GetFName() : HC->CharacterId);

        if (FCharacterStatusSG* Found = SG->CharacterStatuses.Find(Key))
        {
            FCharacterStatusSnapshot In;
            In.Affection = Found->Affection; In.Mood = Found->Mood; In.Energy = Found->Energy;
            In.Focus = Found->Focus; In.Coin = Found->Coin; In.Heart = Found->Heart;
            HC->Status->LoadSnapshot(In);
        }
    }
    return true;
}

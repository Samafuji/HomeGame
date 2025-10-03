// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveManagerSubsystem.generated.h"

class UTaskSaveGame;
/**
 * 
 */
UCLASS()
class COMBATTHIRDPERSON_API USaveManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Save") FString SlotName = TEXT("TaskSlot");
    UPROPERTY(EditAnywhere, Category = "Save") int32   UserIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Save") bool SaveAll(UWorld* World);
    UFUNCTION(BlueprintCallable, Category = "Save") bool LoadAll(UWorld* World);
};
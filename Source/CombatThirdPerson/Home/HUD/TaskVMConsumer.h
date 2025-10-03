// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TaskVMConsumer.generated.h"

class UTaskViewModel;

UINTERFACE(BlueprintType)
class UTaskVMConsumer : public UInterface
{
    GENERATED_BODY()
};


/**
 * 
 */
class COMBATTHIRDPERSON_API ITaskVMConsumer
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SetTaskViewModel(UTaskViewModel* InVM);
};
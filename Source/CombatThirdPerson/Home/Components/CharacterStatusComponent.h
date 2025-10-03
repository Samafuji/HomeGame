// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStatusComponent.generated.h"

USTRUCT(BlueprintType)
struct FHeartWeights
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Affection = 0.35f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Mood = 0.25f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Energy = 0.20f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Focus = 0.20f;
};

USTRUCT(BlueprintType)
struct FCharacterStatusSnapshot
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Affection = 50;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Mood = 60;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Energy = 60;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Focus = 40;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Coin = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Heart = 50;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatusChanged);

UCLASS( ClassGroup=(Life), meta=(BlueprintSpawnableComponent) )
class COMBATTHIRDPERSON_API UCharacterStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterStatusComponent();


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

    // 固有状態（0..100系はClamp）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status") int32 Affection = 50;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status") int32 Mood = 60;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status") int32 Energy = 60;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status") int32 Focus = 40;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status") int32 Coin = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status") int32 Heart = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Heart") FHeartWeights HeartWeights;

    UPROPERTY(BlueprintAssignable) FOnStatusChanged OnStatusChanged;

    // 直接操作API（クリック/タスクから呼ぶ）
    UFUNCTION(BlueprintCallable, Category = "Status|Apply") void AddAffection(int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "Status|Apply") void AddMood(int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "Status|Apply") void AddEnergy(int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "Status|Apply") void AddFocus(int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "Status|Apply") void AddCoin(int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "Status|Apply") void ApplyDelta(int32 dAff, int32 dMood, int32 dEn, int32 dFo, int32 dCoin);

    UFUNCTION(BlueprintCallable, Category = "Status|Heart") void RecomputeHeart();
    UFUNCTION(BlueprintCallable, Category = "Status|IO")    FCharacterStatusSnapshot MakeSnapshot() const;
    UFUNCTION(BlueprintCallable, Category = "Status|IO")    void LoadSnapshot(const FCharacterStatusSnapshot& In);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 横断イベントの受け口（Subsystemから呼ばれる／購読する）
    UFUNCTION() void HandleRecoveryTick(int32 HeartCap);
    UFUNCTION() void HandleDailyReset();

private:
    static int32 Clamp0100(int32 V) { return FMath::Clamp(V, 0, 100); }
    void Broadcast();
		
};

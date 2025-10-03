// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HomeAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class COMBATTHIRDPERSON_API UHomeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
    // Controller/Character から与えるターゲット回転（コンポーネント空間）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat")
    FRotator HeadPatRotCS = FRotator::ZeroRotator;

    // 有効度（0～1）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat")
    float HeadPatAlpha = 0.f;

    // 入力回転のスムージング（補間速度）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat")
    float SmoothSpeed = 10.f;

    // ---- 分配ウェイト（自然さ調整。お好みで）----
    // Yaw（左右振り）は上半身まで少し広く分配
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Head_Yaw = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Neck_Yaw = 0.65f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Spine2_Yaw = 0.35f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Upper_Yaw = 0.20f;

    // Pitch（うなずき）は上半身へは弱めに
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Head_Pitch = 0.90f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Neck_Pitch = 0.60f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Spine2_Pitch = 0.25f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Upper_Pitch = 0.10f;

    // 自然さ強化：Yawに対して軽くRoll（肩の入り）を与える
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Head = 0.00f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Neck = 0.05f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Spine2 = 0.10f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Upper = 0.12f;

    // AnimGraph が読む最終分配回転（コンポーネント空間）
    UPROPERTY(BlueprintReadOnly, Category = "HeadPat|Runtime")
    FRotator Out_Head_CS = FRotator::ZeroRotator;
    UPROPERTY(BlueprintReadOnly, Category = "HeadPat|Runtime")
    FRotator Out_Neck_CS = FRotator::ZeroRotator;
    UPROPERTY(BlueprintReadOnly, Category = "HeadPat|Runtime")
    FRotator Out_Spine2_CS = FRotator::ZeroRotator;
    UPROPERTY(BlueprintReadOnly, Category = "HeadPat|Runtime")
    FRotator Out_Upper_CS = FRotator::ZeroRotator;

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    FRotator Smoothed_CS = FRotator::ZeroRotator;
	
};

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
    // Controller/Character ����^����^�[�Q�b�g��]�i�R���|�[�l���g��ԁj
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat")
    FRotator HeadPatRotCS = FRotator::ZeroRotator;

    // �L���x�i0�`1�j
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat")
    float HeadPatAlpha = 0.f;

    // ���͉�]�̃X���[�W���O�i��ԑ��x�j
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat")
    float SmoothSpeed = 10.f;

    // ---- ���z�E�F�C�g�i���R�������B���D�݂Łj----
    // Yaw�i���E�U��j�͏㔼�g�܂ŏ����L�����z
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Head_Yaw = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Neck_Yaw = 0.65f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Spine2_Yaw = 0.35f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Upper_Yaw = 0.20f;

    // Pitch�i���Ȃ����j�͏㔼�g�ւ͎�߂�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Head_Pitch = 0.90f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Neck_Pitch = 0.60f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Spine2_Pitch = 0.25f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float W_Upper_Pitch = 0.10f;

    // ���R�������FYaw�ɑ΂��Čy��Roll�i���̓���j��^����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Head = 0.00f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Neck = 0.05f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Spine2 = 0.10f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeadPat|Weights")
    float RollFromYaw_Upper = 0.12f;

    // AnimGraph ���ǂލŏI���z��]�i�R���|�[�l���g��ԁj
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

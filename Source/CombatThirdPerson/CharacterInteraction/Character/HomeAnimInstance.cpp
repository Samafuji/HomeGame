// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInteraction/Character/HomeAnimInstance.h"

#include "Kismet/KismetMathLibrary.h"

void UHomeAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // ���͉�]���X���[�Y��
    Smoothed_CS = UKismetMathLibrary::RInterpTo(Smoothed_CS, HeadPatRotCS, DeltaSeconds, SmoothSpeed);

    // �L���łȂ���΃[���o��
    if (HeadPatAlpha <= 0.f)
    {
        Out_Head_CS = Out_Neck_CS = Out_Spine2_CS = Out_Upper_CS = FRotator::ZeroRotator;
        return;
    }

    const float Y = Smoothed_CS.Yaw;
    const float P = Smoothed_CS.Pitch;

    // �e�{�[���ւ̕��z�iYaw/Pitch ���ꂼ��ʃE�F�C�g�j
    Out_Head_CS = FRotator(P * W_Head_Pitch, Y * W_Head_Yaw, Y * RollFromYaw_Head);
    Out_Neck_CS = FRotator(P * W_Neck_Pitch, Y * W_Neck_Yaw, Y * RollFromYaw_Neck);
    Out_Spine2_CS = FRotator(P * W_Spine2_Pitch, Y * W_Spine2_Yaw, Y * RollFromYaw_Spine2);
    Out_Upper_CS = FRotator(P * W_Upper_Pitch, Y * W_Upper_Yaw, Y * RollFromYaw_Upper);
}

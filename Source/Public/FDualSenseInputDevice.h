// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/InputDevice/Public/IInputDevice.h"

/**
 * 
 */
class WINDOWSDUALSENSE_DS5W_API FDualSenseInputDevice : public IInputDevice
{
public:
	FDualSenseInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	~FDualSenseInputDevice(){}

	virtual void Tick(float DeltaTime) override {}
	virtual void SendControllerEvents() override {}
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override {}
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override {}
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override {}
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; }

	FPlatformUserId UserId;
	FInputDeviceId InputDeviceId; // ID Device.
	const TSharedRef<FGenericApplicationMessageHandler>& GetMessageHandler() const;

private:
	const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler;
};

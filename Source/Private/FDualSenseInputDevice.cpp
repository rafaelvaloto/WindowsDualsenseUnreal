// Fill out your copyright notice in the Description page of Project Settings.


#include "FDualSenseInputDevice.h"
#include "DualSenseLibrary.h"
#include "Windows/WindowsApplication.h"


FDualSenseInputDevice::FDualSenseInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler):
	FGenericPlatformInputDeviceMapper(true, false),
	MessageHandler(InMessageHandler)
{
}

void FDualSenseInputDevice::Tick(float DeltaTime)
{
	TArray<FInputDeviceId> DeviceIds;
	Get().GetAllInputDevices(DeviceIds);
	
	for (FInputDeviceId& Device : DeviceIds)
	{
		if (MappedInputDevices.Contains(Device))
		{
			if (MappedInputDevices[Device].ConnectionState == EInputDeviceConnectionState::Connected)
			{
				FPlatformUserId UserId = GetUserForInputDevice(Device);
				UDualSenseLibrary::UpdateInput(MessageHandler, UserId, Device);
				continue;
			}
			
			UE_LOG(LogTemp, Log, TEXT("Device ID: %d Is Not Connected"), Device.GetId());
		}
	}
}

void FDualSenseInputDevice::SetDeviceProperty(int32 ControllerId, const FInputDeviceProperty* Property)
{
	if (!Property)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Name! %s"), *Property->Name.ToString());
	if (Property->Name == "InputDeviceLightColor")
	{
		const FInputDeviceLightColorProperty* ColorProperty = static_cast<const FInputDeviceLightColorProperty*>(Property);
		SetLightColor(ControllerId, ColorProperty->Color);
	}
	
	if (Property->Name == "InputDeviceTriggerResistance")
	{
		if (const FInputDeviceTriggerResistanceProperty* DeviceTriggerResistanceProperty = static_cast<const
			FInputDeviceTriggerResistanceProperty*>(Property))
		{
			UDualSenseLibrary::SetTriggerResistance(ControllerId, *DeviceTriggerResistanceProperty);
		}
	}

	if (Property->Name == "InputDeviceTriggerVibration")
	{
		if (const FInputDeviceTriggerVibrationProperty* TriggerVibrationProperty = static_cast<const
			FInputDeviceTriggerVibrationProperty*>(Property))
		{
			UDualSenseLibrary::SetTriggerVibration(ControllerId, *TriggerVibrationProperty);
		}
	}
	
	UDualSenseLibrary::SendOut(ControllerId);
}

void FDualSenseInputDevice::SetLightColor(int32 ControllerId, FColor Color)
{
	UDualSenseLibrary::UpdateColorOutput(ControllerId, Color);
}

void FDualSenseInputDevice::ResetLightColor(int32 ControllerId)
{
}

const TSharedRef<FGenericApplicationMessageHandler>& FDualSenseInputDevice::GetMessageHandler()
{
	return MessageHandler;
}

bool FDualSenseInputDevice::SupportsForceFeedback(int32 ControllerId)
{
	return false;
}

void FDualSenseInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values)
{
	UDualSenseLibrary::SetVibration(ControllerId, Values);
	UDualSenseLibrary::SendOut(ControllerId);
}

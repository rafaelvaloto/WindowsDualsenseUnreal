// Copyright Epic Games, Inc. All Rights Reserved.

#include "WindowsDualsense_ds5w/Public/WindowsDualsense_ds5w.h"

#include "FDualSenseInputDevice.h"
#define LOCTEXT_NAMESPACE "FWindowsDualsense_ds5wModule"

int32 MAX_CONTROLLERS_SUPPORTED = 8;

TSharedPtr<IInputDevice> FWindowsDualsense_ds5wModule::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& InCustomMessageHandler)
{
	DeviceInstances = MakeShareable(new FDualSenseInputDevice(InCustomMessageHandler));
	
	for (int32 i = 0; i < DualSenseLibraryInstance->ControllersCount; i++)
	{
		FPlatformUserId UserId = i > 0 ? DeviceInstances->AllocateNewUserId() : FPlatformUserId::CreateFromInternalId(i);
		FInputDeviceId InputDeviceId = i > 0 ? DeviceInstances->AllocateNewInputDeviceId() : FInputDeviceId::CreateFromInternalId(i);

		EInputDeviceConnectionState ConnectionState =
			DualSenseLibraryInstance->IsConnected(i)
				? EInputDeviceConnectionState::Connected
				: EInputDeviceConnectionState::Disconnected;

		FPlatformInputDeviceState State;
		State.OwningPlatformUser = UserId;
		State.ConnectionState = ConnectionState;
		
		DeviceInstances->SetController(InputDeviceId, State);

		if (DeviceInstances->RemapUserAndDeviceToControllerId(UserId, i, InputDeviceId))
		{
			UE_LOG(LogTemp, Log, TEXT("Success mapper Device registred: %d and User %d"), i, UserId.GetInternalId());
		}
	}
	
	return DeviceInstances;
}

void FWindowsDualsense_ds5wModule::StartupModule()
{
	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);

	FString DLLPath = FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("WindowsDualsense_ds5w/ThirdParty/DualSenseWindows_V0.1/ds5w_x64.dll"));
	DS5WdllHandle = FPlatformProcess::GetDllHandle(*DLLPath);
	if (!DS5WdllHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("DS5WdllHandle DLL Loading Failed!"));
		return;
	}

	DualSenseLibraryInstance = NewObject<UDualSenseLibrary>();
	DualSenseLibraryInstance->InitializeLibrary();
}

void FWindowsDualsense_ds5wModule::ShutdownModule()
{
	if (DualSenseLibraryInstance)
	{
		DualSenseLibraryInstance->ShutdownLibrary();
		DualSenseLibraryInstance = nullptr;
	}

	if (DS5WdllHandle)
	{
		FPlatformProcess::FreeDllHandle(DS5WdllHandle);
		DS5WdllHandle = nullptr;
	}
}

IMPLEMENT_MODULE(FWindowsDualsense_ds5wModule, WindowsDualsense_ds5w)

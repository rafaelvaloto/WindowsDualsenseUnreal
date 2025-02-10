// Copyright Epic Games, Inc. All Rights Reserved.

#include "WindowsDualsense_ds5w/Public/WindowsDualsense_ds5w.h"

#include "Editor.h"
#include "FDualSenseInputDevice.h"
#include "Thread/FInputUpdateTask.h"

#define LOCTEXT_NAMESPACE "FWindowsDualsense_ds5wModule"

FOnControllerDisconnected FInputUpdateTask::OnControllerDisconnected;

TSharedPtr<IInputDevice> FWindowsDualsense_ds5wModule::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	DualSenseDeviceInstance = MakeShareable(new FDualSenseInputDevice(InMessageHandler));
	DualSenseDeviceInstance->InputDeviceId = FInputDeviceId::CreateFromInternalId(0);
	DualSenseDeviceInstance->UserId = FPlatformUserId::CreateFromInternalId(0);

	if (GetDualSenseDeviceInstance().Get())
	{
		DualSenseLibraryInstance->InitializeLibrary(DualSenseDeviceInstance.Get());
		
		InputUpdateTask = new FInputUpdateTask(DualSenseLibraryInstance);
		InputUpdateTask->Init();
		InputThread = FRunnableThread::Create(InputUpdateTask, TEXT("InputUpdateThread"));

		// Disconect device delegate
		FInputUpdateTask::OnControllerDisconnected.AddRaw(this, &FWindowsDualsense_ds5wModule::OnControllerDisconnectedHandler);
	}
	return DualSenseDeviceInstance;
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
	if (!DualSenseLibraryInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("DualSenseLibraryInstance Loading Failed!"));
		return;
	}

	// Editor delegate play game reconnect device
	FEditorDelegates::BeginPIE.AddRaw(this, &FWindowsDualsense_ds5wModule::OnBeginPIE);
}

void FWindowsDualsense_ds5wModule::ShutdownModule()
{
	if (InputThread)
	{
		InputThread->Kill(true);
		delete InputThread;
		InputThread = nullptr;
	}

	if (InputUpdateTask)
	{
		delete InputUpdateTask;
		InputUpdateTask = nullptr;
	}

	if (DualSenseLibraryInstance)
	{
		DualSenseLibraryInstance->ShutdownLibrary();
		DualSenseLibraryInstance = nullptr;
	}

	DualSenseDeviceInstance.Reset();

	if (DS5WdllHandle)
	{
		FPlatformProcess::FreeDllHandle(DS5WdllHandle);
		DS5WdllHandle = nullptr;
	}
}

TSharedPtr<FDualSenseInputDevice> FWindowsDualsense_ds5wModule::GetDualSenseDeviceInstance() const
{
	if (!DualSenseDeviceInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("DualSenseDeviceInstance Not Exists!"));
		return nullptr;
	}

	return DualSenseDeviceInstance.ToSharedRef();
}


// Função chamada quando o jogo começa
void FWindowsDualsense_ds5wModule::OnBeginPIE(const bool bIsSimulating)
{
	UE_LOG(LogTemp, Log, TEXT("WindowsDualsense_ds5wModule: Game Started."));
	ConnectDualSense();	
}

// Conectar o DualSense
void FWindowsDualsense_ds5wModule::ConnectDualSense()
{
	if (GetDualSenseDeviceInstance().Get() && !DualSenseLibraryInstance->IsConnected())
	{
		UE_LOG(LogTemp, Log, TEXT("Reconnecting to DualSense controller..."));
		DualSenseLibraryInstance->Reconnect();
		InputUpdateTask->Init();
		InputThread = FRunnableThread::Create(InputUpdateTask, TEXT("InputUpdateThread"));
	}
}

void FWindowsDualsense_ds5wModule::OnControllerDisconnectedHandler()
{
	DualSenseLibraryInstance->SetConnectionIsValid(false);
	InputThread->Kill(true);
	delete InputThread;
	InputThread = nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWindowsDualsense_ds5wModule, WindowsDualsense_ds5w)

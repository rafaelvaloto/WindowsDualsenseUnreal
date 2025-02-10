// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DualSenseLibrary.h"
#include "FDualSenseInputDevice.h"
#include "Thread/FInputUpdateTask.h"
#include "Modules/ModuleManager.h"
#include "IInputDeviceModule.h"

class FWindowsDualsense_ds5wModule final : public IInputDeviceModule
{
public:
	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Função pública que permite acessar a instância do dispositivo */
	TSharedPtr<FDualSenseInputDevice> GetDualSenseDeviceInstance() const;

private:
	void* DS5WdllHandle = nullptr;
	FRunnableThread* InputThread = nullptr;
	FInputUpdateTask* InputUpdateTask = nullptr;
	UDualSenseLibrary* DualSenseLibraryInstance = nullptr;

	/** DualSense Input Device */
	TSharedPtr<FDualSenseInputDevice> DualSenseDeviceInstance;

	// Conectar o DualSense
	void ConnectDualSense();

	// Desconectar o DualSense
	void OnControllerDisconnectedHandler();
	
	// Função chamada quando o jogo começa
	void OnBeginPIE(const bool bIsSimulating);
};

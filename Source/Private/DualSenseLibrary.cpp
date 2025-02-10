#include "Public/DualSenseLibrary.h"
#include "InputCoreTypes.h" // Required for FPlatformUser and related constants
#include <Windows.h>
#include <iostream>

#include "FDualSenseInputDevice.h"

DS5W::_DeviceContext UDualSenseLibrary::DeviceContext = {};
DS5W::_DS5InputState UDualSenseLibrary::InputState = {};
DS5W::_DeviceEnumInfo* UDualSenseLibrary::Infos;

FDualSenseInputDevice* UDualSenseLibrary::DualSenseInputDevice = nullptr; // Inicializa com nullptr ou outro valor.

UDualSenseLibrary::UDualSenseLibrary(
	): bIsInitialized(false)
{
}

bool UDualSenseLibrary::Reconnect()
{
	if (!DualSenseInputDevice)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to reconnect DualSense Library: Device is null"));
		return true;
	}
	
	return Connection();
}

bool UDualSenseLibrary::InitializeLibrary(FDualSenseInputDevice* Device)
{
	if (!Device)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize DualSense Library: Device is null"));
		return false; // Falha se o dispositivo for nulo
	}

	if (!DualSenseInputDevice)
	{
		DualSenseInputDevice = Device;
	}

	return Connection();
}

bool UDualSenseLibrary::Connection()
{
	Infos = new DS5W::_DeviceEnumInfo[16];

	unsigned int ControllersCount = 0;
	switch (DS5W::enumDevices(Infos, 16, &ControllersCount))
	{
	case DS5W_OK:

	case DS5W_E_INSUFFICIENT_BUFFER:
		break;

	default:
		std::cout << "Error enumDevices default: " << GetLastError() << std::endl;
		return false;
	}

	std::cout << "Controllers DualSense Count" << ControllersCount << std::endl;

	// Check number of controllers
	if (!ControllersCount)
	{
		std::cout << "Error !Controllers DualSense Count" << GetLastError() << std::endl;
		return false;
	}

	if (DS5W_FAILED(DS5W::initDeviceContext(&Infos[0], &DeviceContext)))
	{
		std::cout << "Error !DS5W_FAILED(DS5W::initDeviceContext(&infos[0], &con))" << GetLastError() << std::endl;
		return false;
	}

	if (!DS5W_SUCCESS(DS5W::getDeviceInputState(&DeviceContext, &InputState)))
	{
		std::cout << "Error not found getDeviceInputState: " << GetLastError() << std::endl;
		SetConnectionIsValid(false);
		return false;
	}

	SetConnectionIsValid(true);
	return true;
}

void UDualSenseLibrary::ShutdownLibrary()
{
	DS5W::freeDeviceContext(&DeviceContext);
	UE_LOG(LogTemp, Log, TEXT("DualSense controller disconnected with success."));
}

bool UDualSenseLibrary::UpdateInput()
{
	if (DS5W_SUCCESS(DS5W::getDeviceInputState(&DeviceContext, &InputState)))
	{
		// Unreal MessageHandler Context
		FGenericApplicationMessageHandler* MessageHandler = &DualSenseInputDevice->GetMessageHandler().Get();

		// Read Buttons
		bool bCross = InputState.buttonsAndDpad & DS5W_ISTATE_BTX_CROSS;
		bool bCircle = InputState.buttonsAndDpad & DS5W_ISTATE_BTX_CIRCLE;
		bool bSquare = InputState.buttonsAndDpad & DS5W_ISTATE_BTX_SQUARE;
		bool bTriangle = InputState.buttonsAndDpad & DS5W_ISTATE_BTX_TRIANGLE;

		// Reseat the main threads
		AsyncTask(ENamedThreads::GameThread, [bCross, bCircle, bSquare, bTriangle, MessageHandler]()
		{
			const FPlatformUserId UserId = DualSenseInputDevice->UserId;
			const FInputDeviceId InputDeviceId = DualSenseInputDevice->InputDeviceId;
			
			if (bCross)
			{
				MessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Bottom.GetFName(), UserId, InputDeviceId, false);
			}

			if (bCircle)
			{
				MessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Right.GetFName(), UserId, InputDeviceId,false);
			}

			if (bSquare)
			{
				MessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Left.GetFName(), UserId, InputDeviceId,false);
			}

			if (bTriangle)
			{
				MessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Top.GetFName(), UserId, InputDeviceId,false);
			}
		});
		

		// Read Analogs
		const FVector2D LeftStick(InputState.leftStick.x / 128.0f, InputState.leftStick.y / 128.0f);
		const FVector2D RightStick(InputState.rightStick.x / 128.0f, InputState.rightStick.y / 128.0f);
		AsyncTask(ENamedThreads::GameThread, [LeftStick, MessageHandler]()
		{
			const FPlatformUserId UserId = DualSenseInputDevice->UserId;
			const FInputDeviceId InputDeviceId = DualSenseInputDevice->InputDeviceId;
			
			MessageHandler->OnControllerAnalog(EKeys::Gamepad_LeftStick_Right.GetFName(), UserId, InputDeviceId, LeftStick.X);
			MessageHandler->OnControllerAnalog(EKeys::Gamepad_LeftStick_Up.GetFName(), UserId, InputDeviceId, LeftStick.Y);
		});
		AsyncTask(ENamedThreads::GameThread, [RightStick, MessageHandler]()
		{
			const FPlatformUserId UserId = DualSenseInputDevice->UserId;
			const FInputDeviceId InputDeviceId = DualSenseInputDevice->InputDeviceId;

			MessageHandler->OnControllerAnalog(EKeys::Gamepad_RightStick_Right.GetFName(), UserId, InputDeviceId, RightStick.X);
			MessageHandler->OnControllerAnalog(EKeys::Gamepad_RightStick_Up.GetFName(), UserId, InputDeviceId, RightStick.Y);
		});


		// Read Triggers
		float LeftTrigger = InputState.leftTrigger / 256.0f;
		float RightTrigger = InputState.rightTrigger / 256.0f;
		AsyncTask(ENamedThreads::GameThread, [LeftTrigger, RightTrigger, MessageHandler]()
		{
			const FPlatformUserId UserId = DualSenseInputDevice->UserId;
			const FInputDeviceId InputDeviceId = DualSenseInputDevice->InputDeviceId;
			
			MessageHandler->OnControllerAnalog(EKeys::Gamepad_LeftTrigger.GetFName(), UserId, InputDeviceId, LeftTrigger);
			MessageHandler->OnControllerAnalog(EKeys::Gamepad_RightTrigger.GetFName(), UserId, InputDeviceId, RightTrigger);
		});

		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("DualSense disconnected, turn on the DualSense device and press StartGame in the unreal editor to reconnect it."));
	return false;
}

bool UDualSenseLibrary::IsConnected() const
{
	return bIsInitialized;
}

void UDualSenseLibrary::SetConnectionIsValid(bool IsValid)
{
	bIsInitialized = IsValid;
}

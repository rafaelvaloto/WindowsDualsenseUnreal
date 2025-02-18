#include "Public/DualSenseLibrary.h"
#include "InputCoreTypes.h" // Required for FPlatformUser and related constants
#include <Windows.h>
#include <iostream>
#include <algorithm>


#define MAX_DEVICES 16

FCriticalSection Mutex;


DS5W::_DeviceEnumInfo* UDualSenseLibrary::Infos;
TMap<int32, DS5W::_DS5InputState> UDualSenseLibrary::InputState;
TMap<int32, DS5W::DS5OutputState> UDualSenseLibrary::OutputState;
TMap<int32, DS5W::DeviceContext> UDualSenseLibrary::DeviceContexts;

int32 UDualSenseLibrary::ControllersCount = 0;
TMap<int32, bool> UDualSenseLibrary::IsInitialized;

bool UDualSenseLibrary::Reconnect(int32 ControllerId)
{
	if (!DeviceContexts.Contains(ControllerId))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to reconnect DualSense Library: Device is null"));
		return false;
	}

	if (DS5W_SUCCESS(DS5W::reconnectDevice(&DeviceContexts[ControllerId])))
	{
		return true;
	}

	return false;
}

bool UDualSenseLibrary::ValidateInputState(int32 ControllerId)
{
	FScopeLock Lock(&Mutex);

	if (!DeviceContexts.Contains(ControllerId) || !InputState.Contains(ControllerId))
	{
		return false;
	}
	
	int32 UserIndex = FSlateApplication::Get().GetUserIndexForController(ControllerId);
	if (UserIndex < 0)
	{
		return false;
	}

	if (!FSlateApplication::IsInitialized())
	{
		return false;
	}
	
	return true;
}

bool UDualSenseLibrary::InitializeLibrary()
{
	Infos = new DS5W::_DeviceEnumInfo[MAX_DEVICES];

	unsigned int Count = 0;
	switch (DS5W::enumDevices(Infos, MAX_DEVICES, &Count))
	{
	case DS5W_OK:

	case DS5W_E_INSUFFICIENT_BUFFER:
		break;

	default:
		std::cout << "Error enumDevices default: " << GetLastError() << std::endl;
		return false;
	}

	ControllersCount = static_cast<int32>(Count);
	return Connection();
}

bool UDualSenseLibrary::Connection()
{
	for (int32 i = 0; i < ControllersCount; i++)
	{
		if (OutputState.Contains(i))
		{
			ZeroMemory(&OutputState[i], sizeof(DS5W::DS5OutputState));
		}

		if (InputState.Contains(i))
		{
			ZeroMemory(&InputState[i], sizeof(DS5W::_DS5InputState));
		}

		if (DeviceContexts.Contains(i))
		{
			DS5W::freeDeviceContext(&DeviceContexts[i]);
		}

		DS5W::DeviceContext Context;
		if (DS5W_SUCCESS(DS5W::initDeviceContext(&Infos[i], &Context)))
		{
			DS5W::_DS5InputState InState;
			if (!DS5W_SUCCESS(DS5W::getDeviceInputState(&Context, &InState)))
			{
				UE_LOG(LogTemp, Error, TEXT("Error DeviceInputState: %d"), i);
				continue;
			}

			InputState.Add(i, InState);
			DeviceContexts.Add(i, Context);
			OutputState.Add(i, DS5W::DS5OutputState());

			ZeroMemory(&Context, sizeof(DS5W::DeviceContext));
			ZeroMemory(&InState, sizeof(DS5W::_DS5InputState));

			if (IsInitialized.Contains(i))
			{
				IsInitialized[i] = true;
				continue;
			}

			IsInitialized.Add(i, true);
			continue;
		}

		if (IsInitialized.Contains(i))
		{
			UE_LOG(LogTemp, Error, TEXT("Error initDeviceContext: %d"), i);
			IsInitialized[i] = false;
			continue;
		}

		UE_LOG(LogTemp, Error, TEXT("Error initDeviceContext: %d"), i);
		IsInitialized.Add(i, false);
	}

	return true;
}

void UDualSenseLibrary::ShutdownLibrary()
{
	for (int32 i = 0; i < ControllersCount; i++)
	{
		if (IsInitialized[i])
		{
			ZeroMemory(&OutputState[i], sizeof(DS5W::DS5OutputState));
			DS5W::freeDeviceContext(&DeviceContexts[i]);
			UE_LOG(LogTemp, Log, TEXT("DualSense ControllerId %d disconnected with success."), i);
		}
	}

	IsInitialized.Reset();
}

bool UDualSenseLibrary::UpdateInput(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler,
	const FPlatformUserId UserId,
	const FInputDeviceId InputDeviceId
)
{
	if (DS5W_SUCCESS(DS5W::getDeviceInputState(&DeviceContexts[InputDeviceId.GetId()], &InputState[InputDeviceId.GetId()])))
	{
		
		// Read Buttons
		bool bCross = InputState[InputDeviceId.GetId()].buttonsAndDpad & DS5W_ISTATE_BTX_CROSS;
		bool bCircle = InputState[InputDeviceId.GetId()].buttonsAndDpad & DS5W_ISTATE_BTX_CIRCLE;
		bool bSquare = InputState[InputDeviceId.GetId()].buttonsAndDpad & DS5W_ISTATE_BTX_SQUARE;
		bool bTriangle = InputState[InputDeviceId.GetId()].buttonsAndDpad & DS5W_ISTATE_BTX_TRIANGLE;

		if (bCross)
		{
			InMessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Bottom.GetFName(), UserId,
			                                            InputDeviceId, false);
		}

		if (bCircle)
		{
			InMessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Right.GetFName(), UserId,
			                                            InputDeviceId, false);
		}

		if (bSquare)
		{
			InMessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Left.GetFName(), UserId,
			                                            InputDeviceId, false);
		}

		if (bTriangle)
		{
			InMessageHandler->OnControllerButtonPressed(EKeys::Gamepad_FaceButton_Top.GetFName(), UserId, InputDeviceId,
			                                            false);
		}

		// Read Analogs
		const FVector2D LeftStick(InputState[InputDeviceId.GetId()].leftStick.x / 128.0f,
		                          InputState[InputDeviceId.GetId()].leftStick.y / 128.0f);
		InMessageHandler->OnControllerAnalog(EKeys::Gamepad_LeftStick_Right.GetFName(), UserId, InputDeviceId,
		                                     LeftStick.X);
		InMessageHandler->OnControllerAnalog(EKeys::Gamepad_LeftStick_Down.GetFName(), UserId, InputDeviceId,
		                                     LeftStick.Y);

		const FVector2D RightStick(InputState[InputDeviceId.GetId()].rightStick.x / 128.0f,
		                           InputState[InputDeviceId.GetId()].rightStick.y / 128.0f);
		InMessageHandler->OnControllerAnalog(EKeys::Gamepad_RightStick_Right.GetFName(), UserId, InputDeviceId,
		                                     RightStick.X);
		InMessageHandler->OnControllerAnalog(EKeys::Gamepad_RightStick_Up.GetFName(), UserId, InputDeviceId,
		                                     RightStick.Y);

		// Read Triggers
		float LeftTrigger = InputState[InputDeviceId.GetId()].leftTrigger / 256.0f;
		float RightTrigger = InputState[InputDeviceId.GetId()].rightTrigger / 256.0f;
		InMessageHandler->OnControllerAnalog(EKeys::Gamepad_LeftTrigger.GetFName(), UserId, InputDeviceId, LeftTrigger);
		InMessageHandler->OnControllerAnalog(EKeys::Gamepad_RightTrigger.GetFName(), UserId, InputDeviceId,
		                                     RightTrigger);
		return true;
	}
	return false;
}

bool UDualSenseLibrary::IsConnected(int32 ControllerId)
{
	if (!IsInitialized.Contains(ControllerId))
	{
		return false;
	}

	return IsInitialized[ControllerId];
}

void UDualSenseLibrary::SetConnectionIsValid(int32 ControllerId, bool IsValid)
{
	if (!IsInitialized.Contains(ControllerId))
	{
		return;
	}
	IsInitialized[ControllerId] = IsValid;
}

void UDualSenseLibrary::UpdateColorOutput(int32 ControllerId, const FColor Color)
{
	if (!OutputState.Contains(ControllerId))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to update DualSense Library: Device is null"));
		return;
	}

	OutputState[ControllerId].lightbar.r = Color.R;
	OutputState[ControllerId].lightbar.g = Color.G;
	OutputState[ControllerId].lightbar.b = Color.B;
}

void UDualSenseLibrary::SetTriggerVibration(int32 ControllerId,
                                            FInputDeviceTriggerVibrationProperty TriggerVibrationProperty)
{
	if (TriggerVibrationProperty.AffectedTriggers == EInputDeviceTriggerMask::All)
	{
		OutputState[ControllerId].rightTriggerEffect.effectType = DS5W::_TriggerEffectType::ContinuousResitance;
		OutputState[ControllerId].rightTriggerEffect.Continuous.startPosition = 0;
		OutputState[ControllerId].rightTriggerEffect.Continuous.force = TriggerVibrationProperty.VibrationFrequency;
	}
}

void UDualSenseLibrary::SetTriggerResistance(int32 ControllerId,
                                             FInputDeviceTriggerResistanceProperty TriggerResistenceProperty)
{
	UE_LOG(LogTemp, Error, TEXT("ControllerId , %d"), ControllerId)
	if (TriggerResistenceProperty.AffectedTriggers == EInputDeviceTriggerMask::All)
	{
		if (OutputState.Contains(ControllerId))
		{
			OutputState[ControllerId].rightTriggerEffect.effectType = DS5W::_TriggerEffectType::ContinuousResitance;
			OutputState[ControllerId].rightTriggerEffect.Continuous.startPosition = TriggerResistenceProperty.
				StartPosition;
			OutputState[ControllerId].rightTriggerEffect.Continuous.force = TriggerResistenceProperty.StartStrengh;

			OutputState[ControllerId].leftTriggerEffect.effectType = DS5W::_TriggerEffectType::SectionResitance;
			OutputState[ControllerId].leftTriggerEffect.Section.startPosition = TriggerResistenceProperty.StartPosition;
			OutputState[ControllerId].leftTriggerEffect.Section.endPosition = TriggerResistenceProperty.EndPosition;

			OutputState[ControllerId].rightTriggerEffect.effectType = DS5W::_TriggerEffectType::SectionResitance;
			OutputState[ControllerId].rightTriggerEffect.Section.startPosition = TriggerResistenceProperty.
				StartPosition;
			OutputState[ControllerId].rightTriggerEffect.Section.endPosition = TriggerResistenceProperty.EndPosition;

			OutputState[ControllerId].leftRumble = InputState[ControllerId].leftTrigger;
			OutputState[ControllerId].rightRumble = InputState[ControllerId].rightTrigger;
		}
	}
}

void UDualSenseLibrary::SetVibration(int32 ControllerId, FForceFeedbackValues Vibration)
{
	if (OutputState.Contains(ControllerId))
	{
		OutputState[ControllerId].leftRumble = max(ConvertTo255(Vibration.LeftLarge),
		                                           ConvertTo255(Vibration.RightLarge));
		OutputState[ControllerId].rightRumble = max(ConvertTo255(Vibration.LeftSmall),
		                                            ConvertTo255(Vibration.RightSmall));
	}
}

int UDualSenseLibrary::ConvertForceTriggersTo255(int Value)
{
	float Min = 0;
	float Max = 8;
	float NormalizedPosition = (Value - Min) / (Max - Min);
	int Value255 = static_cast<int>(NormalizedPosition * 255);
	return std::clamp(Value255, 0, 255);
}

int UDualSenseLibrary::ConvertTo255(float Value)
{
	float Min = 0;
	float Max = 1.0;
	float NormalizedPosition = (Value - Min) / (Max - Min);
	int Value255 = static_cast<int>(NormalizedPosition * 255);
	return std::clamp(Value255, 0, 255);
}

void UDualSenseLibrary::SendOut(int32 ControllerId)
{
	if (!DeviceContexts.Contains(ControllerId) || !OutputState.Contains(ControllerId))
	{
		return;
	}

	DS5W::setDeviceOutputState(&DeviceContexts[ControllerId], &OutputState[ControllerId]);
}

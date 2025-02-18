#pragma once

#include "ds5w.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DualSenseLibrary.generated.h"


/**
 * 
 */
UCLASS()
class WINDOWSDUALSENSE_DS5W_API UDualSenseLibrary : public UObject
{
	GENERATED_BODY()

public:
	bool InitializeLibrary();
	void ShutdownLibrary();
	
	bool IsConnected(int32 ControllerId);
	void SetConnectionIsValid(int32 ControllerId, bool IsValid);

	static bool UpdateInput(
		const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler,
		const FPlatformUserId UserId,
		const FInputDeviceId InputDeviceId
	);
	static bool ValidateInputState(int32 ControllerId);

	static bool Reconnect(int32 ControllerId);
	static void UpdateColorOutput(int32 ControllerId, FColor Color);
	static void SetVibration(int32 ControllerId, FForceFeedbackValues Vibration);
	static void SetTriggerVibration(int32 ControllerId, FInputDeviceTriggerVibrationProperty TriggerVibrationProperty);
	static void SetTriggerResistance(int32 ControllerId,
	                                 FInputDeviceTriggerResistanceProperty TriggerResistenceProperty);

	static void SendOut(int32 ControllerId);
	static int ConvertForceTriggersTo255(int Value);
	static int ConvertTo255(float Value);

	static int32 ControllersCount;

private:
	// Send context DualSense to Unreal
	static DS5W::_DeviceEnumInfo* Infos;
	static TMap<int32, DS5W::_DS5InputState> InputState;
	static TMap<int32, DS5W::DS5OutputState> OutputState;
	static TMap<int32, DS5W::DeviceContext> DeviceContexts;
	static TMap<int32, bool> IsInitialized;
	
	bool Connection();
};

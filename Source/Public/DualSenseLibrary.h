
#pragma once

#include "ds5w.h"
#include "CoreMinimal.h"
#include "FDualSenseInputDevice.h"
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
	UDualSenseLibrary();

	bool InitializeLibrary(FDualSenseInputDevice* Device);
	void ShutdownLibrary();
	
	bool Reconnect();
	bool IsConnected() const;
	void SetConnectionIsValid(bool IsValid);

	static bool UpdateInput();
	
	virtual bool SupportsForceFeedback(int32 ControllerId) { return true; }
	virtual void SetLightColor(int32 ControllerId, FColor Color) {};
	virtual void ResetLightColor(int32 ControllerId) {};
	virtual void SetDeviceProperty(int32 ControllerId, const FInputDeviceProperty* Property) {}

	static FDualSenseInputDevice* DualSenseInputDevice; // Ponteiro para o dispositivo
	
private:
	// Send context DualSense to Unreal
	static DS5W::_DeviceEnumInfo* Infos;
	static DS5W::_DS5InputState InputState;
	static DS5W::DS5OutputState OutputState;
	static DS5W::DeviceContext DeviceContext;
	
	bool bIsInitialized;
	bool Connection();
};

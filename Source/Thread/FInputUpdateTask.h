#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "DualSenseLibrary.h"

/**
 * 
 */
class WINDOWSDUALSENSE_DS5W_API FInputUpdateTask final : public FRunnable
{
public:
	FInputUpdateTask(
		UDualSenseLibrary* InDualSenseLibrary,
		const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler,
		int32 ControllerId,
		const FPlatformUserId IsUserId,
		const FInputDeviceId IsInputDeviceId
		) : ControllerId(ControllerId), bIsRunning(false), DualSenseLibrary(InDualSenseLibrary), MessageHandler(InMessageHandler), UserId(IsUserId), InputDeviceId(IsInputDeviceId)
	{
	}

	virtual ~FInputUpdateTask()
	{
		Cleanup();
	}

	static void Cleanup()
	{
	}

	// Thread Start
	virtual bool Init() override
	{
		bIsRunning = true;
		return bIsRunning;
	}

	// loop read buffer 
	virtual uint32 Run() override
	{
		UE_LOG(LogTemp, Log, TEXT("DualSenseLibrary Is Running... %d"), bIsRunning);
		
		while (bIsRunning)
		{
			FPlatformProcess::Sleep(3.0f); // Loop At 10ms
		}
		return 1;
	}

	// Cleanup end thread
	virtual void Exit() override
	{
	}

	int32 ControllerId;

private:
	bool bIsRunning;
	UDualSenseLibrary* DualSenseLibrary;
	const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler;

	const FPlatformUserId UserId;
	const FInputDeviceId InputDeviceId;
};

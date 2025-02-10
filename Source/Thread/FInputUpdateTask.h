#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "DualSenseLibrary.h"
#include "Delegates/Delegate.h" // Para TMulticastDelegate

DECLARE_MULTICAST_DELEGATE(FOnControllerDisconnected);

/**
 * 
 */
class WINDOWSDUALSENSE_DS5W_API FInputUpdateTask final : public FRunnable
{
public:
	FInputUpdateTask(UDualSenseLibrary* InDualSenseLibrary) : bIsRunning(false), DualSenseLibrary(InDualSenseLibrary)
	{
	}

	virtual ~FInputUpdateTask()
	{
		Cleanup();
	}

	static void Cleanup()
	{
		OnControllerDisconnected.Clear(); // Remove todos os binds
	}

	// Thread Start
	virtual bool Init() override
	{
		bIsRunning = true;
		return true;
	}

	// loop read buffer 
	virtual uint32 Run() override
	{
		UE_LOG(LogTemp, Log, TEXT("DualSenseLibrary Is Running... %d"), bIsRunning);
		
		while (bIsRunning)
		{
			if (!DualSenseLibrary->UpdateInput())
			{
				bIsRunning = false;
				UE_LOG(LogTemp, Log, TEXT("DualSenseLibrary Is Running... %d"), bIsRunning);
				return 1;
			}

			FPlatformProcess::Sleep(0.01f); // Loop At 10ms
		}
		return 1;
	}

	// Cleanup end thread
	virtual void Exit() override
	{
		OnControllerDisconnected.Broadcast();
	}

	static FOnControllerDisconnected OnControllerDisconnected;

private:
	bool bIsRunning;
	UDualSenseLibrary* DualSenseLibrary;
};

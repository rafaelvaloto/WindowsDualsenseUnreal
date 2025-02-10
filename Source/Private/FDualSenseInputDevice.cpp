// Fill out your copyright notice in the Description page of Project Settings.


#include "FDualSenseInputDevice.h"


FDualSenseInputDevice::FDualSenseInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
		: MessageHandler(InMessageHandler)
{
}

const TSharedRef<FGenericApplicationMessageHandler>& FDualSenseInputDevice::GetMessageHandler() const
{
	return MessageHandler;
}

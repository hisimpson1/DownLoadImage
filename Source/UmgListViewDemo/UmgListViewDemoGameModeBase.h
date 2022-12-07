// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UmgListViewDemoGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class UMGLISTVIEWDEMO_API AUmgListViewDemoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUmgListViewDemoGameModeBase();
	virtual void StartPlay() override;

protected:
	TSubclassOf<class UUserWidget> UI_MainClass;

	UPROPERTY()
	class UUI_Main* UI_Main;
};

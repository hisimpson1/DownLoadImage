// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "UmgListViewDemoGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI_Main.h"

#pragma optimize( "", off )

AUmgListViewDemoGameModeBase::AUmgListViewDemoGameModeBase()
{
	UE_LOG(LogTemp, Warning, TEXT("%s (%d)"), *FString(__FUNCTION__), __LINE__);

	static ConstructorHelpers::FClassFinder<UUserWidget> UI_WIDGET(TEXT("/Game/BluePrint/BP_UI_Main.BP_UI_Main_C"));

	if (UI_WIDGET.Succeeded())
	{
		UI_MainClass = UI_WIDGET.Class;
	}
}

void AUmgListViewDemoGameModeBase::StartPlay()
{
	if (GetWorld() == nullptr)
		return;

	if (UI_MainClass)
	{
		UUserWidget* UserWidget = CreateWidget<UUserWidget>(GetWorld(), UI_MainClass);
		UI_Main = Cast<UUI_Main>(UserWidget);
		UI_Main->AddToViewport();
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	PlayerController->SetInputMode(FInputModeUIOnly());
	//PlayerController->SetShowMouseCursor(true); //4.26
	PlayerController->bShowMouseCursor = 1;  //4.24
}

#pragma optimize( "", on )
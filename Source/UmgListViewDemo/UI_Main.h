// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_Main.generated.h"

/**
 * 
 */
UCLASS()
class UMGLISTVIEWDEMO_API UUI_Main : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UUI_Main(const FObjectInitializer& ObjectInitializer);
	virtual bool Initialize() override;

protected:
	void InitUI();
	void SaveImage(UTexture2DDynamic* Texture);

	UFUNCTION()
	void OnGetTexture2D(UTexture2DDynamic* _texture);
	UFUNCTION()
	void OnLoadTexture(UTexture2D* Texture);

	void SyncLoadTexure(const FString FileName);
	void LoadTexture();

protected:
	UPROPERTY()
	class UAsyncTaskDownloadImage* mDownloadTask;
	//UPROPERTY()
	//class UMyDownloadImage* mDownloadTask;
	UPROPERTY()
	class UImageLoader* mImageLoader;

protected:
	UPROPERTY()
	class UImage* Image_download;
};

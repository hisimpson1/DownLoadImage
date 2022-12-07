// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "MyDownloadImage.generated.h"

class UTexture2DDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyDownloadImageDelegate, UTexture2DDynamic*, Texture);

UCLASS()
class UMGLISTVIEWDEMO_API UMyDownloadImage : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=( BlueprintInternalUseOnly="true" ))
	static UMyDownloadImage* DownloadImage(FString URL);

public:

	UPROPERTY(BlueprintAssignable)
	FMyDownloadImageDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FMyDownloadImageDelegate OnFail;

public:

	void Start(FString URL);

private:

	/** Handles image requests coming from the web */
	void HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};

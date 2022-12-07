// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MyDownloadImage.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"

//FFileManagerGeneric 디렉토리 검사
#include "HAL/FileManagerGeneric.h"

//FileHelper 파일 저장 , FFileHelper가 빌드는 되지만 소스에서 빨간색으로 보여서 추가
#include "Misc/FileHelper.h"


#pragma optimize( "", off ) 
//----------------------------------------------------------------------//
// UAsyncTaskDownloadImage
//----------------------------------------------------------------------//

#if !UE_SERVER

static void WriteRawToTexture_RenderThread(FTexture2DDynamicResource* TextureResource, const TArray<uint8>& RawData, bool bUseSRGB = true)
{
	check(IsInRenderingThread());

	FRHITexture2D* TextureRHI = TextureResource->GetTexture2DRHI();

	int32 Width = TextureRHI->GetSizeX();
	int32 Height = TextureRHI->GetSizeY();

	uint32 DestStride = 0;
	uint8* DestData = reinterpret_cast<uint8*>(RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false, false));

	for (int32 y = 0; y < Height; y++)
	{
		uint8* DestPtr = &DestData[(Height - 1 - y) * DestStride];

		const FColor* SrcPtr = &((FColor*)(RawData.GetData()))[(Height - 1 - y) * Width];
		for (int32 x = 0; x < Width; x++)
		{
			*DestPtr++ = SrcPtr->B;
			*DestPtr++ = SrcPtr->G;
			*DestPtr++ = SrcPtr->R;
			*DestPtr++ = SrcPtr->A;
			SrcPtr++;
		}
	}

	RHIUnlockTexture2D(TextureRHI, 0, false, false);
}

#endif

UMyDownloadImage::UMyDownloadImage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if ( HasAnyFlags(RF_ClassDefaultObject) == false )
	{
		AddToRoot();
	}
}

UMyDownloadImage* UMyDownloadImage::DownloadImage(FString URL)
{
	UMyDownloadImage* DownloadTask = NewObject<UMyDownloadImage>();
	DownloadTask->Start(URL);

	return DownloadTask;
}

void UMyDownloadImage::Start(FString URL)
{
#if !UE_SERVER
	// Create the Http request and add to pending request list
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UMyDownloadImage::HandleImageRequest);
	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
#else
	// On the server we don't execute fail or success we just don't fire the request.
	RemoveFromRoot();
#endif
}

UTexture2D * CreateTexture2D(const TArray64<uint8>& PixelData, int32 InSizeX, int32 InSizeY, EPixelFormat InFormat, FName BaseName)
{
	UTexture2D* NewTexture = nullptr;
	if (InSizeX > 0 && InSizeY > 0 &&
		(InSizeX % GPixelFormats[InFormat].BlockSizeX) == 0 &&
		(InSizeY % GPixelFormats[InFormat].BlockSizeY) == 0)
	{
		NewTexture = NewObject<UTexture2D>(
			GetTransientPackage(),
			BaseName,
			RF_Transient
			);

		NewTexture->PlatformData = new FTexturePlatformData();
		NewTexture->PlatformData->SizeX = InSizeX;
		NewTexture->PlatformData->SizeY = InSizeY;
		NewTexture->PlatformData->PixelFormat = InFormat;
		NewTexture->SRGB = true;
		//NewTexture->CompressionSettings = TC_HDR;

		// Allocate first mipmap.
		int32 NumBlocksX = InSizeX / GPixelFormats[InFormat].BlockSizeX;
		int32 NumBlocksY = InSizeY / GPixelFormats[InFormat].BlockSizeY;
		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		NewTexture->PlatformData->Mips.Add(Mip);
		Mip->SizeX = InSizeX;
		Mip->SizeY = InSizeY;
		Mip->BulkData.Lock(LOCK_READ_WRITE);
		void* TextureData = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[InFormat].BlockBytes);
		FMemory::Memcpy(TextureData, PixelData.GetData(), PixelData.Num());
		Mip->BulkData.Unlock();
		NewTexture->UpdateResource();
	}
	else
	{
		//UE_LOG(LogImageManager, Warning, TEXT("Invalid parameters specified for ImageManager::CreateTexture2D()"));
	}
	return NewTexture;
}

void UMyDownloadImage::HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
#if !UE_SERVER

	RemoveFromRoot();

	if ( bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0 )
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrappers[3] =
		{
			ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
		};

		for ( auto ImageWrapper : ImageWrappers )
		{
			if ( ImageWrapper.IsValid() && ImageWrapper->SetCompressed(HttpResponse->GetContent().GetData(), HttpResponse->GetContentLength()) )
			{
				const TArray<uint8>* RawData = NULL;
				const ERGBFormat InFormat = (GShaderPlatformForFeatureLevel[GMaxRHIFeatureLevel] == SP_OPENGL_ES2_WEBGL) ? ERGBFormat::RGBA : ERGBFormat::BGRA;
				if ( ImageWrapper->GetRaw(InFormat, 8, RawData) )
				{
					if ( UTexture2DDynamic* Texture = UTexture2DDynamic::Create(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()) )
					{
						Texture->SRGB = true;
						Texture->UpdateResource();

						FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->Resource);
						TArray<uint8> RawDataCopy = *RawData;
						ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
							[TextureResource, RawDataCopy](FRHICommandListImmediate& RHICmdList)
							{
								WriteRawToTexture_RenderThread(TextureResource, RawDataCopy);
							});											
						OnSuccess.Broadcast(Texture);
						return;
					}
				}
			}
		}
	}

	OnFail.Broadcast(nullptr);

#endif
}
#pragma optimize( "", on ) 
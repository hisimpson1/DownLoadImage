// Fill out your copyright notice in the Description page of Project Settings.
#include "UI_Main.h"
#include "Components/Image.h"
#include "Engine/Canvas.h"
#include "RunTime/UMG/Public/Blueprint/AsyncTaskDownloadImage.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Texture2DDynamic.h"
#include "MyDownloadImage.h"

#include "Runtime/ImageWriteQueue/Public/ImageWriteBlueprintLibrary.h"
#include "ImageLoader.h"

FString FullFIlePath = TEXT("D:\\work_unreal4243\\BasicUITest_downloadImage\\Download\\test.png");

#pragma optimize( "", off ) 
UUI_Main::UUI_Main(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UUI_Main::Initialize()
{
	if (Super::Initialize() == false)
		return false;

	InitUI();
	return true;
}

void UUI_Main::InitUI()
{

	//Start downloading the image.
	//https://www.pngwing.com/ 무료 PNG
	//FString URL = TEXT("https://w7.pngwing.com/pngs/452/33/png-transparent-gold-texture-mapping-gold-textured-background-texture-yellow-surface-texture-gold-coin-photography.png");
	FString URL = TEXT("https://w7.pngwing.com/pngs/561/342/png-transparent-tree-large-green-tree-green-tree-illustration-leaf-branch-presentation.png");

	if (FPaths::FileExists(FullFIlePath))
	{
		LoadTexture();
	}
	else
	{
		mDownloadTask = NewObject<UAsyncTaskDownloadImage>();
		//mDownloadTask = NewObject<UMyDownloadImage>();
		mDownloadTask->OnSuccess.AddDynamic(this, &UUI_Main::OnGetTexture2D);
		mDownloadTask->Start(URL);
	}
}

void UUI_Main::SyncLoadTexure(const FString FileName)
{
	UTexture2D* Texture = UKismetRenderingLibrary::ImportFileAsTexture2D(GetWorld(), FileName);
	Image_download->SetBrushFromTexture(Texture);
}

void UUI_Main::OnLoadTexture(UTexture2D* Texture)
{
	Image_download->SetBrushFromTexture(Texture);
}
#pragma optimize( "", on ) 

void UUI_Main::OnGetTexture2D(UTexture2DDynamic* _texture)
{
	Image_download->SetBrushFromTextureDynamic(_texture);
	SaveImage(_texture);
}

void UUI_Main::LoadTexture()
{
	//1. SyncLoadTexture : 동기 로딩, 로딩이 끝나기를 기다린다.
	//SyncLoadTexure(FullFIlePath);

	//2. ASync Load Texture : 로딩은 끝나면 이벤트를 보낸다.
	mImageLoader = UImageLoader::LoadImageFromDiskAsync(this, FullFIlePath);
	mImageLoader->OnLoadCompleted().AddDynamic(this, &UUI_Main::OnLoadTexture);
}

void UUI_Main::SaveImage(UTexture2DDynamic* Texture)
{
	UCanvas* Canvas = nullptr;
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UTextureRenderTarget2D* RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D( GetWorld(), Texture->SizeX, Texture->SizeY, ETextureRenderTargetFormat::RTF_RGBA8_SRGB);

	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget( GetWorld(), RenderTarget, Canvas, Size, Context);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), RenderTarget);
	Canvas->K2_DrawTexture(Cast<UTexture>(Texture), FVector2D::ZeroVector, Size, FVector2D::ZeroVector);
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget( GetWorld(), Context);

	UKismetRenderingLibrary::ExportRenderTarget(GetWorld(), RenderTarget, FPaths::ProjectDir(), "Download\\test.png");
}
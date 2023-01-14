// Fill out your copyright notice in the Description page of Project Settings.


#include "MemoryTexture.h"

UMemoryTexture::UMemoryTexture() {
}

UMemoryTexture::~UMemoryTexture()
{
	delete[](Data);
}

UTexture2D* UMemoryTexture::GetTexture()
{
	return Texture;
}

void UMemoryTexture::Fill(const FColor& Color, const int X, const int Y, const int SizeX, const int SizeY)
{
	int EndX = SizeX == 0 ? Width - X : SizeX;
	int EndY = SizeY == 0 ? Height - Y : SizeY;

	for (int x = X; x < EndX; x++) {
		for (int y = Y; y < EndY; y++) {

			int p = ((x * Height) + y) * 4;
			Data[p] = Color.R;
			Data[p + 1] = Color.G;
			Data[p + 2] = Color.B;
			Data[p + 3] = Color.A;
		}
	}
}

void UMemoryTexture::Apply() {
	Mip.BulkData.Lock(LOCK_READ_WRITE);

	uint8* TextureData = (uint8*)Mip.BulkData.Realloc(Width * Height * 4);
	FMemory::Memcpy(TextureData, Data, sizeof(uint8) * Width * Height * 4);

	Mip.BulkData.Unlock();
	Texture->GetPlatformData()->Mips[0] = Mip;

	Texture->UpdateResource();
}

void UMemoryTexture::Init(const int TWidth, const int THeight)
{
	Width = TWidth;
	Height = THeight;
	Name = "Mem Texture";
	Data = new uint8[Width * Height * 4];
	Texture = UTexture2D::CreateTransient(Width, Height, EPixelFormat::PF_R8G8B8A8, Name);
	Mip = Texture->GetPlatformData()->Mips[0];
}

void UMemoryTexture::SetPixel(const FColor& Color, int X, int Y) {
	if (X >= Width || Y >= Height) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Texture coordinate: (%d,%d) texture size:(%d,%d)"), X, Y, Width, Height);
		return;
	}

	uint32 i = ((X * Height) + Y) * 4;
	Data[i] = Color.R;
	Data[i + 1] = Color.G;
	Data[i + 2] = Color.B;
	Data[i + 3] = Color.A;
}

void UMemoryTexture::SetRawPixels(uint8* RawData)
{
	for (int i = 0; i < (Width * Height * 4); i++) {
		Data[i] = RawData[i];
	}
}

int UMemoryTexture::GetWidth() const
{
	return Width;
}

int UMemoryTexture::GetHeight() const
{
	return Height;
}




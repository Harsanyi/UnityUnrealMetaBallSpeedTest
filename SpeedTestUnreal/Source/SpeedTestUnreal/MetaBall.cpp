// Fill out your copyright notice in the Description page of Project Settings.


#include "MetaBall.h"

void UMetaBall::Init(const uint32& nX, const uint32& nY, const float& nR)
{
	X = nX;
	Y = nY;
	R = nR;
}

float UMetaBall::GetInfluenceTo(const uint32& pX, const uint32& pY)
{
	if (pX == X && pY == Y) {
		return R;
	}
	else {
		return 1.f / (FMath::Pow(((float)X - pX) / R, 2) + FMath::Pow(((float)Y - pY) / R, 2));
	}
}

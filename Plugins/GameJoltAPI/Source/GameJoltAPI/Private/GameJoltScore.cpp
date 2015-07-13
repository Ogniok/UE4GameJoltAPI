/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#include "GameJoltAPIPrivatePCH.h"
/********************************************************************************************************/
FGameJoltScore::FGameJoltScore()
{
	Reset();
}
FGameJoltScore::FGameJoltScore(TMap<FString, FString> ScoreData)
{
	LoadData(ScoreData);
}
/********************************************************************************************************/
void FGameJoltScore::Reset()
{
	Score = "";
	Sort = 0;
	ExtraData = "";
	UserName = "";
	UserID = -1;
	GuestName = "";
	Stored = "";
}
/********************************************************************************************************/
void FGameJoltScore::LoadData(TMap<FString, FString> ScoreData)
{
	Reset();

	if(FString* ScoreValue = ScoreData.Find("score"))
	{
		Score = *ScoreValue;
	}

	if(FString* ScoreSort = ScoreData.Find("sort"))
	{
		Sort = FCString::Atoi(*(*ScoreSort));
	}

	if(FString* ScoreExtraData = ScoreData.Find("extra_data"))
	{
		ExtraData = *ScoreExtraData;
	}

	if(FString* ScoreUserName = ScoreData.Find("user"))
	{
		UserName = *ScoreUserName;
	}

	if(FString* ScoreUserID = ScoreData.Find("user_id"))
	{
		UserID = FCString::Atoi(*(*ScoreUserID));
	}

	if(FString* ScoreGuestName = ScoreData.Find("guest"))
	{
		GuestName = *ScoreGuestName;
	}

	if(FString* ScoreStored = ScoreData.Find("stored"))
	{
		Stored = *ScoreStored;
	}
}
/********************************************************************************************************/
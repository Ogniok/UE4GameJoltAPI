/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#pragma once
/********************************************************************************************************/
struct FGameJoltScore
{
	FString Score;
	int32 Sort;
	FString ExtraData;
	FString UserName;
	int32 UserID;
	FString GuestName;
	FString Stored;

	/* Clears stored data */
	void Reset();

	/* Returns proper name (user name or guest name depending on what is valid) */
	inline FString GetName() { return UserName == "" ? GuestName : UserName; }

	/* Loads score data from given map */
	void LoadData(TMap<FString, FString> ScoreData);

	FGameJoltScore();
	FGameJoltScore(TMap<FString, FString> ScoreData);
};
/********************************************************************************************************/
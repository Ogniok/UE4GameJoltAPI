/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#pragma once
/********************************************************************************************************/
enum class ETrophyDifficulty : uint8
{
	DIFFICULTY_Bronze,
	DIFFICULTY_Silver,
	DIFFICULTY_Gold,
	DIFFICULTY_Platinum,
};
/********************************************************************************************************/
struct FGameJoltTrophy
{
	int32 ID;
	FString Title;
	FString Description;
	ETrophyDifficulty Difficulty;
	FString ImageURL;
	FString Achieved;

	FString DifficultyString;

	/* Clears stored data */
	void Reset();

	/* Loads trophy data from given map */
	void LoadData(TMap<FString, FString> TrophyData);

	/* Returns trophy difficulty as string */
	FString GetDifficultyAsString();

	FGameJoltTrophy();
	FGameJoltTrophy(TMap<FString, FString> TrophyData);
};
/********************************************************************************************************/
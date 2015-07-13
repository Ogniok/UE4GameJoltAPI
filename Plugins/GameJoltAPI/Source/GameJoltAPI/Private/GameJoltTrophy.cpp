/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#include "GameJoltAPIPrivatePCH.h"
/********************************************************************************************************/
FGameJoltTrophy::FGameJoltTrophy()
{
	Reset();
}
FGameJoltTrophy::FGameJoltTrophy(TMap<FString, FString> TrophyData)
{
	LoadData(TrophyData);
}
/********************************************************************************************************/
void FGameJoltTrophy::Reset()
{
	ID = -1;
	Title = "";
	Description = "";
	Difficulty = ETrophyDifficulty::DIFFICULTY_Bronze;
	ImageURL = "";
	Achieved = "";

	DifficultyString = GetDifficultyAsString();
}
/********************************************************************************************************/
void FGameJoltTrophy::LoadData(TMap<FString, FString> TrophyData)
{
	Reset();

	if(FString* TrophyID = TrophyData.Find("id"))
	{
		ID = FCString::Atoi(*(*TrophyID));
	}

	if(FString* TrophyTitle = TrophyData.Find("title"))
	{
		Title = *TrophyTitle;
	}

	if(FString* TrophyDescription = TrophyData.Find("description"))
	{
		Description = *TrophyDescription;
	}

	if(FString* TrophyDifficulty = TrophyData.Find("difficulty"))
	{
		if((*TrophyDifficulty) == "Bronze") Difficulty = ETrophyDifficulty::DIFFICULTY_Bronze;
		else if((*TrophyDifficulty) == "Silver") Difficulty = ETrophyDifficulty::DIFFICULTY_Silver;
		else if((*TrophyDifficulty) == "Gold") Difficulty = ETrophyDifficulty::DIFFICULTY_Gold;
		else if((*TrophyDifficulty) == "Platinum") Difficulty = ETrophyDifficulty::DIFFICULTY_Platinum;
	}

	if(FString* TrophyImageURL = TrophyData.Find("image_url"))
	{
		ImageURL = *TrophyImageURL;
	}

	if(FString* TrophyAchieved = TrophyData.Find("achieved"))
	{
		Achieved = *TrophyAchieved;
	}

	DifficultyString = GetDifficultyAsString();
}
/********************************************************************************************************/
FString FGameJoltTrophy::GetDifficultyAsString()
{
	if(Difficulty == ETrophyDifficulty::DIFFICULTY_Bronze)
	{
		return "Bronze";
	}
	else if(Difficulty == ETrophyDifficulty::DIFFICULTY_Silver)
	{
		return "Silver";
	}
	else if(Difficulty == ETrophyDifficulty::DIFFICULTY_Gold)
	{
		return "Gold";
	}
	else if(Difficulty == ETrophyDifficulty::DIFFICULTY_Platinum)
	{
		return "Platinum";
	}

	return "";
}
/********************************************************************************************************/
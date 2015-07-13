/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#pragma once
/********************************************************************************************************/
#include "GameJoltScore.h"
/********************************************************************************************************/
struct FGameJoltScoreTable
{
	int32 ID;
	FString Name;
	FString Description;
	bool Primary;

	TArray<FGameJoltScore> Scores;

	/* Clears stored data */
	void Reset();

	/* Clears stored scores data */
	void ClearScores();

	/* Loads score table data from given map */
	void LoadData(TMap<FString, FString> TableData);

	/* Adds score to table */
	void AddScore(TMap<FString, FString> ScoreData);

	FGameJoltScoreTable();
	FGameJoltScoreTable(TMap<FString, FString> TableData);
};
/********************************************************************************************************/
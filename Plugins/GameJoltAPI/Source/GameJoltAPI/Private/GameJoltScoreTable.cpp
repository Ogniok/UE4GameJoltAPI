/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#include "GameJoltAPIPrivatePCH.h"
/********************************************************************************************************/
FGameJoltScoreTable::FGameJoltScoreTable()
{
	Reset();
	ClearScores();
}
FGameJoltScoreTable::FGameJoltScoreTable(TMap<FString, FString> TableData)
{
	ClearScores();
	LoadData(TableData);
}
/********************************************************************************************************/
void FGameJoltScoreTable::Reset()
{
	ID = -1;
	Name = "";
	Description = "";
	Primary = false;
}
void FGameJoltScoreTable::ClearScores()
{
	Scores.Empty();
}
/********************************************************************************************************/
void FGameJoltScoreTable::LoadData(TMap<FString, FString> TableData)
{
	Reset();

	if(FString* TableID = TableData.Find("id"))
	{
		ID = FCString::Atoi(*(*TableID));
	}

	if(FString* TableName = TableData.Find("name"))
	{
		Name = *TableName;
	}

	if(FString* TableDescription = TableData.Find("description"))
	{
		Description = *TableDescription;
	}

	if(FString* TablePrimary = TableData.Find("primary"))
	{
		Primary = (*TablePrimary) == "1";
	}
}
void FGameJoltScoreTable::AddScore(TMap<FString, FString> ScoreData)
{
	Scores.Add(FGameJoltScore(ScoreData));
}
/********************************************************************************************************/
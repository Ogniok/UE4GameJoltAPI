/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#include "GameJoltAPIPrivatePCH.h"
/********************************************************************************************************/
FGameJoltRequestData::FGameJoltRequestData()
{
	Reset();
}
FGameJoltRequestData::FGameJoltRequestData(ERequest RequestType) : FGameJoltRequestData()
{
	this->RequestType = RequestType;
}
FGameJoltRequestData::FGameJoltRequestData(ERequest RequestType, int32 TableID) : FGameJoltRequestData(RequestType)
{
	this->TableID = TableID;
}
/********************************************************************************************************/
void FGameJoltRequestData::Reset()
{
	RequestType = ERequest::REQUEST_None;

	TableID = -1;

	Key = "";
	UserStorage = false;
}	
/********************************************************************************************************/
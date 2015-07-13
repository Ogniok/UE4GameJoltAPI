/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#pragma once
/********************************************************************************************************/
enum class EGameJoltUserType : uint8
{
	User,
	Developer,
};
/********************************************************************************************************/
struct FGameJoltUser
{
	int32 ID;
	EGameJoltUserType Type;
	FString UserName;
	FString AvatarURL;
	FString SignedUp;
	FString LastLoggedIn;
	FString Status;

	FString DeveloperName;
	FString DeveloperWebsite;
	FString DeveloperDescription;

	/* Clears stored data */
	void Reset();

	/* Loads user data from given map */
	void LoadData(TMap<FString, FString> UserData);

	FGameJoltUser();
	FGameJoltUser(TMap<FString, FString> UserData);
};
/********************************************************************************************************/
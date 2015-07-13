/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#include "GameJoltAPIPrivatePCH.h"
/********************************************************************************************************/
FGameJoltUser::FGameJoltUser()
{
	Reset();
}
FGameJoltUser::FGameJoltUser(TMap<FString, FString> UserData)
{
	LoadData(UserData);
}
/********************************************************************************************************/
void FGameJoltUser::Reset()
{
	ID = -1;
	Type = EGameJoltUserType::User;
	UserName = "";
	AvatarURL = "";
	SignedUp = "";
	LastLoggedIn = "";
	Status = "";

	DeveloperName = "";
	DeveloperWebsite = "";
	DeveloperDescription = "";
}
/********************************************************************************************************/
void FGameJoltUser::LoadData(TMap<FString, FString> UserData)
{
	Reset();

	if(FString* UserID = UserData.Find("id"))
	{
		ID = FCString::Atoi(*(*UserID));
	}

	if(FString* UserType = UserData.Find("type"))
	{
		Type = (*UserType) == "User" ? EGameJoltUserType::User : EGameJoltUserType::Developer;
	}
	
	if(FString* UsersName = UserData.Find("username"))
	{
		UserName = (*UsersName);
	}
	
	if(FString* UserAvatarURL = UserData.Find("avatar_url"))
	{
		AvatarURL = (*UserAvatarURL);
	}

	if(FString* UserSignedUp = UserData.Find("signed_up"))
	{
		SignedUp = (*UserSignedUp);
	}

	if(FString* UserLastLoggedIn = UserData.Find("last_logged_in"))
	{
		LastLoggedIn = (*UserLastLoggedIn);
	}

	if(FString* UserStatus = UserData.Find("status"))
	{
		Status = (*UserStatus);
	}

	if(FString* UserDeveloperName = UserData.Find("developer_name"))
	{
		DeveloperName = (*UserDeveloperName);
	}

	if(FString* UserDeveloperWebsite = UserData.Find("developer_website"))
	{
		DeveloperWebsite = (*UserDeveloperWebsite);
	}

	if(FString* UserDeveloperDescription = UserData.Find("developer_description"))
	{
		DeveloperDescription = (*UserDeveloperDescription);
	}
}
/********************************************************************************************************/
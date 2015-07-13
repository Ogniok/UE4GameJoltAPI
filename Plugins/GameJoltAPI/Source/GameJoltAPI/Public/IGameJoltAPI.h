/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#pragma once
/********************************************************************************************************/
#include "ModuleManager.h"
/********************************************************************************************************/
#include "GameJoltTrophy.h"
#include "GameJoltScoreTable.h"
#include "GameJoltScore.h"
#include "GameJoltUser.h"
/********************************************************************************************************/
DECLARE_LOG_CATEGORY_EXTERN(GameJoltAPI, Log, All);
/********************************************************************************************************/
enum class ERequest : uint8
{
	REQUEST_None,
	REQUEST_Auth,
	REQUEST_UserData,
	REQUEST_SessionStart,
	REQUEST_SessionPing,
	REQUEST_SessionClose,
	REQUEST_AchievedTrophy,
	REQUEST_FetchTrophy,
	REQUEST_AddScore,
	REQUEST_FetchScoreTable,
	REQUEST_FetchScore,
	REQUEST_SetData,
	REQUEST_UpdateData,
	REQUEST_RemoveData,
	REQUEST_FetchData,
	REQUEST_GetDataKeys,
};
enum class EAuthenticationStatus : uint8
{
	STATUS_None,
	STATUS_InProgress,
	STATUS_Authenticated,
	STATUS_Failed,
	STATUS_Error,
	STATUS_Guest,
};
enum class EUserSessionState : uint8
{
	STATE_Active,
	STATE_Idle,
};
enum class EDataOperation : uint8
{
	OPERATION_Add,
	OPERATION_Subtract,
	OPERATION_Multiply,
	OPERATION_Divide,
	OPERATION_Append,
	OPERATION_Prepend,
};
/********************************************************************************************************/
DECLARE_DELEGATE_OneParam(FUserDataFetchedDelegate, FGameJoltUser); //Delegate called when requested user data is fetched :: (First Param - Fetched user data)
DECLARE_DELEGATE(FTrophiesCacheUpdatedDelegate); //Delegate called when trophies cache gets updated
DECLARE_DELEGATE(FScoreTablesCacheUpdatedDelegate); //Delegate called when score tables cache gets updated (either with table data or scores data)
DECLARE_DELEGATE_ThreeParams(FDataFetchedDelegate, FString, FString, bool); //Delegate called when requested data is fetched from the data storage :: (First Param - Fetched data) (Second Param - Fetched data's key) (Third Param - True if the data was taken from user's storage)
DECLARE_DELEGATE_OneParam(FKeysFetchedDelegate, TArray<FString>&); //Delegate called when keys array from data storage is fetched :: (First Param - Array of fetched keys)
/********************************************************************************************************/
class IGameJoltAPI : public IModuleInterface
{

public:

	/**********/
	/* Module */
	/**********/

	static inline IGameJoltAPI& Get()
	{
		return FModuleManager::LoadModuleChecked<IGameJoltAPI>("GameJoltAPI");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("GameJoltAPI");
	}

	/********/
	/* Main */
	/********/

	/* Initializes module with game id and it's private key */
	inline void Initialize(int32 GameID, FString GamePrivateKey) { SetGameID(GameID); SetPrivateKey(GamePrivateKey); }

	/* Handles updating the module - needs to be called every frame */
	virtual void Update(float DeltaTime) = NULL;

	/* Sets the game ID */
	inline void SetGameID(int32 GameID) { this->GameID = GameID; }

	/* Sets the game private key */
	inline void SetPrivateKey(FString PrivateKey) { GamePrivateKey = PrivateKey; }

	/********/
	/* User */
	/********/

	/* Delegate called when requested user data is fetched */
	FUserDataFetchedDelegate UserDataFetchedDelegate;

	/* Signs in user with given credidentals and starts a session if CreateSession is true */
	virtual bool LoginUser(FString UserName, FString UserToken, bool CreateSession = false) = NULL;

	/* Sets up playing as guest */
	virtual bool LoginGuest(FString UserName) = NULL;
	
	/* Getter for UserAuthenticationStatus */
	virtual inline EAuthenticationStatus GetUserAuthenticationStatus() { return UserAuthenticationStatus; }

	/* Returns true if user is authenticated */
	virtual inline bool IsUserAuthenticated() { return UserAuthenticationStatus == EAuthenticationStatus::STATUS_Authenticated; }

	/* Returns true if user is a guest */
	virtual inline bool IsGuest() { return UserAuthenticationStatus == EAuthenticationStatus::STATUS_Guest; }

	/* Fetches user's data of a user with a given UserID */
	virtual void FetchUserData(int32 UserID) = NULL;

	/* Fetches user's data of a user with a given UserName */
	virtual void FetchUserData(FString UserName) = NULL;

	/************/
	/* Sessions */
	/************/

	/* Starts session for currently logged user */
	virtual void StartSession() = NULL;

	/* Closes currently open session */
	virtual void CloseSession() = NULL;

	/* Changes current session state */
	virtual void SetSessionState(EUserSessionState NewState) = NULL;

	/************/
	/* Trophies */
	/************/

	/* Contains trophies' cached data */
	TMap<int32, FGameJoltTrophy> Trophies;

	/* Delegate called when Trophies cache gets updated */
	FTrophiesCacheUpdatedDelegate TrophiesCacheUpdatedDelegate;

	/* Sets given trophy as achieved for current user */
	virtual void AchievedTrophy(int32 TrophyID) = NULL;

	/* Updates cached data for a trophy with given ID */
	virtual void FetchTrophy(int32 TrophyID) = NULL;

	/* Updates cached data for trophies with given IDs */
	virtual void FetchTrophies(TArray<int32>& TrophiesIDs) = NULL;

	/* Updates cached data for all trophies if UseFilter is false - If UseFilter is true, then the function updates only achieved or unachieved trophies depending on AchievedOnly value */
	virtual void FetchTrophies(bool UseFilter = false, bool AchievedOnly = true) = NULL;

	/* Returns an array of all cached trophies */
	virtual TArray<FGameJoltTrophy> GetTrophies() = NULL;

	/**********/
	/* Scores */
	/**********/

	/* Contains score tables' cached data */
	TMap<int32, FGameJoltScoreTable> ScoreTables;

	/* Delegate called when score tables cache gets updated */
	FScoreTablesCacheUpdatedDelegate ScoreTablesCacheUpdatedDelegate;

	/* Adds the given score achieved by user or guest to the highscore table with given ID and includes ExtraData if specified - If TableID is lower then zero then the score is added to the primary highscore table */
	virtual void AddScore(FString Score, int32 Sort, int32 TableID = -1, FString ExtraData = "") = NULL;
	
	/* Updates cached data for score tables (only table information, not the actual scores) */
	virtual void FetchScoreTables() = NULL;

	/* Fetches all scores from a given table scored by a current user - if TableID is less than zero then primary is used */
	virtual void FetchUserScores(int32 TableID = -1, int32 Limit = 10) = NULL;

	/* Fetches all scores from a given table - if TableID is less than zero then primary is used */
	virtual void FetchScores(int32 TableID = -1, int32 Limit = 10) = NULL;

	/* Returns an array of all cached score tables */
	virtual TArray<FGameJoltScoreTable> GetScoreTables() = NULL;

	/* Returns the ID of the primary score table or -1 if none was found - requires that score tables were fetched at least once */
	virtual int32 GetPrimaryScoreTableID();

	/* Returns the pointer to the primary score table - nullptr if not found */
	virtual inline FGameJoltScoreTable* GetPrimaryScoreTable() { return ScoreTables.Find(GetPrimaryScoreTableID()); }

	/****************/
	/* Data Storage */
	/****************/

	/* Delegate called when requested data is fetched */
	FDataFetchedDelegate DataFetchedDelegate;

	/* Delegate called when requested keys are fetched */
	FKeysFetchedDelegate KeysFetchedDelegate;

	/* Fetches data associated with Key either from global or user storage depending on the value of UserStorage variable */
	virtual void GetData(FString Key, bool UserStorage = false) = NULL;

	/* Sets data in the Data Storage - if UserStorage is true then the data is stored in the current user's storage */
	virtual void SetData(FString Key, FString Data, bool UserStorage = false) = NULL;

	/* Updates data in the Data Storage - if UserStorage is true then the data updated is searched for in the current user's storage */
	virtual void UpdateData(FString Key, EDataOperation Operation, FString Value, bool UserStorage = false) = NULL;

	/* Removes data in the Data Storage - if UserStorage is true then the data is removed from the current user's storage */
	virtual void RemoveData(FString Key, bool UserStorage = false) = NULL;

	/* Fetches all keys either in the game's global data store or in the current user's data store (depending on UserStorage value) */
	virtual void FetchKeys(bool UserStorage = false) = NULL;

	/********************/
	/* Helper functions */
	/********************/

	/* Getter for error message */
	virtual FString GetErrorMessage() { return ErrorMessage; }

	/* Returns request type as string */
	FString RequestToString(ERequest Value);

protected:

	int32 GameID;
	FString GamePrivateKey;

	EAuthenticationStatus UserAuthenticationStatus;

	FString ErrorMessage;
};
/********************************************************************************************************/
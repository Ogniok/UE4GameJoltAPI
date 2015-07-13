/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#include "GameJoltAPIPrivatePCH.h"
/********************************************************************************************************/
#include "Runtime/Core/Public/Misc/SecureHash.h"
/********************************************************************************************************/
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"
/********************************************************************************************************/
DEFINE_LOG_CATEGORY(GameJoltAPI);
/********************************************************************************************************/
class FGameJoltAPI : public IGameJoltAPI
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:

	virtual void Update(float DeltaTime) override;

	virtual bool LoginUser(FString UserName, FString UserToken, bool CreateSession = false) override;
	virtual bool LoginGuest(FString UserName) override;

	/* Performs user authentication */
	virtual void AuthUser();

	virtual void FetchUserData(int32 UserID) override;
	virtual void FetchUserData(FString UserName) override;

	virtual void StartSession() override;
	virtual void CloseSession() override;
	
	/* Pings session to keep it alive */
	virtual void PingSession();

	virtual void SetSessionState(EUserSessionState NewState) override;

	virtual void AchievedTrophy(int32 TrophyID) override;

	virtual void FetchTrophy(int32 TrophyID) override;
	virtual void FetchTrophies(TArray<int32>& TrophiesIDs) override;
	virtual void FetchTrophies(bool UseFilter = false, bool AchievedOnly = true) override;

	virtual TArray<FGameJoltTrophy> GetTrophies() override;

	virtual void AddScore(FString Score, int32 Sort, int32 TableID = -1, FString ExtraData = "") override;

	virtual void FetchScoreTables() override;

	virtual void FetchUserScores(int32 TableID = -1, int32 Limit = 10) override;
	virtual void FetchScores(int32 TableID = -1, int32 Limit = 10) override;

	virtual TArray<FGameJoltScoreTable> GetScoreTables() override;

	virtual void GetData(FString Key, bool UserStorage = false) override;

	virtual void SetData(FString Key, FString Data, bool UserStorage = false) override;
	virtual void UpdateData(FString Key, EDataOperation Operation, FString Value, bool UserStorage = false) override;
	virtual void RemoveData(FString Key, bool UserStorage = false) override;

	virtual void FetchKeys(bool UserStorage = false) override;

protected:

	virtual inline bool IsUserValid() { return UserName != "" && UserToken != ""; }

	virtual FGameJoltRequestData* ProcessCall(FString Call, ERequest RequestType);
	virtual FString AddSignatureToCall(FString Call);

	virtual FGameJoltRequestData* PerformHTTPRequest(FString URL, ERequest RequestType);

	void RequestCompletedCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool Success);

	virtual ERequest GetRequestTypeFromPtr(FHttpRequestPtr Request);

	static TMap<FString, FString> ParseResponseContent(FString ResponseContent);
	static TMap<FString, FString> ParseResponseContent(TArray<FString>& Pairs);
	static TMap<FString, FString> ParseResponseContent(TArray<FString>& Pairs, int32 StartIndex, int32 EndIndex);

	static FString GetGameJoltOperationString(EDataOperation Operation);

private:

	TMap<FHttpRequestPtr, FGameJoltRequestData> RequestsData;

	FString UserName;
	FString UserToken;

	bool IsSessionOpen;
	float TimeUntilNextSessionPing;

	EUserSessionState UserSessionState;
};
/********************************************************************************************************/
IMPLEMENT_MODULE(FGameJoltAPI, GameJoltAPI)
/********************************************************************************************************/
void FGameJoltAPI::StartupModule()
{
	GamePrivateKey = "";

	RequestsData.Empty();

	UserName = "";
	UserToken = "";

	UserAuthenticationStatus = EAuthenticationStatus::STATUS_None;

	IsSessionOpen = false;
	TimeUntilNextSessionPing = 0.0f;

	UserSessionState = EUserSessionState::STATE_Active;

	ErrorMessage = "";

	Trophies.Empty();
	ScoreTables.Empty();
}
void FGameJoltAPI::ShutdownModule()
{
	GamePrivateKey = "";

	UserName = "";
	UserToken = "";
}
/********************************************************************************************************/
void FGameJoltAPI::Update(float DeltaTime)
{
	if((TimeUntilNextSessionPing -= DeltaTime) <= 0.0f)
	{
		//Ping session if active
		PingSession();

		TimeUntilNextSessionPing = 30.0f;
	}
}
/********************************************************************************************************/
bool FGameJoltAPI::LoginUser(FString UserName, FString UserToken, bool CreateSession)
{
	if(UserName == "" || UserToken == "") return false;

	this->UserName = UserName;
	this->UserToken = UserToken;

	UserAuthenticationStatus = EAuthenticationStatus::STATUS_None;

	AuthUser();

	if(CreateSession) StartSession();

	return true;
}
bool FGameJoltAPI::LoginGuest(FString UserName)
{
	if(UserName == "") return false;

	this->UserName = UserName;
	this->UserToken = "";

	UserAuthenticationStatus = EAuthenticationStatus::STATUS_Guest;

	return true;
}
/********************************************************************************************************/
void FGameJoltAPI::AuthUser()
{
	if(IsUserValid())
	{
		UserAuthenticationStatus = EAuthenticationStatus::STATUS_InProgress;

		ProcessCall(GAME_JOLT_API_URL + FString("v1/users/auth/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken, ERequest::REQUEST_Auth);
	}
}
/********************************************************************************************************/
void FGameJoltAPI::FetchUserData(int32 UserID)
{
	ProcessCall(GAME_JOLT_API_URL + FString("v1/users/?game_id=") + FString::FromInt(GameID) + FString("&user_id=") + FString::FromInt(UserID), ERequest::REQUEST_UserData);
}
void FGameJoltAPI::FetchUserData(FString UserName)
{
	ProcessCall(GAME_JOLT_API_URL + FString("v1/users/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName, ERequest::REQUEST_UserData);
}
/********************************************************************************************************/
void FGameJoltAPI::StartSession()
{
	if(IsUserValid())
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/sessions/open/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken, ERequest::REQUEST_SessionStart);

		UserSessionState = EUserSessionState::STATE_Active;
	}
}
void FGameJoltAPI::CloseSession()
{
	if(IsUserValid())
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/sessions/close/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken, ERequest::REQUEST_SessionClose);

		IsSessionOpen = false;
	}
}
/********************************************************************************************************/
void FGameJoltAPI::PingSession()
{
	if(IsSessionOpen && IsUserValid())
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/sessions/ping/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&status="), ERequest::REQUEST_SessionPing);
	}
}
/********************************************************************************************************/
void FGameJoltAPI::SetSessionState(EUserSessionState NewState)
{
	if(UserSessionState != NewState)
	{
		UserSessionState = NewState;

		if(IsSessionOpen && IsUserValid())
		{
			ProcessCall(GAME_JOLT_API_URL + FString("v1/sessions/ping/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&status=") + (UserSessionState == EUserSessionState::STATE_Active ? "active" : "idle"), ERequest::REQUEST_SessionPing);
		}
	}
}
/********************************************************************************************************/
void FGameJoltAPI::AchievedTrophy(int32 TrophyID)
{
	if(IsUserValid())
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/trophies/add-achieved/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&trophy_id=") + FString::FromInt(TrophyID), ERequest::REQUEST_AchievedTrophy);
	}
}
/********************************************************************************************************/
void FGameJoltAPI::FetchTrophy(int32 TrophyID)
{
	if(IsUserValid())
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/trophies/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&trophy_id=") + FString::FromInt(TrophyID), ERequest::REQUEST_FetchTrophy);
	}
}
void FGameJoltAPI::FetchTrophies(TArray<int32>& TrophiesIDs)
{
	if(TrophiesIDs.Num() == 1)
	{
		FetchTrophy(TrophiesIDs[0]);
	}
	else if(TrophiesIDs.Num() > 1)
	{
		if(IsUserValid())
		{
			FString TrophyIDString = "";

			for(int32 i = 0; i < TrophiesIDs.Num(); i++) TrophyIDString.Append(FString::FromInt(TrophiesIDs[i]) + (i != TrophiesIDs.Num() - 1 ? "," : ""));

			ProcessCall(GAME_JOLT_API_URL + FString("v1/trophies/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&trophy_id=") + TrophyIDString, ERequest::REQUEST_FetchTrophy);
		}
	}
}
void FGameJoltAPI::FetchTrophies(bool UseFilter, bool AchievedOnly)
{
	if(IsUserValid())
	{
		if(UseFilter)
		{
			ProcessCall(GAME_JOLT_API_URL + FString("v1/trophies/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&achieved=") + (AchievedOnly ? "true" : "false"), ERequest::REQUEST_FetchTrophy);
		}
		else
		{
			ProcessCall(GAME_JOLT_API_URL + FString("v1/trophies/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken, ERequest::REQUEST_FetchTrophy);
		}
	}
}
/********************************************************************************************************/
TArray<FGameJoltTrophy> FGameJoltAPI::GetTrophies()
{
	TArray<FGameJoltTrophy> Result;

	Trophies.GenerateValueArray(Result);

	return Result;
}
/********************************************************************************************************/
void FGameJoltAPI::AddScore(FString Score, int32 Sort, int32 TableID, FString ExtraData)
{
	if(IsUserValid())
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/scores/add/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&score=") + Score + FString("&sort=") + FString::FromInt(Sort) + FString("&guest=") + (TableID >= 0 ? (FString("&table_id=") + FString::FromInt(TableID)) : "") + (ExtraData != "" ? (FString("&extra_data=") + ExtraData) : ""), ERequest::REQUEST_AddScore);
	}
	else if(IsGuest())
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/scores/add/?game_id=") + FString::FromInt(GameID) + FString("&username=") + FString("&user_token=") + FString("&score=") + Score + FString("&sort=") + Sort + FString("&guest=") + UserName + (TableID >= 0 ? (FString("&table_id=") + FString::FromInt(TableID)) : "") + (ExtraData != "" ? (FString("&extra_data=") + ExtraData) : ""), ERequest::REQUEST_AddScore);
	}
}
/********************************************************************************************************/
void FGameJoltAPI::FetchScoreTables()
{
	ProcessCall(GAME_JOLT_API_URL + FString("v1/scores/tables/?game_id=") + FString::FromInt(GameID), ERequest::REQUEST_FetchScoreTable);
}
/********************************************************************************************************/
void FGameJoltAPI::FetchUserScores(int32 TableID, int32 Limit)
{
	if(IsUserValid())
	{
		if(FGameJoltRequestData* Request = ProcessCall(GAME_JOLT_API_URL + FString("v1/scores/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&limit=") + FString::FromInt(Limit) + (TableID >= 0 ? (FString("&table_id=") + FString::FromInt(TableID)) : ""), ERequest::REQUEST_FetchScore))
		{
			Request->TableID = TableID >= 0 ? TableID : GetPrimaryScoreTableID();
		}
	}
}
void FGameJoltAPI::FetchScores(int32 TableID, int32 Limit)
{
	if(FGameJoltRequestData* Request = ProcessCall(GAME_JOLT_API_URL + FString("v1/scores/?game_id=") + FString::FromInt(GameID) + FString("&limit=") + FString::FromInt(Limit) + (TableID >= 0 ? (FString("&table_id=") + FString::FromInt(TableID)) : ""), ERequest::REQUEST_FetchScore))
	{
		Request->TableID = TableID >= 0 ? TableID : GetPrimaryScoreTableID();
	}
}
/********************************************************************************************************/
TArray<FGameJoltScoreTable> FGameJoltAPI::GetScoreTables()
{
	TArray<FGameJoltScoreTable> Result;

	ScoreTables.GenerateValueArray(Result);

	return Result;
}
/********************************************************************************************************/
void FGameJoltAPI::GetData(FString Key, bool UserStorage)
{
	FGameJoltRequestData* Request = nullptr;

	if(UserStorage)
	{
		if(IsUserValid())
		{
			Request = ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&key=") + Key + FString("&format=dump"), ERequest::REQUEST_FetchData);
		}
	}
	else
	{
		Request = ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/?game_id=") + FString::FromInt(GameID) + FString("&key=") + Key + FString("&format=dump"), ERequest::REQUEST_FetchData);
	}

	if(Request)
	{
		Request->Key = Key;
		Request->UserStorage = UserStorage;
	}
}
/********************************************************************************************************/
void FGameJoltAPI::SetData(FString Key, FString Data, bool UserStorage)
{
	if(UserStorage)
	{
		if(IsUserValid())
		{
			ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/set/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&key=") + Key + FString("&data=") + Data, ERequest::REQUEST_SetData);
		}
	}
	else
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/set/?game_id=") + FString::FromInt(GameID) + FString("&key=") + Key + FString("&data=") + Data, ERequest::REQUEST_SetData);
	}
}
void FGameJoltAPI::UpdateData(FString Key, EDataOperation Operation, FString Value, bool UserStorage)
{
	if(UserStorage)
	{
		if(IsUserValid())
		{
			ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/update/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&key=") + Key + FString("&operation=") + GetGameJoltOperationString(Operation) + FString("&value=") + Value, ERequest::REQUEST_UpdateData);
		}
	}
	else
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/update/?game_id=") + FString::FromInt(GameID) + FString("&key=") + Key + FString("&operation=") + GetGameJoltOperationString(Operation) + FString("&value=") + Value, ERequest::REQUEST_UpdateData);
	}
}
void FGameJoltAPI::RemoveData(FString Key, bool UserStorage)
{
	if(UserStorage)
	{
		if(IsUserValid())
		{
			ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/remove/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&key=") + Key, ERequest::REQUEST_RemoveData);
		}
	}
	else
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/remove/?game_id=") + FString::FromInt(GameID) + FString("&key=") + Key, ERequest::REQUEST_RemoveData);
	}
}
/********************************************************************************************************/
void FGameJoltAPI::FetchKeys(bool UserStorage)
{
	if(UserStorage)
	{
		if(IsUserValid())
		{
			ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/remove/?game_id=") + FString::FromInt(GameID) + FString("&username=") + UserName + FString("&user_token=") + UserToken + FString("&format=dump"), ERequest::REQUEST_GetDataKeys);
		}
	}
	else
	{
		ProcessCall(GAME_JOLT_API_URL + FString("v1/data-store/get-keys/?game_id=") + FString::FromInt(GameID), ERequest::REQUEST_GetDataKeys);
	}
}
/********************************************************************************************************/
FGameJoltRequestData* FGameJoltAPI::ProcessCall(FString Call, ERequest RequestType)
{
	if(Call != "")
	{
		return PerformHTTPRequest(AddSignatureToCall(Call), RequestType);
	}

	return nullptr;
}
/********************************************************************************************************/
FString FGameJoltAPI::AddSignatureToCall(FString Call)
{
	if(Call != "")
	{
		return Call + FString("&signature=") + FMD5::HashAnsiString(*(Call + GamePrivateKey));
	}

	return "";
}
/********************************************************************************************************/
FGameJoltRequestData* FGameJoltAPI::PerformHTTPRequest(FString URL, ERequest RequestType)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	RequestsData.Add(Request, FGameJoltRequestData(RequestType));
	
	//Encode spaces
	URL = URL.Replace(TEXT(" "), TEXT("%20"), ESearchCase::IgnoreCase);

	UE_LOG(GameJoltAPI, Error, TEXT("(URL) %s"), *URL);

	//Set URL
	Request->SetURL(URL);

	//Link proper delegate
	Request->OnProcessRequestComplete().BindRaw(this, &FGameJoltAPI::RequestCompletedCallback);

	//Process request
	Request->ProcessRequest();

	return RequestsData.Find(Request);
}
/********************************************************************************************************/
void FGameJoltAPI::RequestCompletedCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool Success)
{
	FGameJoltRequestData* RequestData = RequestsData.Find(Request);
	ERequest RequestType = GetRequestTypeFromPtr(Request);
	
	UE_LOG(GameJoltAPI, Error, TEXT("(Request %s response) %s"), *RequestToString(RequestType), *Response->GetContentAsString());

	if(Success)
	{
		if(RequestType == ERequest::REQUEST_Auth)
		{
			TMap<FString, FString> ResponseData = ParseResponseContent(Response->GetContentAsString());
			bool RequestSucceeded = false;

			if(FString* ReturnedValue = ResponseData.Find("success"))
			{
				RequestSucceeded = (*ReturnedValue) == "true";
			}

			if(RequestSucceeded)
			{
				UserAuthenticationStatus = EAuthenticationStatus::STATUS_Authenticated;
			}
			else
			{
				UserAuthenticationStatus = EAuthenticationStatus::STATUS_Failed;

				UserName = "";
				UserToken = "";

				if(FString* ReturnedValue = ResponseData.Find("message"))
				{
					ErrorMessage = *ReturnedValue;
				}

				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ErrorMessage);
			}
		}
		else if(RequestType == ERequest::REQUEST_UserData)
		{
			TMap<FString, FString> ResponseData = ParseResponseContent(Response->GetContentAsString());
			bool RequestSucceeded = false;

			if(FString* ReturnedValue = ResponseData.Find("success"))
			{
				RequestSucceeded = (*ReturnedValue) == "true";
			}

			if(RequestSucceeded)
			{
				UserDataFetchedDelegate.ExecuteIfBound(FGameJoltUser(ResponseData));
			}
			else
			{
				if(FString* ReturnedValue = ResponseData.Find("message"))
				{
					ErrorMessage = *ReturnedValue;
				}

				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ErrorMessage);
			}
		}
		else if(RequestType == ERequest::REQUEST_SessionStart)
		{
			TMap<FString, FString> ResponseData = ParseResponseContent(Response->GetContentAsString());
			bool RequestSucceeded = false;

			if(FString* ReturnedValue = ResponseData.Find("success"))
			{
				RequestSucceeded = (*ReturnedValue) == "true";
			}

			if(RequestSucceeded)
			{
				IsSessionOpen = true;
			}
			else
			{
				if(FString* ReturnedValue = ResponseData.Find("message"))
				{
					ErrorMessage = *ReturnedValue;
				}

				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ErrorMessage);
			}
		}
		else if(RequestType == ERequest::REQUEST_AchievedTrophy)
		{
			if(!Response->GetContentAsString().StartsWith("success:\"true\""))
			{
				TMap<FString, FString> ResponseData = ParseResponseContent(Response->GetContentAsString());

				if(FString* ReturnedValue = ResponseData.Find("message"))
				{
					ErrorMessage = *ReturnedValue;
				}

				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ErrorMessage);
			}
		}
		else if(RequestType == ERequest::REQUEST_FetchTrophy)
		{
			TArray<FString> ResponseLines;
			bool RequestSucceeded = false;

			Response->GetContentAsString().ParseIntoArrayLines(ResponseLines);

			RequestSucceeded = ResponseLines[0] == "success:\"true\"";
			ResponseLines.RemoveAt(0);

			if(RequestSucceeded)
			{
				int32 TrophiesReceivedAmount = ResponseLines.Num() / 6;

				for(int32 i = 0; i < TrophiesReceivedAmount; i++)
				{
					TMap<FString, FString> TrophyData = ParseResponseContent(ResponseLines, i * 6, i * 6 + 5);

					if(FString* TrophyIDAsString = TrophyData.Find("id"))
					{
						if(FGameJoltTrophy* Trophy = Trophies.Find(FCString::Atoi(*(*TrophyIDAsString))))
						{
							Trophy->LoadData(TrophyData);
						}
						else
						{
							Trophies.Add(FCString::Atoi(*(*TrophyIDAsString)), FGameJoltTrophy(TrophyData));
						}
					}
				}

				//Call delegate
				TrophiesCacheUpdatedDelegate.ExecuteIfBound();
			}
			else
			{
				TMap<FString, FString> ResponseData = ParseResponseContent(Response->GetContentAsString());

				if(FString* ReturnedValue = ResponseData.Find("message"))
				{
					ErrorMessage = *ReturnedValue;
				}

				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ErrorMessage);
			}
		}
		else if(RequestType == ERequest::REQUEST_FetchScoreTable)
		{
			TArray<FString> ResponseLines;
			bool RequestSucceeded = false;

			Response->GetContentAsString().ParseIntoArrayLines(ResponseLines);

			RequestSucceeded = ResponseLines[0] == "success:\"true\"";
			ResponseLines.RemoveAt(0);

			if(RequestSucceeded)
			{
				int32 TablesReceivedAmount = ResponseLines.Num() / 4;

				for(int32 i = 0; i < TablesReceivedAmount; i++)
				{
					TMap<FString, FString> TableData = ParseResponseContent(ResponseLines, i * 4, i * 4 + 3);

					if(FString* TableIDAsString = TableData.Find("id"))
					{
						if(FGameJoltScoreTable* Table = ScoreTables.Find(FCString::Atoi(*(*TableIDAsString))))
						{
							Table->LoadData(TableData);
						}
						else
						{
							ScoreTables.Add(FCString::Atoi(*(*TableIDAsString)), FGameJoltScoreTable(TableData));
						}
					}
				}

				//Call delegate
				ScoreTablesCacheUpdatedDelegate.ExecuteIfBound();
			}
			else
			{
				TMap<FString, FString> ResponseData = ParseResponseContent(Response->GetContentAsString());

				if(FString* ReturnedValue = ResponseData.Find("message"))
				{
					ErrorMessage = *ReturnedValue;
				}

				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ErrorMessage);
			}
		}
		else if(RequestType == ERequest::REQUEST_FetchScore)
		{
			TArray<FString> ResponseLines;
			bool RequestSucceeded = false;

			Response->GetContentAsString().ParseIntoArrayLines(ResponseLines);

			RequestSucceeded = ResponseLines[0] == "success:\"true\"";
			ResponseLines.RemoveAt(0);

			if(!RequestData) ErrorMessage = "Could not find request data";

			if(RequestSucceeded && RequestData)
			{
				int32 ScoresReceivedAmount = ResponseLines.Num() / 7;

				if(FGameJoltScoreTable* Table = ScoreTables.Find(RequestData->TableID))
				{
					Table->ClearScores();

					for(int32 i = 0; i < ScoresReceivedAmount; i++)
					{
						Table->AddScore(ParseResponseContent(ResponseLines, i * 7, i * 7 + 6));
					}
				}

				//Call delegate
				ScoreTablesCacheUpdatedDelegate.ExecuteIfBound();
			}
			else
			{
				TMap<FString, FString> ResponseData = ParseResponseContent(Response->GetContentAsString());

				if(FString* ReturnedValue = ResponseData.Find("message"))
				{
					ErrorMessage = *ReturnedValue;
				}

				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ErrorMessage);
			}
		}
		else if(RequestType == ERequest::REQUEST_FetchData)
		{
			TArray<FString> ResponseLines;

			Response->GetContentAsString().ParseIntoArrayLines(ResponseLines);

			if(!RequestData) ErrorMessage = "Could not find request data";

			if(ResponseLines[0] == "SUCCESS" && RequestData)
			{
				FString FetchedData = "";

				ResponseLines.RemoveAt(0);

				for(int32 i = 0; i < ResponseLines.Num(); i++)
				{
					FetchedData.Append(ResponseLines[i]);

					if(i != ResponseLines.Num() - 1) FetchedData.Append("\n");
				}

				//Call delagate
				DataFetchedDelegate.ExecuteIfBound(FetchedData, RequestData->Key, RequestData->UserStorage);
			}
			else
			{
				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *ResponseLines[1]);
			}
		}
		else if(RequestType == ERequest::REQUEST_GetDataKeys)
		{
			TArray<FString> ResponseLines;

			Response->GetContentAsString().ParseIntoArrayLines(ResponseLines);

			if(ResponseLines[0] == "success:\"true\"")
			{
				ResponseLines.RemoveAt(0);

				for(int32 i = 0; i < ResponseLines.Num(); i++)
				{
					ResponseLines[i] = (ResponseLines[i].RightChop(5)).LeftChop(1);
				}

				//Call delagate
				KeysFetchedDelegate.ExecuteIfBound(ResponseLines);
			}
			else
			{
				UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed :: Error message: %s"), *RequestToString(RequestType), *((ResponseLines[1].RightChop(9)).LeftChop(1)));
			}
		}
	}
	else
	{
		if(RequestType == ERequest::REQUEST_SessionStart)
		{
			UserAuthenticationStatus = EAuthenticationStatus::STATUS_Error;
		}

		UE_LOG(GameJoltAPI, Error, TEXT("Request %s failed to be processed :: Returned code: %s"), *RequestToString(RequestType), *FString::FromInt(Response->GetResponseCode()));
	}

	RequestsData.Remove(Request);
}
/********************************************************************************************************/
ERequest FGameJoltAPI::GetRequestTypeFromPtr(FHttpRequestPtr Request)
{
	if(FGameJoltRequestData* RequestData = RequestsData.Find(Request))
	{
		return RequestData->RequestType;
	}

	return ERequest::REQUEST_None;
}
/********************************************************************************************************/
TMap<FString, FString> FGameJoltAPI::ParseResponseContent(FString ResponseContent)
{
	TArray<FString> Pairs;
	
	ResponseContent.ParseIntoArrayLines(Pairs);

	return ParseResponseContent(Pairs);
}
TMap<FString, FString> FGameJoltAPI::ParseResponseContent(TArray<FString>& Pairs)
{
	return ParseResponseContent(Pairs, 0, Pairs.Num() - 1);
}
TMap<FString, FString> FGameJoltAPI::ParseResponseContent(TArray<FString>& Pairs, int32 StartIndex, int32 EndIndex)
{
	TMap<FString, FString> Result;

	if(StartIndex < 0) StartIndex = 0;
	if(EndIndex > Pairs.Num() - 1) EndIndex = Pairs.Num() - 1;

	for(int32 i = StartIndex; i <= EndIndex; i++)
	{
		int32 Index = -1;

		Pairs[i].FindChar(':', Index);

		if(Index >= 0)
		{
			FString Data = Pairs[i].RightChop(Index + 1);

			if(Data[0] == '"' && Data[Data.Len() - 1] == '"')
			{
				Data.RemoveAt(0, 1, false);
				Data.RemoveAt(Data.Len() - 1, 1, false);
			}

			Result.Add(Pairs[i].Left(Index), Data);
		}
	}

	return Result;
}
/********************************************************************************************************/
FString FGameJoltAPI::GetGameJoltOperationString(EDataOperation Operation)
{
	if(Operation == EDataOperation::OPERATION_Add)
	{
		return "add";
	}
	else if(Operation == EDataOperation::OPERATION_Subtract)
	{
		return "subtract";
	}
	else if(Operation == EDataOperation::OPERATION_Multiply)
	{
		return "multiply";
	}
	else if(Operation == EDataOperation::OPERATION_Divide)
	{
		return "divide";
	}
	else if(Operation == EDataOperation::OPERATION_Append)
	{
		return "append";
	}
	else if(Operation == EDataOperation::OPERATION_Prepend)
	{
		return "prepend";
	}

	return "";
}
/********************************************************************************************************/
int32 IGameJoltAPI::GetPrimaryScoreTableID()
{
	TArray<FGameJoltScoreTable> ScoreTablesArray = GetScoreTables();

	for(int32 i = 0; i < ScoreTablesArray.Num(); i++)
	{
		if(ScoreTablesArray[i].Primary) return ScoreTablesArray[i].ID;
	}

	return -1;
}
/********************************************************************************************************/
FString IGameJoltAPI::RequestToString(ERequest Value)
{
	if(Value == ERequest::REQUEST_None)
	{
		return "REQUEST_None";
	}
	else if(Value == ERequest::REQUEST_Auth)
	{
		return "REQUEST_Auth";
	}
	else if(Value == ERequest::REQUEST_UserData)
	{
		return "REQUEST_UserData";
	}
	else if(Value == ERequest::REQUEST_SessionStart)
	{
		return "REQUEST_SessionStart";
	}
	else if(Value == ERequest::REQUEST_SessionPing)
	{
		return "REQUEST_SessionPing";
	}
	else if(Value == ERequest::REQUEST_SessionClose)
	{
		return "REQUEST_SessionClose";
	}
	else if(Value == ERequest::REQUEST_AchievedTrophy)
	{
		return "REQUEST_AchievedTrophy";
	}
	else if(Value == ERequest::REQUEST_FetchTrophy)
	{
		return "REQUEST_FetchTrophy";
	}
	else if(Value == ERequest::REQUEST_AddScore)
	{
		return "REQUEST_AddScore";
	}
	else if(Value == ERequest::REQUEST_FetchScoreTable)
	{
		return "REQUEST_FetchScoreTable";
	}
	else if(Value == ERequest::REQUEST_FetchScore)
	{
		return "REQUEST_FetchScore";
	}
	else if(Value == ERequest::REQUEST_SetData)
	{
		return "REQUEST_SetData";
	}
	else if(Value == ERequest::REQUEST_UpdateData)
	{
		return "REQUEST_UpdateData";
	}
	else if(Value == ERequest::REQUEST_RemoveData)
	{
		return "REQUEST_RemoveData";
	}
	else if(Value == ERequest::REQUEST_FetchData)
	{
		return "REQUEST_FetchData";
	}
	else if(Value == ERequest::REQUEST_GetDataKeys)
	{
		return "REQUEST_GetDataKeys";
	}

	return "(Invalid Request Type)";
}
/********************************************************************************************************/
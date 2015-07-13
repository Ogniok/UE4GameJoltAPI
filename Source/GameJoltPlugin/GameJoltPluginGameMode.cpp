/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#include "GameJoltPlugin.h"
#include "GameJoltPluginGameMode.h"
/********************************************************************************************************/
#include "../../Plugins/GameJoltAPI/Source/GameJoltAPI/Public/IGameJoltAPI.h"
/********************************************************************************************************/
AGameJoltPluginGameMode::AGameJoltPluginGameMode()
{
	//Initialize the plugin with your game ID and private key (they can be obtained through Game Jolt site)
	IGameJoltAPI::Get().Initialize(123456, "qwertyuioasdfgh1234567589");

	//Link user data fetched delegate
	IGameJoltAPI::Get().UserDataFetchedDelegate.BindUObject(this, &AGameJoltPluginGameMode::OnUserDataFetched);

	//Link data fetched and keys fetched delegates
	IGameJoltAPI::Get().DataFetchedDelegate.BindUObject(this, &AGameJoltPluginGameMode::OnDataFetched);
	IGameJoltAPI::Get().KeysFetchedDelegate.BindUObject(this, &AGameJoltPluginGameMode::OnKeysFetched);

	PrimaryActorTick.bCanEverTick = true;
}
/********************************************************************************************************/
void AGameJoltPluginGameMode::BeginPlay()
{
	Super::BeginPlay();

	//Login user that will be playing and start a session
	IGameJoltAPI::Get().LoginUser("username", "token", true);
}
void AGameJoltPluginGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//Close previously started session
	IGameJoltAPI::Get().CloseSession();
}
/********************************************************************************************************/
void AGameJoltPluginGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Update Game Jolt API plugin
	IGameJoltAPI::Get().Update(DeltaTime);

	if(APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		/*********/
		/* Users */
		/*********/

		//Fetch user data by username
		if(PlayerController->WasInputKeyJustPressed(EKeys::Q))
		{
			IGameJoltAPI::Get().FetchUserData("SomeUsername");
		}

		//Fetch user data by user ID
		if(PlayerController->WasInputKeyJustPressed(EKeys::E))
		{
			IGameJoltAPI::Get().FetchUserData(123456);
		}

		/************/
		/* Trophies */
		/************/

		//Update trophies cached data
		if(PlayerController->WasInputKeyJustPressed(EKeys::C))
		{
			IGameJoltAPI::Get().FetchTrophies();
		}

		//Achieve some trophy and updated it's cached data
		if(PlayerController->WasInputKeyJustPressed(EKeys::V))
		{
			IGameJoltAPI::Get().AchievedTrophy(34711);
			IGameJoltAPI::Get().FetchTrophy(34711);
		}

		//Update only achieved trophies cached data
		if(PlayerController->WasInputKeyJustPressed(EKeys::B))
		{
			IGameJoltAPI::Get().FetchTrophies(true, true);
		}

		//Show cached trophies data
		if(PlayerController->WasInputKeyJustPressed(EKeys::N))
		{
			TArray<FGameJoltTrophy> Trophies = IGameJoltAPI::Get().GetTrophies();

			for(int32 i = 0; i < Trophies.Num(); i++)
			{
				if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("(") + FString::FromInt(Trophies[i].ID) + ", " + Trophies[i].Title + ", " + Trophies[i].Description + ", " + Trophies[i].DifficultyString + ", " + Trophies[i].Achieved + ", " + Trophies[i].ImageURL + ")");
			}
		}

		/**********/
		/* Scores */
		/**********/

		//Add random score to primary table and then to some other table
		if(PlayerController->WasInputKeyJustPressed(EKeys::F))
		{
			int32 RandomScore = FMath::RandRange(0, 10000);

			IGameJoltAPI::Get().AddScore(FString::FromInt(RandomScore) + " Jumps", RandomScore, -1, "Test Extra Data " + FString::FromInt(RandomScore));
			IGameJoltAPI::Get().AddScore(FString::FromInt(RandomScore + 1) + " Jumps", RandomScore + 1, 81996, "Test Extra Data " + FString::FromInt(RandomScore + 1));
		}

		//Fetch scoreboards list
		if(PlayerController->WasInputKeyJustPressed(EKeys::G))
		{
			IGameJoltAPI::Get().FetchScoreTables();
		}

		//Fetch current user scores from some scoreboard
		if(PlayerController->WasInputKeyJustPressed(EKeys::H))
		{
			IGameJoltAPI::Get().FetchUserScores(81996);
		}

		//Fetch all scores from the primary scoreboard
		if(PlayerController->WasInputKeyJustPressed(EKeys::J))
		{
			IGameJoltAPI::Get().FetchScores();
		}

		//Show cached scores data
		if(PlayerController->WasInputKeyJustPressed(EKeys::K))
		{
			TArray<FGameJoltScoreTable> Scoreboards = IGameJoltAPI::Get().GetScoreTables();

			for(int32 i = 0; i < Scoreboards.Num(); i++)
			{
				if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("!!! ") + FString::FromInt(Scoreboards[i].ID) + ", " + Scoreboards[i].Name + ", " + Scoreboards[i].Description);

				for(int32 x = 0; x < Scoreboards[i].Scores.Num(); x++)
				{
					if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("(") + Scoreboards[i].Scores[x].Score + ", " + FString::FromInt(Scoreboards[i].Scores[x].Sort) + ", " + FString::FromInt(Scoreboards[i].Scores[x].UserID) + ", " + Scoreboards[i].Scores[x].GetName() + ", " + Scoreboards[i].Scores[x].ExtraData + ", " + Scoreboards[i].Scores[x].Stored + ")");
				}
			}
		}

		/****************/
		/* Data Storage */
		/****************/

		//Set some data
		if(PlayerController->WasInputKeyJustPressed(EKeys::R))
		{
			IGameJoltAPI::Get().SetData("TestKey", "Test Data");
			IGameJoltAPI::Get().SetData("TestKey2", "Test Data 2");
			IGameJoltAPI::Get().SetData("AnotherKey", "User Data", true);
		}

		//Update some data
		if(PlayerController->WasInputKeyJustPressed(EKeys::T))
		{
			IGameJoltAPI::Get().UpdateData("TestKey", EDataOperation::OPERATION_Append, " Appended Text", false);
		}

		//Remove some data
		if(PlayerController->WasInputKeyJustPressed(EKeys::Y))
		{
			IGameJoltAPI::Get().RemoveData("TestKey");
			IGameJoltAPI::Get().RemoveData("AnotherKey", true);
		}

		//Fetch some data
		if(PlayerController->WasInputKeyJustPressed(EKeys::U))
		{
			IGameJoltAPI::Get().GetData("TestKey");
		}

		//Get all keys list
		if(PlayerController->WasInputKeyJustPressed(EKeys::I))
		{
			IGameJoltAPI::Get().FetchKeys();
		}

		/************/
		/* Sessions */
		/************/

		//Change session state to active
		if(PlayerController->WasInputKeyJustPressed(EKeys::Z))
		{
			IGameJoltAPI::Get().SetSessionState(EUserSessionState::STATE_Active);
		}

		//Change session state to idle
		if(PlayerController->WasInputKeyJustPressed(EKeys::X))
		{
			IGameJoltAPI::Get().SetSessionState(EUserSessionState::STATE_Idle);
		}
	}
}
/********************************************************************************************************/
void AGameJoltPluginGameMode::OnUserDataFetched(FGameJoltUser User)
{
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("User Data (") + FString::FromInt(User.ID) + ", " + User.UserName + ", " + User.LastLoggedIn + ")");
}
/********************************************************************************************************/
void AGameJoltPluginGameMode::OnDataFetched(FString Data, FString Key, bool UserStorage)
{
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("Data Fetched (") + Data + ", " + Key + ", " + (UserStorage ? "User Storage" : "Global Storage") + ")");
}
void AGameJoltPluginGameMode::OnKeysFetched(TArray<FString>& Keys)
{
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Keys fetched");

	for(int32 i = 0; i < Keys.Num(); i++)
	{
		if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Keys[i]);
	}
}
/********************************************************************************************************/
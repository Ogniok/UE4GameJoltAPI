/********************************************************************************************************/
/* Copyright by Kacper "Ogniok" Kowalczuk */
/********************************************************************************************************/
#pragma once
/********************************************************************************************************/
#include "GameFramework/GameMode.h"
#include "GameJoltPluginGameMode.generated.h"
/********************************************************************************************************/
UCLASS()
class GAMEJOLTPLUGIN_API AGameJoltPluginGameMode : public AGameMode
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

public:

	void OnUserDataFetched(struct FGameJoltUser User);

	void OnDataFetched(FString Data, FString Key, bool UserStorage);
	void OnKeysFetched(TArray<FString>& Keys);

	AGameJoltPluginGameMode();
};
/********************************************************************************************************/
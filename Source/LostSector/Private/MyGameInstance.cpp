// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include "MainMenu.h"
#include "PauseMenu.h"

//�ʺ�ä��,�߼�ä��
const static FName SESSION_NAME = TEXT("GameSession"); //ä�θ�
const static FName SESSION_SETTINGS_KEY = TEXT("FREE");//���Ӹ��

UMyGameInstance::UMyGameInstance()
{
	ConstructorHelpers::FClassFinder<UUserWidget> MainMenuBPClass(TEXT("/Game/UI/WB_MainMenu"));
	if (MainMenuBPClass.Succeeded())
		MainMenuWidgetClass = MainMenuBPClass.Class;

	ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuBPClass(TEXT("/Game/UI/WB_PauseMenu"));
	if (PauseMenuBPClass.Succeeded())
		PauseMenuWidgetClass = PauseMenuBPClass.Class;
}
void UMyGameInstance::LoadMainMenu()
{
	if (!ensure(MainMenuWidgetClass)) return;

	MainMenu =  CreateWidget<UMainMenu>(this, MainMenuWidgetClass);
	if (!MainMenu) return;

	MainMenu->SetOwningInstance(this);
	MainMenu->StartUp();
}

void UMyGameInstance::LoadPauseMenu()
{
	if (!ensure(PauseMenuWidgetClass)) return;

	PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuWidgetClass);
	if (!PauseMenu) return;

	PauseMenu->SetOwningInstance(this);
	PauseMenu->StartUp();
}


void UMyGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Warning, 
			TEXT("OSS : %s is Avaliable."), *OSS->GetSubsystemName().ToString());

		SessionInterface = OSS->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnCreateSessionComplate);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnDestroySessionComplate);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnFindSessionComplate);

			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnJoinSessionComplate);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not found subsystem."));
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMyGameInstance::OnNetworkFailure);
	}

}



void UMyGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;
	if (SessionInterface.IsValid())
	{
		auto AlreadyExsistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (AlreadyExsistingSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is already exsist. re-createSession."),*SESSION_NAME.ToString());
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession();
		}
	}
}
void UMyGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
			SessionSettings.bIsLANMatch = true;
		else
			SessionSettings.bIsLANMatch = false;


		SessionSettings.NumPublicConnections = 24;
		SessionSettings.bUsesPresence = SessionSettings.bShouldAdvertise = true;
		SessionSettings.Set(
			SESSION_SETTINGS_KEY, DesiredServerName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);


		if (MainMenu)
			MainMenu->Shutdown();

		//�����
		SessionInterface->CreateSession(0,SESSION_NAME,SessionSettings);
	}
}

void UMyGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Finding Session"));
		//���� 100�� �ִ� ã�ƿ´�.
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")),true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}
void UMyGameInstance::OpenMainMenuLevel()
{
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC) return;

	PC->ClientTravel("/Game/Maps/Main",ETravelType::TRAVEL_Absolute);
}
void UMyGameInstance::Join(uint32 Index)
{
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;

	if (MainMenu)
		MainMenu->Shutdown();

	if(SessionSearch->SearchResults.Num() > (int32)Index)
		SessionInterface->JoinSession(0,SESSION_NAME,SessionSearch->SearchResults[Index]);
	else
		UE_LOG(LogTemp, Warning, TEXT("Empty Session"));
}

void UMyGameInstance::OnCreateSessionComplate(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Createsession"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Session name is %s"), *InSessionName.ToString());

	UEngine* Engine = GetEngine();
	if (!Engine) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Host Complate!"));

	UWorld* World = GetWorld();
	if (!World) return;

	//����(��)
	World->ServerTravel("/Game/Maps/Lobby?listen");
	//World->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
}


void UMyGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
		SessionInterface->StartSession(SESSION_NAME);
}
void UMyGameInstance::OnDestroySessionComplate(FName InSessionName, bool IsSuccess)
{
	if (IsSuccess == true)
		CreateSession();
}

void UMyGameInstance::OnFindSessionComplate(bool IsSuccess)
{
	if (IsSuccess && SessionSearch.IsValid())
	{
		TArray<FServerData> ServerNames;

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Display, TEXT("Found Session name : %s"), *SearchResult.GetSessionIdStr());
			UE_LOG(LogTemp, Display, TEXT("Ping : %d"), SearchResult.PingInMs);

			FServerData ServerData;
			ServerData.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			ServerData.CurrentPlayers = SearchResult.Session.NumOpenPublicConnections;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;

			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(SESSION_SETTINGS_KEY, ServerName))
				ServerData.Name = ServerName;
			else
				UE_LOG(LogTemp, Warning, TEXT("Session Name Not Found"));

			ServerNames.Add(ServerData);
		}

		MainMenu->SetServerList(ServerNames);

		UE_LOG(LogTemp, Warning, TEXT("Finished Finding Session"));

	}
}
void UMyGameInstance::OnJoinSessionComplate(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	if (SessionInterface.IsValid() == false) return;

	FString Address;//�ش� ���� �������ּ�
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not convert IP Address"));
		return;
	}
	UEngine* Engine = GetEngine();
	if (!Engine) return;
	Engine->AddOnScreenDebugMessage(0,5,FColor::Green,FString::Printf(TEXT("Joining To %s"),*Address));
	
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;
	PC->ClientTravel(Address,ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
}

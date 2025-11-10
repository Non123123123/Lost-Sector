// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "Components/Button.h" 
#include "Kismet/KismetSystemLibrary.h" // QuitGame �Լ� ���뵵
#include "Kismet/GameplayStatics.h" // OpenLevel �Լ� ���뵵

void UMyUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (StartGame)
	{

	}
}

void UMyUserWidget::OnStartGameClicked()
{
}

void UMyUserWidget::OnOptionClicked()
{
}

void UMyUserWidget::OnQuitGameClicked()

{
}

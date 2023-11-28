// Copyright Epic Games, Inc. All Rights Reserved.

#include "VelesEditorModule.h"
#include "Styles/VelesEditorStyle.h"
#include "VelesEditorCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "VelesCoreSubsystem.h"
#include "VelesLayersEditorModeCommands.h"
#include "Details/VelesLayersEditorDetails.h"

static const FName VelesEditorTabName("VelesPlugin");

#define LOCTEXT_NAMESPACE "FVelesEditorModule"

void FVelesEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FVelesEditorStyle::Initialize();
	FVelesEditorStyle::ReloadTextures();

	FVelesEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVelesEditorCommands::Get().RebuildAllAction,
		FExecuteAction::CreateRaw(this, &FVelesEditorModule::RebuildAllButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVelesEditorModule::RegisterMenus));

	FVelesLayersEditorModeCommands::Register();

	// register customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("VelesLayersEditorObject", FOnGetDetailCustomizationInstance::CreateStatic(&FVelesLayersEditorDetails::MakeInstance));
}

void FVelesEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FVelesEditorStyle::Shutdown();

	FVelesEditorCommands::Unregister();

	FVelesLayersEditorModeCommands::Unregister();
}

void FVelesEditorModule::RebuildAllButtonClicked()
{
	// // Put your "OnButtonClicked" stuff here
	// FText DialogText = FText::Format(
	// 						LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
	// 						FText::FromString(TEXT("FVelesEditorModule::PluginButtonClicked()")),
	// 						FText::FromString(TEXT("VelesEditorModule.cpp"))
	// 				   );
	// FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	if (GWorld)
	{
		if (UVelesCoreSubsystem* CoreSubsystem = GWorld->GetSubsystem<UVelesCoreSubsystem>())
		{
			CoreSubsystem->RebuildAll();
		}
	}
}

void FVelesEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
		{
			FToolMenuSection& Section = Menu->AddSection("VelesPlugin", LOCTEXT("VelesPlugin", "Veles Plugin"));
			Section.AddMenuEntryWithCommandList(FVelesEditorCommands::Get().RebuildAllAction, PluginCommands);
		}
	}

	// disable btn from PlayToolBar
	// {
	// 	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	// 	{
	// 		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
	// 		{
	// 			FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVelesEditorCommands::Get().PluginAction));
	// 			Entry.SetCommandList(PluginCommands);
	// 		}
	// 	}
	// }
}

// void FVelesEditorModule::OpenUtilityWidget()
// {
// 	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
// 	UEditorUtilityWidgetBlueprint* EditorWidget = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, TEXT("/VelesPlugin/UMG/WBP_VelesUtility.WBP_VelesUtility"));
// 	EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
// }

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVelesEditorModule, VelesEditor)
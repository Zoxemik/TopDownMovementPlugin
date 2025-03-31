// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownMovement.h"
#include "Engine/Engine.h"
#include "GameFramework/InputSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "InputCoreTypes.h" 

#define LOCTEXT_NAMESPACE "FTopDownMovementModule"

void FTopDownMovementModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    LoadPluginInputSettings();
}

void FTopDownMovementModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FTopDownMovementModule::LoadPluginInputSettings()
{
    UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
    if (!InputSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get Input Settings!"));
        return;
    }

    FString ConfigPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("TopDownMovement/Config/DefaultInput.ini"));
    GConfig->LoadFile(*ConfigPath);

    // --- Action Mappings ---
    TArray<FInputActionKeyMapping> ExistingActionMappings = InputSettings->GetActionMappings();
    TArray<FString> ActionMappingStrings;
    GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), ActionMappingStrings, ConfigPath);

    for (const FString& Mapping : ActionMappingStrings)
    {
        FInputActionKeyMapping ActionMapping;
        FInputActionKeyMapping::StaticStruct()->ImportText(*Mapping, &ActionMapping, nullptr, 0, nullptr, TEXT("InputMapping"));

        if (!ExistingActionMappings.Contains(ActionMapping))
        {
            InputSettings->AddActionMapping(ActionMapping);
        }
    }

    // --- Axis Mappings ---
    TArray<FInputAxisKeyMapping> ExistingAxisMappings = InputSettings->GetAxisMappings();
    TArray<FString> AxisMappingStrings;
    GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), AxisMappingStrings, ConfigPath);

    for (const FString& Mapping : AxisMappingStrings)
    {
        FInputAxisKeyMapping AxisMapping;
        FInputAxisKeyMapping::StaticStruct()->ImportText(*Mapping, &AxisMapping, nullptr, 0, nullptr, TEXT("InputMapping"));

        if (!ExistingAxisMappings.Contains(AxisMapping))
        {
            InputSettings->AddAxisMapping(AxisMapping);
        }
    }

    InputSettings->SaveKeyMappings();
    InputSettings->ForceRebuildKeymaps();
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTopDownMovementModule, TopDownMovement)
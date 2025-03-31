// The source code, authored by Zoxemik in 2025

#pragma once

#include "Modules/ModuleManager.h"

class FTopDownMovementModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Load DefaultInput.ini implementation */
	void LoadPluginInputSettings();
};

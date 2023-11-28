#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FComponentVisualizer;

class FVelesEditorVisualizersModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    void RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer);
};

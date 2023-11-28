#include "VelesEditorVisualizers.h"

#include "UnrealEd.h"
#include "VelesLayerComponent.h"
#include "Visualizers/VelesBaseComponentVisualizer.h"

#define LOCTEXT_NAMESPACE "FVelesEditorVisualizersModule"

void FVelesEditorVisualizersModule::StartupModule()
{
	RegisterComponentVisualizer(UVelesLayerComponent::StaticClass()->GetFName(), MakeShareable(new FVelesBaseComponentVisualizer));
}

void FVelesEditorVisualizersModule::ShutdownModule()
{
	if (GUnrealEd != NULL)
	{
		GUnrealEd->UnregisterComponentVisualizer(UVelesLayerComponent::StaticClass()->GetFName());
	}
}

void FVelesEditorVisualizersModule::RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd != NULL)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}

	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVelesEditorVisualizersModule, VelesEditorVisualizers)
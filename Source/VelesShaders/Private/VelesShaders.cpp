#include "VelesShaders.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FVelesShadersModule"

void FVelesShadersModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("VelesPlugin"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Veles"), PluginShaderDir);
}

void FVelesShadersModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FVelesShadersModule, VelesShaders)
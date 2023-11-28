using UnrealBuildTool;

public class VelesShaders : ModuleRules
{
    public VelesShaders(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
                "MaterialShaderQualitySettings",
            }
        );
        
        PrivateIncludePaths.AddRange(new string[] 
        {
            "VelesShaders/Private",
            // "Runtime/Renderer/Private",
        });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Renderer",
                "RenderCore",
                "RHI",
                "Projects"
            }
        );
        
        if (Target.bBuildEditor == true)
        {

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "TargetPlatform",
                    "SlateCore",
                    "Slate",
                    "UnrealEd",
                    "MaterialUtilities",
                }
            );
        }
    }
}
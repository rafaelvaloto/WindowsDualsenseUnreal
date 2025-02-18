

using UnrealBuildTool;
using System.IO;

public class WindowsDualsense_ds5w : ModuleRules
{
	public WindowsDualsense_ds5w(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Configurar caminhos para as depedências do projeto
        string PluginPath = Path.Combine(ModuleDirectory, "../");
        string ThirdPartyPath = Path.Combine(PluginPath, "ThirdParty/DualSenseWindows_V0.1");

        // Adicionar Include Path
        PublicIncludePaths.Add(ThirdPartyPath);

        // Adicionar arquivo .lib para vinculação estática
        PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "ds5w_x64.lib"));

        // Adicionar Runtime Dependency (copiar a DLL para o diretório final)
        string DLLFullPath = Path.Combine(ThirdPartyPath, "ds5w_x64.dll");
        RuntimeDependencies.Add("$(BinaryOutputDir)/ds5w_x64.dll", DLLFullPath);

        // Módulos necessários do Unreal Engine
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "ApplicationCore", "InputCore", "InputDevice"});
        
        PrivateDependencyModuleNames.AddRange(new string[]
        {
	        "Slate",
	        "SlateCore",
        });
	}
}
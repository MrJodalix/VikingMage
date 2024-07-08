// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class VikingMageTarget : TargetRules
{
	public VikingMageTarget( TargetInfo Target) : base(Target)
    {
        //BuildEnvironment = TargetBuildEnvironment.Shared;
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "VikingMage" } );
	}
}

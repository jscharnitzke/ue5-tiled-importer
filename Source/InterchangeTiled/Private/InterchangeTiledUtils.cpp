#include "InterchangeTiledUtils.h"

FString InterchangeTiled::GetAbsolutePath(FString Path, FString RelativeTo)
{
	if (!FPaths::IsRelative(Path))
	{
		return FPaths::ConvertRelativePathToFull(Path);
	}

	const FString ReferringDirectory = FPaths::GetPath(FPaths::ConvertRelativePathToFull(RelativeTo));
	FString AbsolutePath = FPaths::Combine(ReferringDirectory, Path);
	FPaths::CollapseRelativeDirectories(AbsolutePath);
	return AbsolutePath;
}

#include "InterchangeTsxTranslator.h"

#include "InterchangeTexture2DNode.h"
#include "InterchangeTiledModule.h"
#include "InterchangeTiledUtils.h"
#include "InterchangeTileSetNode.h"
#include "InterchangeTileSetFactoryNode.h"
#include "ImageUtils.h"
#include "Logging/StructuredLog.h"
#include "Misc/Optional.h"
#include "Nodes/InterchangeSourceNode.h"
#include "XmlFile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTsxTranslator)

// global variable to enable/disable TSX import from the Unreal console
static bool bGInterchangeEnableTsxImport = true;
static FAutoConsoleVariableRef CCvarInterchangeEnableTsxImport(
	TEXT("Interchange.FeatureFlags.Import.TSX"),
	bGInterchangeEnableTsxImport,
	TEXT("Whether TSX support is enabled."),
	ECVF_Default
);

bool UInterchangeTsxTranslator::CanImportSourceData(const UInterchangeSourceData* InSourceData) const
{
	if (!InSourceData)
	{
		return false;
	}

	FString Filename = InSourceData->GetFilename();
	FString Extension = FPaths::GetExtension(Filename).ToLower();

	// Only return true if the file is actually a Tiled Tileset (.tsx)
	return Extension == TEXT("tsx");
}

EInterchangeTranslatorAssetType UInterchangeTsxTranslator::GetSupportedAssetTypes() const
{
	return EInterchangeTranslatorAssetType::None;
}

TArray<FString> UInterchangeTsxTranslator::GetSupportedFormats() const
{
#if WITH_EDITOR
	if (bGInterchangeEnableTsxImport)
	{
		TArray<FString> Formats{ TEXT("tsx;Tiled Tile Set") };
		return Formats;
	}
#endif
	return TArray<FString>{};
}

EInterchangeTranslatorType UInterchangeTsxTranslator::GetTranslatorType() const
{
	return EInterchangeTranslatorType::Assets;
}

bool UInterchangeTsxTranslator::Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
	FString Filename = SourceData->GetFilename();
	FPaths::NormalizeFilename(Filename);

	if (!FPaths::FileExists(Filename))
	{
		return false;
	}

	return TranslateTileSet(Filename, BaseNodeContainer);
}

bool UInterchangeTsxTranslator::TranslateTileSet(FString Filename, UInterchangeBaseNodeContainer& BaseNodeContainer) const
{
	// Get the texture path first. 
	FString TexturePath = GetTexturePathFromSourceFilename(Filename);

	if (TexturePath.IsEmpty())
	{
		// This file isn't a valid Tiled TSX. 
		// Returning false tells the Interchange system "I can't handle this file."
		return false;
	}

	UClass* TileSetClass = UInterchangeTileSetNode::StaticClass();

	if (!ensure(TileSetClass))
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Error importing TSX tile set: UInterchangeTileSetNode is unsupported."))
			return false;
	}

	FString DisplayLabel = FPaths::GetBaseFilename(Filename);
	FString NodeUid("tsx:" + DisplayLabel);

	UInterchangeTileSetNode* TileSetNode = NewObject<UInterchangeTileSetNode>(&BaseNodeContainer, TileSetClass);

	if (!ensure(TileSetNode))
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Error importing TSX tile set: Failed to create UInterchangeTileSetNode."));
		return false;
	}

	TileSetNode->InitializeNode(
		NodeUid,
		DisplayLabel,
		EInterchangeNodeContainerType::TranslatedAsset
	);

	// TexturePath is valid
	TileSetNode->SetAttribute("TextureFilename", TexturePath);

	BaseNodeContainer.AddNode(TileSetNode);

	return true;
}

FString UInterchangeTsxTranslator::GetTexturePathFromSourceFilename(FString Filename)
{
	FXmlFile TileSetFile(Filename);
	FXmlNode* RootNode = TileSetFile.GetRootNode();
	// Ensure the file parsed as XML
	if (!RootNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tiled Importer: Failed to parse %s as XML."), *Filename);
		return FString();
	}

	const FXmlNode* ImageNode = RootNode->FindChildNode("image");

	// : Ensure the <image> tag exists
	if (!ImageNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tiled Importer: No <image> node found in %s"), *Filename);
		return FString();
	}

	FString ImageSource = ImageNode->GetAttribute("source");
	return InterchangeTiled::GetAbsolutePath(ImageSource, Filename);
}

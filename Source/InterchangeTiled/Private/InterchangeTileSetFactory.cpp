#include "InterchangeTileSetFactory.h"

#include "AssetToolsModule.h"
#include "AutomatedAssetImportData.h"
#include "Factories/TextureFactory.h"
#include "InterchangeSourceData.h"
#include "InterchangeTextureFactoryNode.h"
#include "InterchangeTiledModule.h"
#include "InterchangeTileSetFactoryNode.h"
#include "Logging/StructuredLog.h"
#include "Modules/ModuleManager.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTileSetFactory)

UInterchangeFactoryBase::FImportAssetResult UInterchangeTileSetFactory::BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInterchangeTileSetFactory::BeginImportAsset_GameThread);

	FImportAssetResult ImportAssetResult;
	UPaperTileSet* TileSet = nullptr;

	if (!Arguments.AssetNode)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileSetFactory_AssetNodeNull", "Asset node parameter is null."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	const UClass* TileSetClass = Arguments.AssetNode->GetObjectClass();
	if (!TileSetClass || !TileSetClass->IsChildOf(UPaperTileSet::StaticClass()))
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileSetFactory_NodeClassMissmatch", "Asset node parameter class doesn't derive from UPaperTileSet."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	UObject* ExistingAsset = Arguments.ReimportObject;

	// even if not flagged as a re-import, try to load the asset in case
	// it already exists
	if (!ExistingAsset)
	{
		FSoftObjectPath ReferenceObject;
		if (Arguments.AssetNode->GetCustomReferenceObject(ReferenceObject))
		{
			ExistingAsset = ReferenceObject.TryLoad();
		}
	}

	if (!ExistingAsset)
	{
		TileSet = NewObject<UPaperTileSet>(
			Arguments.Parent,
			TileSetClass,
			*Arguments.AssetName,
			RF_Public | RF_Standalone
		);
	}

	if (!TileSet)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileSetFactory_TileSetCreateFail", "Tile Set creation failed."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	ImportAssetResult.ImportedObject = TileSet;

	return ImportAssetResult;
}

void UInterchangeTileSetFactory::SetupObject_GameThread(const FSetupObjectParams& Arguments)
{
	if (Arguments.bIsReimport)
	{
		return;
	}

	if (Arguments.FactoryNode->GetClass() != UInterchangeTileSetFactoryNode::StaticClass())
	{
		UE_LOGFMT(LogInterchangeTiledImport, Warning, "Invalid Factory Node class: {ClassName}", Arguments.FactoryNode->GetClass()->GetName());

		return;
	}

	UTexture2D* Texture = LoadOrCreateTextureAsset(
		Arguments.FactoryNode,
		FPaths::GetPath(Arguments.ImportedObject->GetPathName())
	);

	FXmlFile TileSetFile(Arguments.SourceData->GetFilename());
	FXmlNode* RootNode = TileSetFile.GetRootNode();

	FString TileWidth = RootNode->GetAttribute("tilewidth");
	FString TileHeight = RootNode->GetAttribute("tileheight");
	FString TileSpacing = RootNode->GetAttribute("spacing");
	FString ImageMargin = RootNode->GetAttribute("margin");
	FString TileCount = RootNode->GetAttribute("tilecount");
	FString ColumnCount = RootNode->GetAttribute("columns");

	UPaperTileSet* TileSet = Cast<UPaperTileSet>(Arguments.ImportedObject);

	TileSet->SetTileSize(FIntPoint(FCString::Atoi(*TileWidth), FCString::Atoi(*TileHeight)));
	TileSet->SetMargin(FCString::Atoi(*ImageMargin));
	TileSet->SetPerTileSpacing(FCString::Atoi(*TileSpacing));
	TileSet->SetTileSheetTexture(Texture);
	// trigger PostEdit to init PerTileData
	TileSet->PostEditChange();

	PopulateTileMetadata(RootNode->GetChildrenNodes(), TileSet);
}

UTexture2D* UInterchangeTileSetFactory::LoadOrCreateTextureAsset(
	UInterchangeFactoryBaseNode* FactoryNode,
	FString PackagePath
)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	UInterchangeTileSetFactoryNode* TileSetFactoryNode = Cast<UInterchangeTileSetFactoryNode>(FactoryNode);

	check(TileSetFactoryNode);

	UTexture2D* Texture = nullptr;

	FString TextureFilename;
	TileSetFactoryNode->GetAttribute("TextureFilename", TextureFilename);

	if (!TextureFilename.Len())
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Invalid Texture filename"));
	}

	FString Filename = FPaths::GetBaseFilename(TextureFilename);
	TArray<FString> Filenames;
	Filenames.Add(TextureFilename);

	UAutomatedAssetImportData* ImportSettings = NewObject<UAutomatedAssetImportData>(UAutomatedAssetImportData::StaticClass());
	ImportSettings->bReplaceExisting = false;
	ImportSettings->DestinationPath = PackagePath;
	ImportSettings->Factory = NewObject<UTextureFactory>(UTextureFactory::StaticClass());
	ImportSettings->FactoryName = UTextureFactory::StaticClass()->GetName();
	ImportSettings->Filenames = Filenames;

	TArray<UObject*> NewAssets = AssetToolsModule.Get().ImportAssetsAutomated(ImportSettings);

	if (NewAssets.IsEmpty())
	{
		UE_LOG(LogInterchangeTiledImport, Warning, TEXT("Failed to import texture '%s'."), *TextureFilename);
		return nullptr;
	}

	Texture = Cast<UTexture2D>(NewAssets[0]);

	return Texture;
}

void UInterchangeTileSetFactory::PopulateTileMetadata(TArray<FXmlNode*> TilesetNodes, UPaperTileSet* TileSet)
{
	for (const FXmlNode* Node : TilesetNodes)
	{
		if (Node->GetTag() == "tile")
		{
			const uint32 TileId = FCString::Atoi(*Node->GetAttribute("id"));
			const FXmlNode* ObjectGroupNode = Node->FindChildNode("objectgroup");

			if (!ObjectGroupNode)
			{
				continue;
			}

			const FXmlNode* ObjectNode = ObjectGroupNode->FindChildNode("object");

			if (!ObjectNode)
			{
				continue;
			}

			FIntPoint TileSize = TileSet->GetTileSize();
			double TileWidth = TileSize.X;
			double TileHeight = TileSize.Y;

			double TiledX = FCString::Atod(*ObjectNode->GetAttribute("x"));
			double TiledY = FCString::Atod(*ObjectNode->GetAttribute("y"));
			double BoxWidth = FCString::Atod(*ObjectNode->GetAttribute("width"));
			double BoxHeight = FCString::Atod(*ObjectNode->GetAttribute("height"));

			// Convert from Tiled coordinates (collision box top left) to
			// Unreal coordinates (collision box center).
			double UnrealX = TiledX - TileWidth / 2 + BoxWidth / 2;
			double UnrealY = TiledY - TileHeight / 2 + BoxHeight / 2;

			FVector2D RectanglePosition(UnrealX, UnrealY);
			FVector2D RectangleSize(BoxWidth, BoxHeight);

			FSpriteGeometryCollection CollisionData;
			CollisionData.AddRectangleShape(
				RectanglePosition,
				RectangleSize
			);

			FPaperTileMetadata TileMetadata;
			TileMetadata.CollisionData = CollisionData;

			FPaperTileMetadata* CurrentTileMetadata = TileSet->GetMutableTileMetadata(TileId);
			*CurrentTileMetadata = TileMetadata;
		}
	}
}

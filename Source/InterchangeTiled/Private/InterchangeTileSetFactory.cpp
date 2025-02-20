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

	// lambda to standardize log message when asset creation fails
	auto CouldNotCreateTileSetLog = [this, &Arguments, &ImportAssetResult](const FText& Info)
	{
		UInterchangeResultError_Generic* Message = AddMessage<UInterchangeResultError_Generic>();
		Message->SourceAssetName = Arguments.SourceData->GetFilename();
		Message->DestinationAssetName = Arguments.AssetName;
		Message->AssetType = GetFactoryClass();
		Message->Text = FText::Format(LOCTEXT("TileSetFact_CouldNotCreateMat", "UInterchangeTileSetFactory: Could not create tile set asset %s. Reason: %s"), FText::FromString(Arguments.AssetName), Info);
		ImportAssetResult.bIsFactorySkipAsset = true;
	};

	if (!Arguments.AssetNode)
	{
		CouldNotCreateTileSetLog(LOCTEXT("TileSetFactory_AssetNodeNull", "Asset node parameter is null."));
		return ImportAssetResult;
	}

	const UClass* TileSetClass = Arguments.AssetNode->GetObjectClass();
	if (!TileSetClass || !TileSetClass->IsChildOf(UPaperTileSet::StaticClass()))
	{
		CouldNotCreateTileSetLog(LOCTEXT("TileSetFactory_NodeClassMissmatch", "Asset node parameter class doesn't derive from UPaperTileSet."));
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
		CouldNotCreateTileSetLog(LOCTEXT("TileSetFactory_TileSetCreateFail", "Tile Set creation failed."));
		return ImportAssetResult;
	}

	ImportAssetResult.ImportedObject = TileSet;

	return ImportAssetResult;
}

void UInterchangeTileSetFactory::SetupObject_GameThread(const FSetupObjectParams& Arguments)
{
	if (Arguments.FactoryNode->GetClass() != UInterchangeTileSetFactoryNode::StaticClass())
	{
		UE_LOGFMT(LogTemp, Warning, "Invalid Factory Node class: {ClassName}", Arguments.FactoryNode->GetClass()->GetName());

		return;
	}

	UTexture2D* Texture = LoadOrCreateTextureAsset(
		Arguments.FactoryNode,
		FPaths::GetPath(Arguments.ImportedObject->GetPathName())
	);

	FXmlFile TileSetFile(Arguments.SourceData->GetFilename());
	FXmlNode* RootNode = TileSetFile.GetRootNode();

	FString TileSetName = RootNode->GetAttribute("name");
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
	TileSet->PostEditChange();

	PopulateTileMetadata(RootNode->GetChildrenNodes(), TileSet);
}

void UInterchangeTileSetFactory::BuildObject_GameThread(const FSetupObjectParams& Arguments, bool& OutPostEditchangeCalled)
{
	Super::BuildObject_GameThread(Arguments, OutPostEditchangeCalled);

	UPaperTileSet* TileSet = Cast<UPaperTileSet>(Arguments.ImportedObject);
	const FPaperTileMetadata* TileMetadata = TileSet->GetTileMetadata(0);
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
		UE_LOG(LogTemp, Warning, TEXT("Invalid Texture filename"));
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
	UObject* NewAsset = NewAssets[0];
	Texture = Cast<UTexture2D>(NewAsset);

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

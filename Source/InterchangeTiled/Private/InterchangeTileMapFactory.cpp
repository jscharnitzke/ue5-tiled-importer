#include "InterchangeTileMapFactory.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "InterchangeSourceData.h"
#include "InterchangeTiledModule.h"
#include "PaperTileLayer.h"
#include "PaperTileSet.h"
#include "XmlFile.h"

UInterchangeFactoryBase::FImportAssetResult UInterchangeTileMapFactory::BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInterchangeTileMapFactory::BeginImportAsset_GameThread);

	FImportAssetResult ImportAssetResult;
	UPaperTileMap* TileMap = nullptr;

	if (!Arguments.AssetNode)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_AssetNodeNull", "Asset node parameter is null."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	const UClass* TileMapClass = Arguments.AssetNode->GetObjectClass();

	if (!TileMapClass || !TileMapClass->IsChildOf(UPaperTileMap::StaticClass()))
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_NodeClassMissmatch", "Asset node parameter class doesn't derive from UPaperTileMap."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	UObject* ExistingAsset = Arguments.ReimportObject;

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
		TileMap = NewObject<UPaperTileMap>(
			Arguments.Parent,
			TileMapClass,
			*Arguments.AssetName,
			RF_Public | RF_Standalone
		);
	}

	if (!TileMap)
	{
		LogAssetCreationError(
			Arguments,
			LOCTEXT("TileMapFactory_TileMapCreateFail", "Tile Map creation failed."),
			ImportAssetResult
		);

		return ImportAssetResult;
	}

	ImportAssetResult.ImportedObject = TileMap;

	return ImportAssetResult;
}

TArray<FTilesetImportInfo> UInterchangeTileMapFactory::LoadTileSets(const FSetupObjectParams& Arguments, const FString& TileMapPathName)
{
	TArray<FTilesetImportInfo> TileSets;

	FString CountStr;
	Arguments.FactoryNode->GetAttribute("TileSetCount", CountStr);
	int32 TileSetCount = FCString::Atoi(*CountStr);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	for (int32 i = 0; i < TileSetCount; ++i)
	{
		FTilesetImportInfo Info;

		FString Filename;
		if (Arguments.FactoryNode->GetAttribute<FString>(FString::Printf(TEXT("TileSetFilename[%d]"), i), Filename))
		{
			FString TileSetAssetName = FPaths::GetBaseFilename(Filename) + "_tile_set";
			FString ObjectPath = FPaths::Combine(TileMapPathName, TileSetAssetName);

			TSet<FTopLevelAssetPath> ClassPaths{ UPaperTileSet::StaticClass()->GetClassPathName() };
			TSet<FName> PackageNames{ FName(*ObjectPath) };

			TArray<FAssetData> AssetData;
			FARCompiledFilter Filter;
			Filter.ClassPaths = ClassPaths;
			Filter.PackageNames = PackageNames;
			AssetRegistryModule.Get().GetAssets(Filter, AssetData);

			if (AssetData.Num() > 0)
			{
				Info.TileSet = Cast<UPaperTileSet>(AssetData[0].GetAsset());
			}
		}

		FString FirstGidStr;
		if (Arguments.FactoryNode->GetAttribute<FString>(FString::Printf(TEXT("TileSetFirstGid[%d]"), i), FirstGidStr))
		{
			Info.FirstGid = FCString::Atoi(*FirstGidStr);
		}

		TileSets.Add(Info);
	}

	return TileSets;
}

void UInterchangeTileMapFactory::SetupTileMapDimensions(UPaperTileMap* TileMap, FXmlNode* RootNode)
{
	TileMap->MapHeight = FCString::Atoi(*RootNode->GetAttribute(TEXT("height")));
	TileMap->MapWidth = FCString::Atoi(*RootNode->GetAttribute(TEXT("width")));
	TileMap->TileHeight = FCString::Atoi(*RootNode->GetAttribute(TEXT("tileheight")));
	TileMap->TileWidth = FCString::Atoi(*RootNode->GetAttribute(TEXT("tilewidth")));

	FString Orientation = RootNode->GetAttribute(TEXT("orientation"));
	if (Orientation.Equals(TEXT("hexagonal"), ESearchCase::IgnoreCase))
	{
		TileMap->ProjectionMode = ETileMapProjectionMode::HexagonalStaggered;
		TileMap->HexSideLength = FCString::Atoi(*RootNode->GetAttribute(TEXT("hexsidelength")));
	}
	else if (Orientation.Equals(TEXT("orthogonal"), ESearchCase::IgnoreCase))
	{
		TileMap->ProjectionMode = ETileMapProjectionMode::Orthogonal;
	}
	else if (Orientation.Equals(TEXT("isometric"), ESearchCase::IgnoreCase))
	{
		TileMap->ProjectionMode = ETileMapProjectionMode::IsometricDiamond;
	}
}

void UInterchangeTileMapFactory::PopulateLayerTiles(
	UPaperTileLayer* Layer,
	const FString& LayerData,
	int32 LayerWidth,
	int32 LayerHeight,
	const TArray<FTilesetImportInfo>& TileSets
)
{
	TArray<FString> TileUids;
	LayerData.ParseIntoArray(TileUids, TEXT(","));

	for (int TileIndex = 0; TileIndex < LayerWidth * LayerHeight; TileIndex++)
	{
		int32 TileUid = FCString::Atoi(*TileUids[TileIndex]);

		if (TileUid == 0)
		{
			continue;
		}

		UPaperTileSet* TileSet = nullptr;
		int32 LocalIndex = TileUid;

		for (int32 i = TileSets.Num() - 1; i >= 0; --i)
		{
			if (TileUid >= TileSets[i].FirstGid)
			{
				TileSet = TileSets[i].TileSet;
				LocalIndex = TileUid - TileSets[i].FirstGid;
				break;
			}
		}

		if (!TileSet)
		{
			continue;
		}

		int TileX = TileIndex % LayerWidth;
		int TileY = TileIndex / LayerWidth;

		FPaperTileInfo TileInfo;
		TileInfo.TileSet = TileSet;
		TileInfo.PackedTileIndex = LocalIndex;

		Layer->SetCell(TileX, TileY, TileInfo);
	}
}

void UInterchangeTileMapFactory::ImportTileLayer(UPaperTileMap* TileMap, FXmlNode* LayerNode, const TArray<FTilesetImportInfo>& TileSets)
{
	FString LayerName = LayerNode->GetAttribute("name");
	int LayerHeight = FCString::Atoi(*LayerNode->GetAttribute("height"));
	int LayerWidth = FCString::Atoi(*LayerNode->GetAttribute("width"));

	UPaperTileLayer* Layer = NewObject<UPaperTileLayer>(
		TileMap,
		UPaperTileLayer::StaticClass()
	);

	TileMap->TileLayers.Add(Layer);

	Layer->DestructiveAllocateMap(LayerWidth, LayerHeight);
	Layer->LayerName = FText::FromString(LayerName);

	const FXmlNode* LayerDataNode = LayerNode->GetFirstChildNode();
	FString LayerData = LayerDataNode->GetContent();

	PopulateLayerTiles(Layer, LayerData, LayerWidth, LayerHeight, TileSets);
}

void UInterchangeTileMapFactory::SetupObject_GameThread(const FSetupObjectParams& Arguments)
{
	FString TileMapPathName = FPaths::GetPath(Arguments.ImportedObject->GetPathName());
	TArray<FTilesetImportInfo> TileSets = LoadTileSets(Arguments, TileMapPathName);

	FXmlFile TileMapFile(Arguments.SourceData->GetFilename());
	FXmlNode* RootNode = TileMapFile.GetRootNode();

	UPaperTileMap* TileMap = Cast<UPaperTileMap>(Arguments.ImportedObject);

	if (TileSets.Num() > 0 && TileSets[0].TileSet != nullptr)
	{
		TileMap->InitializeNewEmptyTileMap(TileSets[0].TileSet);
	}

	SetupTileMapDimensions(TileMap, RootNode);

	TArray<FXmlNode*> ChildrenNodes = RootNode->GetChildrenNodes();
	for (FXmlNode* ChildNode : ChildrenNodes)
	{
		if (ChildNode->GetTag() == "layer")
		{
			ImportTileLayer(TileMap, ChildNode, TileSets);
		}
	}
}

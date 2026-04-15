#pragma once

#include "CoreMinimal.h"
#include "InterchangeFactoryBase.h"
#include "InterchangeTiledFactory.h"
#include "PaperTileMap.h"
#include "XmlFile.h"

#include "InterchangeTileMapFactory.generated.h"

USTRUCT()
struct FTilesetImportInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UPaperTileSet> TileSet = nullptr;

	UPROPERTY()
	int32 FirstGid = 0;
};

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileMapFactory : public UInterchangeTiledFactory
{
	GENERATED_BODY()


public:

	virtual UClass* GetFactoryClass() const override
	{
		return UPaperTileMap::StaticClass();
	}

private:

	virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;

	virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;

	TArray<FTilesetImportInfo> LoadTileSets(const FSetupObjectParams& Arguments, const FString& TileMapPathName);

	void SetupTileMapDimensions(UPaperTileMap* TileMap, FXmlNode* RootNode);

	void ImportTileLayer(
		UPaperTileMap* TileMap,
		FXmlNode* LayerNode,
		const TArray<FTilesetImportInfo>& TileSets
	);

	void PopulateLayerTiles(
		UPaperTileLayer* Layer,
		const FString& LayerData,
		int32 LayerWidth,
		int32 LayerHeight,
		const TArray<FTilesetImportInfo>& TileSets
	);
};

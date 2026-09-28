#pragma once

#include "CoreMinimal.h"
#include "InterchangeFactoryBase.h"
#include "InterchangeTiledFactory.h"
#include "Misc/EngineVersionComparison.h"
#include "PaperTileSet.h"
#include "XmlFile.h"

#include "InterchangeTileSetFactory.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileSetFactory : public UInterchangeTiledFactory
{
	GENERATED_BODY()
	

public:

	virtual UClass* GetFactoryClass() const override
	{
		return UPaperTileSet::StaticClass();
	}

	virtual EInterchangeFactoryAssetType GetFactoryAssetType() override
	{
		// Custom was added in UE 5.8, where None is treated as a scene
		// node and skipped during asset imports.
#if UE_VERSION_NEWER_THAN(5, 8, 0)
		return EInterchangeFactoryAssetType::Custom;
#else
		return EInterchangeFactoryAssetType::None;
#endif
	}

private:

	virtual FImportAssetResult BeginImportAsset_GameThread(const FImportAssetObjectParams& Arguments) override;

	virtual void SetupObject_GameThread(const FSetupObjectParams& Arguments) override;

	virtual UTexture2D* LoadOrCreateTextureAsset(
		UInterchangeFactoryBaseNode* FactoryNode,
		FString PackagePath
	);

	virtual void PopulateTileMetadata(TArray<FXmlNode*> TilesetNodes, UPaperTileSet* TileSet);
};

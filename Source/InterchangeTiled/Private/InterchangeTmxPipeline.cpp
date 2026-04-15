#include "InterchangeTmxPipeline.h"

#include "InterchangeManager.h"
#include "InterchangeTileMapFactoryNode.h"
#include "InterchangeTileMapNode.h"
#include "Misc/SecureHash.h"
#include "PaperTileMap.h"

FString UInterchangeTmxPipeline::GetPipelineCategory(UClass* AssetClass)
{
	return TEXT("Assets");
}

void UInterchangeTmxPipeline::GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const
{
	PipelineSupportAssetClasses.Add(UPaperTileMap::StaticClass());
}

void UInterchangeTmxPipeline::ExecutePipeline(
	UInterchangeBaseNodeContainer* BaseNodeContainer, 
	const TArray<UInterchangeSourceData*>& InSourceDatas, 
	const FString& ContentBasePath
)
{
	if (!BaseNodeContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInterchangeTmxPipeline: Cannot execute pre-import pipeline because BaseNodeContainer is null"));
		return;
	}

	ensure(Results);

	TArray<FString> TileMapNodeUids;
	BaseNodeContainer->GetNodes(
		UInterchangeTileMapNode::StaticClass(), TileMapNodeUids
	);

	UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();

	for (FString TileMapNodeUid : TileMapNodeUids)
	{
		const UInterchangeBaseNode* Node = BaseNodeContainer->GetNode(TileMapNodeUid);
		const UInterchangeTileMapNode* TileMapNode = Cast<UInterchangeTileMapNode>(Node);

		TArray<FString> TileSetFilenames;
		TArray<int32> TileSetFirstGids;
		FString CountStr;
		if (TileMapNode->GetAttribute("TileSetCount", CountStr))
		{
			int32 TilesetCount = FCString::Atoi(*CountStr);
			for (int32 i = 0; i < TilesetCount; ++i)
			{
				FString Filename;
				if (TileMapNode->GetAttribute<FString>(FString::Printf(TEXT("TileSetFilename[%d]"), i), Filename))
				{
					TileSetFilenames.Add(Filename);
				}
				FString FirstGidStr;
				if (TileMapNode->GetAttribute<FString>(FString::Printf(TEXT("TileSetFirstGid[%d]"), i), FirstGidStr))
				{
					TileSetFirstGids.Add(FCString::Atoi(*FirstGidStr));
				}
			}
		}

		UInterchangeTileMapFactoryNode* TileMapFactoryNode = NewObject<UInterchangeTileMapFactoryNode>(
			BaseNodeContainer,
			UInterchangeTileMapFactoryNode::StaticClass()
		);
		TileMapFactoryNode->InitializeNode(
			UInterchangeFactoryBaseNode::BuildFactoryNodeUid(TileMapNodeUid),
			TileMapNode->GetDisplayLabel() + "_tile_map",
			EInterchangeNodeContainerType::FactoryData
		);

		TileMapFactoryNode->SetAttribute<FString>("TileSetCount", CountStr);
		for (int32 i = 0; i < TileSetFilenames.Num(); ++i)
		{
			TileMapFactoryNode->SetAttribute<FString>(FString::Printf(TEXT("TileSetFilename[%d]"), i), TileSetFilenames[i]);
			TileMapFactoryNode->SetAttribute<FString>(FString::Printf(TEXT("TileSetFirstGid[%d]"), i), FString::FromInt(TileSetFirstGids[i]));
		}

		for (int32 i = 0; i < TileSetFilenames.Num(); ++i)
		{
			UInterchangeSourceData* SourceData = InterchangeManager.CreateSourceData(TileSetFilenames[i]);

			SourceData->GetFileContentHash();

			FImportAssetParameters ImportAssetParameters;
			ImportAssetParameters.bReplaceExisting = false;
			ImportAssetParameters.bIsAutomated = true;

			InterchangeManager.ImportAsset(
				ContentImportPath,
				SourceData,
				ImportAssetParameters
			);
		}

		BaseNodeContainer->AddNode(TileMapFactoryNode);
	}
}

#include "InterchangeTsxPipeline.h"

#include "Logging/StructuredLog.h"
#include "PaperTileSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InterchangeTsxPipeline)

FString UInterchangeTsxPipeline::GetPipelineCategory(UClass* AssetClass)
{
	return TEXT("Assets");
}

void UInterchangeTsxPipeline::GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const
{
	PipelineSupportAssetClasses.Add(UPaperTileSet::StaticClass());
}

void UInterchangeTsxPipeline::ExecutePipeline(
	UInterchangeBaseNodeContainer* BaseNodeContainer, 
	const TArray<UInterchangeSourceData*>& InSourceDatas, 
	const FString& ContentBasePath
)
{
	if (!BaseNodeContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInterchangeTsxPipeline: Cannot execute pre-import pipeline because BaseNodeContainer is null"));
		return;
	}

	ensure(Results);

	TArray<FString> TileSetNodeUids;
	BaseNodeContainer->GetNodes(UInterchangeTileSetNode::StaticClass(), TileSetNodeUids);

	for (FString TileSetNodeUid : TileSetNodeUids)
	{
		const UInterchangeBaseNode* Node = BaseNodeContainer->GetNode(TileSetNodeUid);
		const UInterchangeTileSetNode* TileSetNode = Cast<UInterchangeTileSetNode>(Node);

		FString TextureFilename;
		TileSetNode->GetAttribute("TextureFilename", TextureFilename);

		UInterchangeTileSetFactoryNode* TileSetFactoryNode = NewObject<UInterchangeTileSetFactoryNode>(
			BaseNodeContainer,
			UInterchangeTileSetFactoryNode::StaticClass()
		);

		TileSetFactoryNode->InitializeNode(
			UInterchangeFactoryBaseNode::BuildFactoryNodeUid(TileSetNodeUid),
			TileSetNode->GetDisplayLabel() + "_tile_set",
			EInterchangeNodeContainerType::FactoryData
		);
		TileSetFactoryNode->SetAttribute("TextureFilename", TextureFilename);

		// Link the factory node to the translated node so the import
		// task knows which node it consumes.
		TileSetFactoryNode->AddTargetNodeUid(TileSetNodeUid);
		TileSetNode->AddTargetNodeUid(TileSetFactoryNode->GetUniqueID());

		BaseNodeContainer->AddNode(TileSetFactoryNode);
	}
}

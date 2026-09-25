// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterchangePipelineBase.h"
#include "InterchangeTileSetNode.h"
#include "InterchangeTileSetFactoryNode.h"
#include "InterchangeTsxPipeline.generated.h"

class UInterchangeGenericTexturePipeline;

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTsxPipeline : public UInterchangePipelineBase
{
	GENERATED_BODY()

public:
	static FString GetPipelineCategory(UClass* AssetClass);

	// Creates the factory node for a translated tile set node, including
	// the bidirectional target link. Shared with the TMX pipeline, which
	// also produces tile set factory nodes for a map's tile set references.
	static UInterchangeTileSetFactoryNode* CreateTileSetFactoryNode(
		const UInterchangeTileSetNode* TileSetNode,
		UInterchangeBaseNodeContainer* BaseNodeContainer
	);

	// The base class virtual was added in UE 5.5, so no override specifier.
	virtual void GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const;

protected:

	virtual void ExecutePipeline(
		UInterchangeBaseNodeContainer* BaseNodeContainer, 
		const TArray<UInterchangeSourceData*>& InSourceDatas, 
		const FString& ContentBasePath
	) override;
};

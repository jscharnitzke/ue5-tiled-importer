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
	UInterchangeTsxPipeline();

	static FString GetPipelineCategory(UClass* AssetClass);

	/** The name of the pipeline that will be display in the import dialog. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets", meta = (StandAlonePipelineProperty = "True", PipelineInternalEditionData = "True"))
	FString PipelineDisplayName;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Instanced, Category = "Textures")
	TObjectPtr<UInterchangeGenericTexturePipeline> TexturePipeline;

	virtual void GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const override;

protected:

	virtual void ExecutePipeline(UInterchangeBaseNodeContainer* InBaseNodeContainer, const TArray<UInterchangeSourceData*>& InSourceDatas, const FString& ContentBasePath) override;

	UPROPERTY()
	TObjectPtr<UInterchangeBaseNodeContainer> BaseNodeContainer;

private:

	TArray<UInterchangeTileSetNode*> TileSetNodes;
	TArray<UInterchangeTileSetFactoryNode*> TileSetFactoryNodes;
};

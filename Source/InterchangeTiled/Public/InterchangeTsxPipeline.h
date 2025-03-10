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

	virtual void GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const override;

protected:

	virtual void ExecutePipeline(
		UInterchangeBaseNodeContainer* BaseNodeContainer, 
		const TArray<UInterchangeSourceData*>& InSourceDatas, 
		const FString& ContentBasePath
	) override;
};

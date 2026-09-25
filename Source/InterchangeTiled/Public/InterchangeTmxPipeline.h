#pragma once

#include "CoreMinimal.h"
#include "InterchangePipelineBase.h"
#include "InterchangeTileMapFactoryNode.h"
#include "InterchangeTsxPipeline.h"

#include "InterchangeTmxPipeline.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTmxPipeline : public UInterchangePipelineBase
{
	GENERATED_BODY()

public:
	static FString GetPipelineCategory(UClass* AssetClass);

	// The base class virtual was added in UE 5.5, so no override specifier.
	virtual void GetSupportAssetClasses(TArray<UClass*>& PipelineSupportAssetClasses) const;

protected:

	virtual void ExecutePipeline(
		UInterchangeBaseNodeContainer* BaseNodeContainer, 
		const TArray<UInterchangeSourceData*>& InSourceDatas, 
		const FString& ContentBasePath
	) override;

};

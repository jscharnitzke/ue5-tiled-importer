// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterchangeTranslatorBase.h"
#include "InterchangeTmxTranslator.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTmxTranslator : public UInterchangeTranslatorBase
{
	GENERATED_BODY()

public:

	// inherited methods
	virtual bool CanImportSourceData(const UInterchangeSourceData* InSourceData) const;
	virtual bool IsThreadSafe() const { return false; }
	virtual EInterchangeTranslatorAssetType GetSupportedAssetTypes() const override;
	virtual TArray<FString> GetSupportedFormats() const override;
	virtual EInterchangeTranslatorType GetTranslatorType() const override;
	virtual bool Translate(UInterchangeBaseNodeContainer& BaseNodeContainer) const override;
	
private:

	bool TranslateTileMap(FString Filename, UInterchangeBaseNodeContainer& BaseNodeContainer) const;

	struct FTilesetReference
	{
		FString Filename;
		int32 FirstGid;
	};

	static TArray<FTilesetReference> GetTilesetReferencesFromSourceFilename(FString Filename);
};

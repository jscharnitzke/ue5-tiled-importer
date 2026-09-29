// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterchangeTranslatorBase.h"
#include "InterchangeTsxTranslator.generated.h"

UCLASS(BlueprintType, EditInlineNew, MinimalAPI)
class UInterchangeTsxTranslatorSettings : public UInterchangeTranslatorSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "TSX Translator")
	bool bKeepTsxNamespace = false;
};

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTsxTranslator : public UInterchangeTranslatorBase
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

	// Also used by the TMX translator to translate referenced tile sets
	// into the same node container.
	bool TranslateTileSet(
		FString Filename,
		UInterchangeBaseNodeContainer& BaseNodeContainer
	) const;

private:

	static FString GetTexturePathFromSourceFilename(FString Filename);
};

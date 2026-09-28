// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/InterchangeBaseNode.h"
#include "InterchangeTileMapNode.generated.h"

/**
 * 
 */
UCLASS()
class INTERCHANGETILED_API UInterchangeTileMapNode : public UInterchangeBaseNode
{
	GENERATED_BODY()

public:
	virtual FString GetTypeName() const override;
	virtual FString GetAssetName() const override { return AssetName; }

	virtual bool SetAssetName(const FString& NewAssetName) override
	{
		AssetName = NewAssetName;
		return true;
	}

protected:

	UPROPERTY(EditAnywhere, Category = "Tiled")
	FString AssetName;
	
};

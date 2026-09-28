#include "InterchangeTiledModule.h"

#include "InterchangeManager.h"
#include "InterchangeProjectSettings.h"
#include "InterchangeTileMapFactory.h"
#include "InterchangeTileSetFactory.h"
#include "InterchangeTmxTranslator.h"
#include "InterchangeTsxTranslator.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FInterchangeTiled, InterchangeTiled);

DEFINE_LOG_CATEGORY(LogInterchangeTiledImport);

namespace
{
	void RegisterTranslatorPipeline(
		FInterchangePipelineStack& PipelineStack,
		const TSoftClassPtr<UInterchangeTranslatorBase>& Translator,
		const TCHAR* PipelinePath)
	{
		FInterchangeTranslatorPipelines* Entry = PipelineStack.PerTranslatorPipelines.FindByPredicate(
			[&Translator](const FInterchangeTranslatorPipelines& Item)
			{
				return Item.Translator == Translator;
			});
		if (!Entry)
		{
			Entry = &PipelineStack.PerTranslatorPipelines.AddDefaulted_GetRef();
			Entry->Translator = Translator;
		}
		Entry->Pipelines.AddUnique(FSoftObjectPath(PipelinePath));
	}
}

void FInterchangeTiled::StartupModule()
{
	UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();

	InterchangeManager.RegisterTranslator(UInterchangeTmxTranslator::StaticClass());
	InterchangeManager.RegisterTranslator(UInterchangeTsxTranslator::StaticClass());

	InterchangeManager.RegisterFactory(UInterchangeTileMapFactory::StaticClass());
	InterchangeManager.RegisterFactory(UInterchangeTileSetFactory::StaticClass());

	// Register the default pipelines for our translators on the default
	// pipeline stack, so imports work without manual project setup.
	FInterchangeImportSettings& ImportSettings =
		FInterchangeProjectSettingsUtils::GetMutableDefaultImportSettings(false);
	if (FInterchangePipelineStack* PipelineStack =
			ImportSettings.PipelineStacks.Find(ImportSettings.DefaultPipelineStack))
	{
		RegisterTranslatorPipeline(
			*PipelineStack,
			UInterchangeTmxTranslator::StaticClass(),
			TEXT("/Script/InterchangeTiled.Default__InterchangeTmxPipeline"));
		RegisterTranslatorPipeline(
			*PipelineStack,
			UInterchangeTsxTranslator::StaticClass(),
			TEXT("/Script/InterchangeTiled.Default__InterchangeTsxPipeline"));
	}
}

void FInterchangeTiled::ShutdownModule()
{
}

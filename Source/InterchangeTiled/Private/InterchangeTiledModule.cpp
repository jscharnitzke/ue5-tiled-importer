#include "InterchangeTiledModule.h"

#include "InterchangeManager.h"
#include "InterchangeTsxTranslator.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FInterchangeTiled, InterchangeTiled);

DEFINE_LOG_CATEGORY(LogInterchangeTiledImport);

void FInterchangeTiled::StartupModule()
{
	UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();

	InterchangeManager.RegisterTranslator(UInterchangeTsxTranslator::StaticClass());
}

void FInterchangeTiled::ShutdownModule()
{
}

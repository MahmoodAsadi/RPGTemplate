// Copyright Pixelation Labs Pte Ltd. All rights reserved.

#include "QuestTreeStyle.h"
#include "Misc/Paths.h"
#include "Styling/SlateStyle.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FQuestTreeStyle::StyleInstance = nullptr;

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define PLUGIN_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FQuestTreeStyle::InResource( RelativePath, ".png" ), __VA_ARGS__ )

void FQuestTreeStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FQuestTreeStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

const ISlateStyle& FQuestTreeStyle::Get()
{
	return *StyleInstance;
}

FName FQuestTreeStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("QuestTreeStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FQuestTreeStyle::Create()
{
	TSharedRef<FStyle> StyleRef = MakeShareable(new FStyle);
	StyleRef->Initialize();

	return StyleRef;
}

FString FQuestTreeStyle::InResource(const FString& RelativePath, const ANSICHAR* Extension)
{
	TSharedPtr<IPlugin> QuestTreePlugin = IPluginManager::Get().FindPlugin("QuestTree");
	check(QuestTreePlugin.IsValid());

	static FString ResourcesDir = QuestTreePlugin->GetBaseDir() / TEXT("/Resources");
	return (ResourcesDir / RelativePath) + Extension;
}

FQuestTreeStyle::FStyle::FStyle() : FSlateStyleSet(FQuestTreeStyle::GetStyleSetName())
{
}

void FQuestTreeStyle::FStyle::Initialize()
{
	const FVector2D Icon26x26(26.0f, 26.0f);

	{
		Set("QuestTree.InputPin", new PLUGIN_IMAGE_BRUSH("InputPin", Icon26x26));
		Set("QuestTree.OutputPin", new PLUGIN_IMAGE_BRUSH("OutputPin", Icon26x26));
		Set("QuestTree.InputPin.Connected", new PLUGIN_IMAGE_BRUSH("InputPinConnected", Icon26x26));
		Set("QuestTree.OutputPin.Connected", new PLUGIN_IMAGE_BRUSH("OutputPinConnected", Icon26x26));
	}

	// Get as SlateIcon
	//FSlateIcon(FQuestTreeStyle::GetStyleSetName(), "QuestTree.InputPin");
	//FSlateIcon(FQuestTreeStyle::GetStyleSetName(), "QuestTree.OutputPin");
	
	// Get as SlateBrush
	//FQuestTreeStyle::Get().GetBrush(TEXT("QuestTree.InputPin"));
	//FQuestTreeStyle::Get().GetBrush(TEXT("QuestTree.OutputPin"));
}

#undef IMAGE_BRUSH
#undef PLUGIN_IMAGE_BRUSH

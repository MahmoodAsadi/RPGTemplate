// Copyright Pixelation Labs Pte Ltd. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * 
 */
class FQuestTreeStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

	class FStyle : public FSlateStyleSet {
	public:
		FStyle();
		void Initialize();

	private:
		FTextBlockStyle NormalText;
	};

private:
	static TSharedRef<class FSlateStyleSet> Create();
	static FString InResource(const FString& RelativePath, const ANSICHAR* Extension);


private:
	static TSharedPtr<class FSlateStyleSet> StyleInstance;

};
//

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"

#include "EditorValidator_AssetName.generated.h"

/**
 *
 */
UCLASS()
class COMMONVALIDATORS_API UEditorValidator_AssetName : public UEditorValidatorBase
{
	GENERATED_BODY()

	virtual bool CanValidateAsset_Implementation(
		const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;

	virtual EDataValidationResult ValidateLoadedAsset_Implementation(
		const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;

	bool IsAssetNameGuidelineConforming(const UObject* InAsset, FDataValidationContext& InContext) const;
	bool CheckAssetPrefix(const UObject* InAsset, FDataValidationContext& InContext) const;
	bool HandleSpecialCases(const UClass* Class, bool& bOutIsSpecialCase) const;

	bool IsMatchingFixes(const FString& AssetName, const UClass* Class, FDataValidationContext& InContext) const;
};
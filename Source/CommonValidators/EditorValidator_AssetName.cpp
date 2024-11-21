//

#include "EditorValidator_AssetName.h"

#include "Misc/DataValidation.h"

bool UEditorValidator_AssetName::CanValidateAsset_Implementation(
	const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject != nullptr;
}

EDataValidationResult UEditorValidator_AssetName::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	if (IsAssetNameGuidelineConforming(InAsset, Context))
	{
		return EDataValidationResult::Valid;
	}

	return EDataValidationResult::Invalid;
}

bool UEditorValidator_AssetName::IsAssetNameGuidelineConforming(const UObject* InAsset, FDataValidationContext& InContext) const
{
	auto AssetName = InAsset->GetName();

	auto ThrowError = [&](const FString& ErrorMessage)
	{
		const FText Output = FText::Join(FText::FromString(" "), FText::FromString(ErrorMessage));
		InContext.AddError(Output);
	};

	const FString RegexPattern = TEXT("^[A-Z](?:[^_]*_(?=[A-Z0-9])[^_]*)*[^_]*$");
	const FRegexPattern Pattern(RegexPattern);
	FRegexMatcher Matcher(Pattern, AssetName);

	if (!Matcher.FindNext())
	{
		ThrowError(
			"The asset is not following the naming conventions. All underscores must be followed by an upper case letter or a "
			"digit. Also, all assets must start with an upper case letter.");
		return false;
	}

	return true;
}
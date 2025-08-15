//

#include "EditorValidator_AssetName.h"

#include "Misc/DataValidation.h"
#include "ValidationSettings.h"

#if WITH_EDITOR
#define LOCTEXT_NAMESPACE "UMG"

bool UEditorValidator_AssetName::CanValidateAsset_Implementation(
	const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	// Only consider assets that are part of our game
	if (const FString GameName = FString("/Game/") + FApp::GetProjectName(); InObject->GetPathName().Contains(GameName))
	{
		return InObject != nullptr;
	}

	return false;
}

EDataValidationResult UEditorValidator_AssetName::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	return EDataValidationResult::Valid;

	// if (IsAssetNameGuidelineConforming(InAsset, Context))
	// {
	// 	return EDataValidationResult::Valid;
	// }

	// Provide quick fix
	// @todo The implementation is rather tricky as we can't really tell if a prefix is there but just slightly incorrect.
	// Additionally, UE doesn't allow us to rename if all that changes is the case of a letter. An intermediate step also doesn't
	// work, and it creates a redirector, so it's error-prone.
	// {
	// 	TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(
	// 		EMessageSeverity::Error, LOCTEXT("FoundNamingConventionIssues", "Found issues with asset naming conventions."));
	// 	Message->AddToken(UE::DataValidation::MakeFix(
	// 		[this, InAsset, &DesiredName]
	// 		{
	// 			FString AssetName = InAsset->GetName();
	// 			for (int32 Index = 0; Index < AssetName.Len(); Index++)
	// 			{
	// 				if (const TCHAR Underscore = AssetName[Index]; Underscore == TEXT('_'))
	// 				{
	// 					if (AssetName.IsValidIndex(Index + 1))
	// 					{
	// 						if (TChar<TCHAR>::IsAlpha(AssetName[Index + 1]))
	// 						{
	// 							AssetName[Index + 1] = TChar<TCHAR>::ToUpper(AssetName[Index + 1]);
	// 						}
	// 					}
	// 				}
	// 			}
	//
	// 			FString DestinationAssetPath = InAsset->GetPathName();
	// 			DestinationAssetPath.ReplaceInline(*InAsset->GetName(), *AssetName, ESearchCase::CaseSensitive);
	//
	// 			if (UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	// 				EditorAssetSubsystem->RenameAsset(InAsset->GetPathName(), DestinationAssetPath))
	// 			{
	// 				return FFixResult::Success(LOCTEXT("NamingConventionIssuesFixed", "Asset has been renamed."));
	// 			}
	//
	// 			return FFixResult::Failure(
	// 				LOCTEXT("NamingConventionIssuesNotFixed", "Asset could not be renamed. Check console for more information."));
	// 		})->CreateToken(LOCTEXT("FixNamingConventionIssues", "Fix all naming convention issues.")));
	//
	// 	Context.AddMessage(MoveTemp(Message));
	// }

	// return EDataValidationResult::Invalid;
}

bool UEditorValidator_AssetName::IsAssetNameGuidelineConforming(const UObject* InAsset, FDataValidationContext& InContext) const
{
	bool bResult = false;

	// Check prefix and postfix
	if (FAssetNameFix AssetNameFix; CheckAssetPrefix(InAsset, InContext))
	{
		bResult = true;
	}

	// Check upper case letter after underscores
	const FString RegexPattern = TEXT("^[A-Z](?:[^_]*_(?=[A-Z0-9])[^_]*)*[^_]*$");
	const FRegexPattern Pattern(RegexPattern);
	if (FRegexMatcher Matcher(Pattern, InAsset->GetName()); !Matcher.FindNext())
	{
		const FText Output = FText::Join(FText::FromString(" "),
			FText::FromString(
				"The asset is not following the naming conventions. All underscores must be followed by an upper case letter or a "
				"digit. Also, all assets must start with an upper case letter."));
		InContext.AddError(Output);

		bResult = false;
	}

	return bResult;
}

bool UEditorValidator_AssetName::CheckAssetPrefix(const UObject* InAsset, FDataValidationContext& InContext) const
{
	const UClass* CurrentClass = InAsset->GetClass();
	const UClass* RealAssetClass = InAsset->GetClass();

	// Blueprint classes need to be handled separately to extract the real C++ base class
	const UBlueprint* Blueprint = Cast<UBlueprint>(InAsset);
	if (Blueprint != nullptr)
	{
		CurrentClass = Blueprint->ParentClass;
		RealAssetClass = Blueprint->ParentClass;
	}

	while (CurrentClass != nullptr)
	{
		if (IsMatchingFixes(InAsset->GetName(), CurrentClass, InContext))
		{
			return true;
		}

		CurrentClass = CurrentClass->GetSuperClass();
	}

	// If the real class of the BP (i.e. the first parent class defined in C++ and none of its own parent classes) does not have a
	// fix rule defined, then fall back to the generic Blueprint type, assuming there is a rule defined for it.
	if (Blueprint != nullptr)
	{
		if (IsMatchingFixes(InAsset->GetName(), InAsset->GetClass(), InContext))
		{
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No prefix convention defined for assets of type %s"), *RealAssetClass->GetName());
	return false;
}

bool UEditorValidator_AssetName::IsMatchingFixes(
	const FString& AssetName, const UClass* Class, FDataValidationContext& InContext) const
{
	const UValidationSettings* ValidationSettings = GetDefault<UValidationSettings>();
	check(ValidationSettings != nullptr);

	if (const FAssetNameFix* Result = ValidationSettings->AssetPrefixRules.Find(Class); Result != nullptr)
	{
		if (!Result->Prefix.IsEmpty())
		{
			// Special case: DEPRECATED_ pre-prefix
			int32 Index = 0;
			if (AssetName.StartsWith("DEPRECATED_"))
			{
				Index = FString("DEPRECATED_").Len();
			}

			// Special case: NONCOMMERCIAL_ pre-prefix
			if (AssetName.StartsWith("NONCOMMERCIAL_"))
			{
				Index = FString("NONCOMMERCIAL_").Len();
			}

			// Skip any special case prefixes for the actual fix check
			if (AssetName.Mid(Index, AssetName.Len() - 1).StartsWith(*Result->Prefix))
			{
				// @note Assumes that every asset with a postfix must also have a prefix
				if (!Result->Postfix.IsEmpty())
				{
					if (AssetName.EndsWith(*Result->Postfix))
					{
						return true;
					}

					const FText Output = FText::Format(
						FText::FromString(TEXT("The asset is not following the naming conventions. All assets of type '{0}' "
											   "must end with the postfix '{1}'")),
						FText::FromString(Class->GetName()), FText::FromString(*Result->Postfix));

					InContext.AddError(Output);
					return false;
				}

				return true;
			}

			const FText Output =
				FText::Format(FText::FromString(TEXT("The asset is not following the naming conventions. All assets of type '{0}' "
													 "must start with the prefix '{1}'")),
					FText::FromString(Class->GetName()), FText::FromString(*Result->Prefix));

			InContext.AddError(Output);
			return false;
		}

		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
#endif
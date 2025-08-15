//

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "ValidationSettings.generated.h"

USTRUCT(BlueprintType)
struct FAssetNameFix
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Prefix;

	UPROPERTY(EditAnywhere)
	FString Postfix;
};

/**
 *
 */
UCLASS(config = Editor, DefaultConfig, meta = (DisplayName = "Validation Settings"))
class COMMONVALIDATORS_API UValidationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Asset Validation", Meta = (AllowAbstract = "true"))
	TMap<TSoftClassPtr<UObject>, FAssetNameFix> AssetPrefixRules;
};

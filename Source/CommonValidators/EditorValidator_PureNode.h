#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "EditorValidator_PureNode.generated.h"

/**
 * 
 */
UCLASS()
class COMMONVALIDATORS_API UEditorValidator_PureNode : public UEditorValidatorBase
{
	GENERATED_BODY()

	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;

	bool IsMultiPinPureNode(const class UK2Node* PureNode) const;
	void CountNonPureNodesRecursively(const UEdGraphNode*GraphNode, uint32_t&Count,TArray<const UK2Node*>& VisitedNodes) const;
	bool IsNodeValidInShipping(const UEdGraphNode*Node) const;
};
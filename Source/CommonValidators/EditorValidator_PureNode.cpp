#include "EditorValidator_PureNode.h"

#include "Engine/Blueprint.h"
#include "Misc/DataValidation.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "K2Node.h"
#include "K2Node_Variable.h"

bool UEditorValidator_PureNode::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UBlueprint>();
}

EDataValidationResult UEditorValidator_PureNode::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(InAsset);
	if (!Blueprint)
	{
		return EDataValidationResult::NotValidated;
	}

	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			UK2Node* PureNode = Cast<UK2Node>(Node);
			if (PureNode && PureNode->IsNodePure())
			{
				if (IsMultiPinPureNode(PureNode))
				{
					FText Output = FText::Join(FText::FromString(" "), PureNode->GetNodeTitle(ENodeTitleType::Type::MenuTitle), FText::FromString(TEXT("MultiPin Pure Nodes actually get called for each connected pin output.")));
					Context.AddWarning(Output);
					return EDataValidationResult::Invalid;
				}
			}
		}
	}

	return EDataValidationResult::Valid;
}

bool UEditorValidator_PureNode::IsMultiPinPureNode(const UK2Node* PureNode) const
{
	// Skip if the note to test is a variable or is development only, but intentionally do not skip composite nodes.
	// We don't want the validator to complain when all we do is print some debugging information that will never execute in a shipping build anyways.
	const UK2Node* KismetNode = Cast<UK2Node>(PureNode);
	if (KismetNode->IsA<UK2Node_Variable>() || !IsNodeValidInShipping(KismetNode))
	{
		return false;	
	}

	// Recursively trace node connection and only consider non-pure nodes that are going to be valid in shipping builds.
	// Keep a list to avoid incrementing the counter in situations where a break vector's X and Y feed into a make vector's X and Y would increment twice.
	uint32_t Count = 0;
	TArray<const UK2Node*> VisitedNodes;
	CountNonPureNodesRecursively(PureNode, Count,VisitedNodes);

	return Count > 1;
}

bool UEditorValidator_PureNode::IsNodeValidInShipping(const UEdGraphNode* Node) const
{
    if (Node)
    {
    	return Node->GetDesiredEnabledState() == ENodeEnabledState::Enabled;
    }
	
    return false;
}

void UEditorValidator_PureNode::CountNonPureNodesRecursively(const UEdGraphNode* GraphNode, uint32_t& Count,TArray<const UK2Node*>& VisitedNodes) const
{
	if (GraphNode != nullptr)
	{
		const UK2Node* KismetNode = Cast<UK2Node>(GraphNode);
		if (KismetNode != nullptr)
		{
			if (KismetNode->IsNodePure())
			{
				for (const UEdGraphPin* Pin : KismetNode->Pins)
				{
					if (Pin->Direction == EGPD_Output && !Pin->LinkedTo.IsEmpty())
					{
						for (const UEdGraphPin* LinkedPin : Pin->LinkedTo)
						{
							const UK2Node* LinkedNode = Cast<UK2Node>(LinkedPin->GetOwningNode());
							if (LinkedNode->IsNodePure())
							{
								CountNonPureNodesRecursively(LinkedNode, Count,VisitedNodes);
							}
							else
							{
								if (IsNodeValidInShipping(LinkedNode))
								{
									if (!VisitedNodes.Contains(LinkedNode))
									{
										VisitedNodes.Add(LinkedNode);
										++Count;
									}
								}
							}
						}
					}
				}
			}
			else
			{
				if (IsNodeValidInShipping(KismetNode))
				{
					if (!VisitedNodes.Contains(KismetNode))
					{
						VisitedNodes.Add(KismetNode);
						++Count;
					}
				}
			}
		}
	}
}

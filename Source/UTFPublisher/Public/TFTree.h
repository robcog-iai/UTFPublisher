// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "TFNodeComp.h"
#include "TFTree.generated.h"

/**
* FTFTree - TF Tree, containing:
*
*  - pointer to the root node
*/
USTRUCT(BlueprintType)
struct UTFPUBLISHER_API FTFTree
{
	GENERATED_BODY()

	// Default constructor
	FTFTree() { }

    // Constructor with root node
	FTFTree(UObject* InObject, const FString& InFrameId) : Root(nullptr)
	{
		UTFNodeComp* NewTFNode = NewObject<UTFNodeComp>(InObject);
		NewTFNode->RegisterComponent();
		NewTFNode->Init(InObject, InFrameId);
		AddRoot(NewTFNode);
	}

	// Constructor with root node
	FTFTree(UTFNodeComp* InNode) : Root(nullptr)
	{
		AddRoot(InNode);
	}

	// Add tree root
	void AddRoot(UTFNodeComp* InNode)
	{
		if (Root != nullptr)
		{
			// Remove all currently existing nodes
			// TODO
		}
		Root = InNode;

	}

	// Add node to parent
	void AddNode(UObject* InObject, const FString& InFrameId, UTFNodeComp* ParentNode)
	{
		UTFNodeComp* NewTFNode = NewObject<UTFNodeComp>(InObject);
		NewTFNode->RegisterComponent();
		NewTFNode->Init(InObject, InFrameId);
		AddNode(NewTFNode, ParentNode);
	}

	// Add node to parent
	void AddNode(UTFNodeComp* InNode, UTFNodeComp* ParentNode)
	{
		ParentNode->AddChild(InNode);
	}

	// Find Node
	bool FindNode(const FString& InFrameId, UTFNodeComp*& OutNode)
	{
		if (Root == nullptr)
		{
			return false; // Tree is empty
		}

		if (Root->FrameId.Equals(InFrameId))
		{
			OutNode = Root;
			return true;
		}
		else
		{
			TArray<UTFNodeComp*> Stack;
			Stack.Push(Root);

			while (Stack.Num() > 0)
			{
				UTFNodeComp* CurrNode = Stack.Pop();
				if (CurrNode->FrameId.Equals(InFrameId))
				{
					OutNode = CurrNode;
					return true;
				}
				for (const auto& ChildItr : CurrNode->Children)
				{
					Stack.Push(ChildItr);
				}
			}
			return false; // Node not found
		}
	}

	FORCEINLINE bool operator==(const FTFTree& Other) const
	{
		return Root == Other.Root;
	}

private:
	// Root node
	UTFNodeComp* Root;	

	// Give access to private data 
	friend struct FTFWorldTree;
};

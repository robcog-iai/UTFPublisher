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

private:
	// Root node
	UTFNodeComp* Root;

	

	// Give access to private data 
	friend struct FTFWorldTree;
};

// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFWorldTree.h"

// Default constructor
UTFWorldTree::UTFWorldTree()
{
}

// Destructor
UTFWorldTree::~UTFWorldTree()
{
}

// Create and add node to the world tf tree
bool UTFWorldTree::AddNode(UObject* InObject, const FString& InFrameId, const FString& InParentFrameId, UObject* InOuter)
{
	// Create and initialize new tf node
	UTFNode* NewNode = NewObject<UTFNode>(this);
	NewNode->Init(InObject, InFrameId);

	// Add the created node to the world tree
	return AddNode(NewNode, InParentFrameId);
}

// Add node to the world tf tree (by default root parent frame)
bool UTFWorldTree::AddNode(UTFNode* InNode, const FString& InParentFrameId)
{
	if (InParentFrameId.Equals(TEXT("World")))
	{
		// Create a new tf tree
		UTFTree* NewTree = NewObject<UTFTree>(this);
		NewTree->AddRoot(InNode);
		TFTrees.Emplace(NewTree);
		return true;
	}
	else
	{
		// Search for the parent node in every tree
		for (auto& TreeItr : TFTrees)
		{
			if (TreeItr->AddNodeAt(InNode, InParentFrameId))
			{
				return true;
			}
		}
	}
	return false;
}

// Get all tf nodes as array
void UTFWorldTree::GetNodesAsArray(TArray<UTFNode*>& OutNodes) const
{
	OutNodes.Empty();
	// Collect nodes from every tree
	for (auto& TreeItr : TFTrees)
	{
		TreeItr->AddNodesToArray(OutNodes);
	}
}

// String output of the tf trees
FString UTFWorldTree::ToString() const
{
	FString TreesString;
	uint32 Index = 0;
	for (const auto& TreeItr : TFTrees)
	{
		TreesString.Append(FString::Printf(TEXT(" \n TF Tree %i: \n"), Index));
		TreesString.Append(TreeItr->ToString());
		Index++;
	}
	return TreesString;
}

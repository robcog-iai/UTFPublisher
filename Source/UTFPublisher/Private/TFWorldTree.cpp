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
	UE_LOG(LogTF, Error, TEXT("[%s]"), *FString(__FUNCTION__));
	// Set default root frame name
	RootFrameName = TEXT("world");
}

// Set root frame name
void UTFWorldTree::SetRootFrameName(const FString& InRootFrameName)
{
	RootFrameName = InRootFrameName;
}

// Create and add node to the world tf tree
bool UTFWorldTree::AddNode(UObject* InObject, const FString& InFrameId, const FString& InParentFrameId)
{
	if (InParentFrameId.Equals(RootFrameName))
	{
		// Create and initialize new tf node
		UTFNode* NewNode = NewObject<UTFNode>(this);
		NewNode->Init(InObject, InFrameId);

		// Create a new tf tree
		AddTree(NewNode);
		// Add node to array for convenient iteration
		TFNodesAsArray.Emplace(NewNode);
		return true;
	}
	else
	{
		// Search for the parent node in every tree
		for (auto& TreeItr : TFTrees)
		{
			if (UTFNode* ParentNode = TreeItr->FindNode(InParentFrameId))
			{
				// Create and initialize new tf node
				UTFNode* NewNode = NewObject<UTFNode>(this);
				NewNode->Init(InObject, InFrameId);
				// Add node as child
				ParentNode->AddChild(NewNode);
				// Add node to array for convenient iteration
				TFNodesAsArray.Emplace(NewNode);
				return true;
			}
		}
	}
	return false;
}

// Create and add new tree
void UTFWorldTree::AddTree(UTFNode* InRoot)
{
	// Create a new tf tree
	UTFTree* NewTree = NewObject<UTFTree>(this);
	// Add node to tree
	NewTree->AddRoot(InRoot);
	// Add tree to TF World Trees
	TFTrees.Emplace(NewTree);
}

// Find and remove node from tree
void UTFWorldTree::RemoveNode(UTFNode* InNode)
{
	// Search for the parent node in every tree
	for (auto& TreeItr : TFTrees)
	{
		if (TreeItr->HasNode(InNode))
		{
			TreeItr->RemoveNode(InNode);
			return;
		}
	}
}

// Remove root node (generate new trees from the children)
void UTFWorldTree::RemoveRootNode(UTFTree* InTFTree)
{
	// Iterate children and add them as new trees
	for (auto& ChildItr : InTFTree->Root->Children)
	{
		// Add new tree
		AddTree(ChildItr);
	}
	// Remove old tree from array
	TFTrees.Remove(InTFTree);
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

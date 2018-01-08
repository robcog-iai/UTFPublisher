// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFTree.h"
#include "TFWorldTree.h"

// Default constructor
UTFTree::UTFTree() : Root(nullptr)
{
}

// Destructor
UTFTree::~UTFTree()
{
	UE_LOG(LogTF, Error, TEXT("[%s]"), *FString(__FUNCTION__));
}

// Find Node
UTFNode* UTFTree::FindNode(const FString& InFrameId)
{
	if (Root != nullptr)
	{
		if (Root->ChildFrameId.Equals(InFrameId))
		{
			return Root;
		}
		else
		{
			TArray<UTFNode*> Stack;
			Stack.Push(Root);

			while (Stack.Num() > 0)
			{
				UTFNode* CurrNode = Stack.Pop();
				if (CurrNode->ChildFrameId.Equals(InFrameId))
				{
					return CurrNode;
				}
				for (const auto& ChildItr : CurrNode->Children)
				{
					Stack.Push(ChildItr);
				}
			}
		}
	}
	return nullptr; // Tree is empty
}

// Check if tree has node
bool UTFTree::HasNode(UTFNode* InNode)
{
	if (Root != nullptr)
	{
		if (Root == InNode)
		{
			return true;
		}
		else
		{
			TArray<UTFNode*> Stack;
			Stack.Push(Root);

			while (Stack.Num() > 0)
			{
				UTFNode* CurrNode = Stack.Pop();
				if (CurrNode == InNode)
				{
					return true;
				}
				for (const auto& ChildItr : CurrNode->Children)
				{
					Stack.Push(ChildItr);
				}
			}
		}
	}
	return false;
}

// Add tree root
bool UTFTree::AddRoot(UTFNode* InNode)
{
	if (Root == nullptr)
	{
		Root = InNode;
		return true;
	}
	return false; // Root already exists
}

// Add Node at the given frame
bool UTFTree::AddNodeAt(UTFNode* InNode, const FString& InFrameId)
{
	UTFNode* ParentNode = FindNode(InFrameId);
	if (ParentNode != nullptr)
	{
		ParentNode->AddChild(InNode);
		return true;
	}
	return false; // Frame id was not found
}

// Remove only the node, connect its children to its parent
void UTFTree::RemoveNode(UTFNode* InNode)
{
	UE_LOG(LogTF, Error, TEXT("[%s] Before removing: %s, tree is:\n %s"),
		*FString(__FUNCTION__), *InNode->GetName(), *ToString());
	if (InNode->Parent != nullptr)
	{
		// Remove node from array
		if (auto TFWorldTree = Cast<UTFWorldTree>(GetOuter()))
		{
			UE_LOG(LogTF, Error, TEXT("[%s] Before Num=%i %i nodes removed from array, After Num=%i"),
				*FString(__FUNCTION__), TFWorldTree->TFNodesAsArray.Num(),
				TFWorldTree->TFNodesAsArray.Remove(InNode),
				TFWorldTree->TFNodesAsArray.Num());
		}

		// Node is not root
		for (auto& ChildItr : InNode->Children)
		{
			ChildItr->Parent = InNode->Parent;
			UE_LOG(LogTF, Error, TEXT("[%s] Node %s is child, its child parent %s now points to %s "),
				*FString(__FUNCTION__), *InNode->GetName(), *ChildItr->GetName(), *ChildItr->Parent->GetName());
		}
		InNode->Children.Empty();
		InNode->Parent->Children.Remove(InNode);
	}
	else
	{
		// Node is root, create TF trees from children
		if (auto TFWorldTree = Cast<UTFWorldTree>(GetOuter()))
		{
			UE_LOG(LogTF, Error, TEXT("[%s] %i nodes removed from array"),
				*FString(__FUNCTION__), TFWorldTree->TFNodesAsArray.Remove(InNode));
			TFWorldTree->RemoveRootNode(this);
			UE_LOG(LogTF, Error, TEXT("[%s] Node %s is root, removing tree %s from %s "),
				*FString(__FUNCTION__), *InNode->GetName(), *GetName(), *GetOuter()->GetName());
		}
	}
	UE_LOG(LogTF, Error, TEXT("[%s] After removing: %s, tree is:\n %s"),
		*FString(__FUNCTION__), *InNode->GetName(), *ToString());
}

// Get all tf nodes as array
void UTFTree::AddNodesToArray(TArray<UTFNode*>& OutNodes) const
{
	if (Root != nullptr)
	{
		TArray<UTFNode*> Stack;
		Stack.Push(Root);

		while (Stack.Num() > 0)
		{
			UTFNode* CurrNode = Stack.Pop();
			OutNodes.Emplace(CurrNode);
			for (const auto& ChildItr : CurrNode->Children)
			{
				Stack.Push(ChildItr);
			}
		}
	}
}

// String output of the tf tree
FString UTFTree::ToString() const
{
	FString TreeString;
	if (Root != nullptr)
	{
		TArray<UTFNode*> Stack;
		Stack.Push(Root);
		while (Stack.Num() > 0)
		{
			UTFNode* CurrNode = Stack.Pop();
			TreeString.Append(CurrNode->ToString());
			for (const auto& ChildItr : CurrNode->Children)
			{
				Stack.Push(ChildItr);
			}
		}
	}
	return TreeString;
}



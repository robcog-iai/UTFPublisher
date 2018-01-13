// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "TagStatics.h"
#include "TFTree.h"
#include "TFNodeComp.h"
#include "TFWorldTree.generated.h"

/**
 * UTFWorldTree - TF Trees connected to 'world' 
 *  ('world' does not have an FTransform, so it cannot be used as a node)
 * 
 *  - name of root frame (world, map etc)
 *  - array of tf trees
 */
USTRUCT(BlueprintType)
struct UTFPUBLISHER_API FTFWorldTree
{
	GENERATED_BODY()

	// Array of all nodes for convenient iteration (since order is not important)
	UPROPERTY()
	TArray<UTFNodeComp*> TFNodes;

	// Initialize world tree
	void Build(UWorld* InWorld, const FString& InTFRootFrameName)
	{
		// Set the tree root frame
		RootFrameName = InTFRootFrameName;

		// Get all objects with TF tags
		auto ObjToTagData = FTagStatics::GetObjectsToKeyValuePairs(InWorld, TEXT("TF"));

		// Set initial root nodes (nodes that have no parent, or the parent equals the root frame name)
		AddInitRootNodes(ObjToTagData);

		// Search and add child nodes (O n^2 complexity)
		AddChildNodes(ObjToTagData);

		// Add orphan nodes as new root trees
		AddOrphanNodes(ObjToTagData);

		// Check if all TF tagged objects have been added to the world tree
		if (ObjToTagData.Num() == 0)
		{
			UE_LOG(LogTF, Log, TEXT("[%s] All objects successfully added as TF nodes"), *FString(__FUNCTION__));
		}
		else
		{
			UE_LOG(LogTF, Error, TEXT("[%s] %i objects could not be added to the TF World Tree"),
				*FString(__FUNCTION__), ObjToTagData.Num());
		}
	}

private:
	//  Set initial root nodes (nodes that have no parent, or the parent equals the root frame name)
	void AddInitRootNodes(TMap<UObject*, TMap<FString, FString>>& ObjectsToTagData)
	{
		// Iterate map and add initial root nodes
		for (auto MapItr(ObjectsToTagData.CreateIterator()); MapItr; ++MapItr)
		{
			// Set default values to child and parent frames
			FString ChildFrameId = MapItr->Key->GetName();
			FString ParentFrameId = RootFrameName;

			// Set child frame id from tag
			if (MapItr->Value.Contains(TEXT("ChildFrameId")))
			{
				ChildFrameId = MapItr->Value["ChildFrameId"];
			}
			// Set parent frame id from tag
			if (MapItr->Value.Contains(TEXT("ParentFrameId")))
			{
				ParentFrameId = MapItr->Value["ParentFrameId"];
			}

			// Create new tree if root node
			if (ParentFrameId.Equals(RootFrameName))
			{
				AddNewTree(MapItr->Key, ChildFrameId);
				// Remove element from map
				MapItr.RemoveCurrent();
			}
		}
	}

	// Search and add child nodes (O n^2 complexity)
	void AddChildNodes(TMap<UObject*, TMap<FString, FString>>& ObjectsToTagData)
	{
		// Try adding objects to the tf tree until no more objects in the map
		// and the map size has changed
		bool bMapSizeChanged = true;
		while (ObjectsToTagData.Num() > 0 && bMapSizeChanged)
		{
			int32 MapSize = ObjectsToTagData.Num();
			// Iterate map and try adding objects to tree
			for (auto MapItr(ObjectsToTagData.CreateIterator()); MapItr; ++MapItr)
			{
				// Frame Ids default values
				FString ChildFrameId = MapItr->Key->GetName();
				FString ParentFrameId = RootFrameName;
		
				// Set child frame id from tag
				if (MapItr->Value.Contains(TEXT("ChildFrameId")))
				{
					ChildFrameId = MapItr->Value["ChildFrameId"];
				}
				// Set parent frame id from tag
				if (MapItr->Value.Contains(TEXT("ParentFrameId")))
				{
					ParentFrameId = MapItr->Value["ParentFrameId"];
				}
		
				// Try to add node to world tree
				if (AddNode(MapItr->Key, ChildFrameId, ParentFrameId))
				{
					MapItr.RemoveCurrent();
				}
			}
		
			// Check if the map size has changed
			if (MapSize == ObjectsToTagData.Num())
			{
				bMapSizeChanged = false;
			}
		}
	}

	// Search and add child nodes (O n^2 complexity)
	void AddOrphanNodes(TMap<UObject*, TMap<FString, FString>>& ObjectsToTagData)
	{
		// Iterate map and add initial root nodes
		for (auto MapItr(ObjectsToTagData.CreateIterator()); MapItr; ++MapItr)
		{
			// Set default values to child and parent frames
			FString ChildFrameId = MapItr->Key->GetName();
			FString ParentFrameId = RootFrameName;

			// Set child frame id from tag
			if (MapItr->Value.Contains(TEXT("ChildFrameId")))
			{
				ChildFrameId = MapItr->Value["ChildFrameId"];
			}

			// Create new tree if root node
			if (ParentFrameId.Equals(RootFrameName))
			{
				AddNewTree(MapItr->Key, ChildFrameId);
				// Remove element from map
				MapItr.RemoveCurrent();
			}
		}
	}
	
	// Add root node
	void AddNewTree(UObject* InObject, const FString& InFrameId)
	{
		// Create new node
		UTFNodeComp* NewTFNode = NewObject<UTFNodeComp>(InObject);
		NewTFNode->RegisterComponent();
		NewTFNode->Init(InObject, InFrameId);

		// Create tree with root
		TFTrees.Emplace(FTFTree(NewTFNode));

		// Add node to array
		TFNodes.Emplace(NewTFNode);

		UE_LOG(LogTF, Error, TEXT("[%s] New tree, with root node: (%s)%s, total trees: %i"),
			*FString(__FUNCTION__), *InObject->GetName(), *InFrameId, TFTrees.Num());
	}

	// Find node // TODO use references to avoid nullptr, so no extra checks need to be made.
	bool AddNode(UObject* InObject, const FString& InFrameId, const FString& InParentFrameId)
	{
		// Try to add node to tree
		FTFTree* ParentTree = nullptr;
		UTFNodeComp* ParentNode = nullptr;
		if (FindNode(InParentFrameId, ParentTree, ParentNode))
		{
			// Create new node
			UTFNodeComp* NewTFNode = NewObject<UTFNodeComp>(InObject);
			NewTFNode->RegisterComponent();
			NewTFNode->Init(InObject, InFrameId);

			// Add node to parent
			ParentTree->AddNode(NewTFNode, ParentNode);

			// Add node to array
			TFNodes.Emplace(NewTFNode);
			return true;
		}
		return false;
	}

	// Find node // TODO use references to avoid nullptr, so no extra checks need to be made.
	bool FindNode(const FString& InParentFrameId, FTFTree* OutTree, UTFNodeComp* OutNode)
	{
		return false;
	}

	// Root frame name
	FString RootFrameName;

	// Array of TFTrees (since world cannot be a node)
	TArray<FTFTree> TFTrees;
};

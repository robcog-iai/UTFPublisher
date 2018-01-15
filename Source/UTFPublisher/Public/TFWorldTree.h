// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "TagStatics.h"
#include "TFTree.h"
#include "TFNodeComp.h"
#include "tf2_msgs/TFMessage.h"
#include "CoordConvStatics.h"
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
		AddInitRootNodes(&ObjToTagData);

		// Search and add child nodes (O n^2 complexity)
		AddChildNodes(&ObjToTagData);
	
		// Add orphan nodes as new root trees
		AddOrphanNodes(&ObjToTagData);
	}

	// Get TF Message of the tree
	TSharedPtr<tf2_msgs::TFMessage> GetAsTFMessage(const FROSTime& InTime, const uint32 InSeq = 0)
	{
		// Create TFMessage
		TSharedPtr<tf2_msgs::TFMessage> TFMsgPtr =
			MakeShareable(new tf2_msgs::TFMessage());
		for (const auto& NodeItr : TFNodes)
		{
			TFMsgPtr->AddTransform(TFNodeToMsg(NodeItr, InTime, InSeq));
		}
		return TFMsgPtr;
	}

private:
	//  Set initial root nodes (nodes that have no parent, or the parent equals the root frame name)
	void AddInitRootNodes(TMap<UObject*, TMap<FString, FString>>* ObjectsToTagData)
	{
		// Iterate map and add initial root nodes
		for (auto MapItr(ObjectsToTagData->CreateIterator()); MapItr; ++MapItr)
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
				UE_LOG(LogTF, Warning, TEXT("[%s][%i] Add init root node %s"),
					*FString(__FUNCTION__), __LINE__, *ChildFrameId);

				AddNewTree(MapItr->Key, ChildFrameId);
				// Remove element from map
				MapItr.RemoveCurrent();
			}
		}
	}

	// Search and add child nodes (O n^2 complexity)
	void AddChildNodes(TMap<UObject*, TMap<FString, FString>>* ObjectsToTagData)
	{
		// Try adding objects to the tf tree until no more objects in the map
		// and the map size has changed
		bool bMapSizeChanged = true;
		while (ObjectsToTagData->Num() > 0 && bMapSizeChanged)
		{
			int32 MapSize = ObjectsToTagData->Num();
			// Iterate map and try adding objects to tree
			for (auto MapItr(ObjectsToTagData->CreateIterator()); MapItr; ++MapItr)
			{
				// Frame Ids default values
				FString ChildFrameId = MapItr->Key->GetName();
				FString ParentFrameId = RootFrameName;
		
				// If available, set child frame id from tag
				if (MapItr->Value.Contains(TEXT("ChildFrameId")))
				{
					ChildFrameId = MapItr->Value["ChildFrameId"];
				}
				// If available, set parent frame id from tag
				if (MapItr->Value.Contains(TEXT("ParentFrameId")))
				{
					ParentFrameId = MapItr->Value["ParentFrameId"];
				}
		
				// Try to add node to world tree
				if (TryAddNode(MapItr->Key, ChildFrameId, ParentFrameId))
				{
					UE_LOG(LogTF, Warning, TEXT("[%s][%i] Add child node %s"),
						*FString(__FUNCTION__), __LINE__, *ChildFrameId);
					MapItr.RemoveCurrent();
				}
			}
		
			// Check if the map size has changed
			if (MapSize == ObjectsToTagData->Num())
			{
				bMapSizeChanged = false;
			}
		}
	}

	// Add orphan nodes as new root trees
	void AddOrphanNodes(TMap<UObject*, TMap<FString, FString>>* ObjectsToTagData)
	{
		// Iterate map and add initial root nodes
		for (auto MapItr(ObjectsToTagData->CreateIterator()); MapItr; ++MapItr)
		{
			// Set default values to child frames
			FString ChildFrameId = MapItr->Key->GetName();

			// If available, set child frame id from tag
			if (MapItr->Value.Contains(TEXT("ChildFrameId")))
			{
				ChildFrameId = MapItr->Value["ChildFrameId"];
			}
			UE_LOG(LogTF, Error, TEXT("[%s] Add orphan node: %s"),
				*FString(__FUNCTION__), *ChildFrameId);

			AddNewTree(MapItr->Key, ChildFrameId);
			// Remove element from map
			MapItr.RemoveCurrent();
		}
	}
	
	// Add root node
	void AddNewTree(UObject* InObject, const FString& InFrameId)
	{
		// Create new node
		UTFNodeComp* NewTFNode = NewObject<UTFNodeComp>(InObject);
		NewTFNode->RegisterComponent();
		NewTFNode->Init(InObject, InFrameId, this);

		// Create tree with root
		TFTrees.Emplace(FTFTree(NewTFNode));

		// Add node to array
		TFNodes.Emplace(NewTFNode);

		UE_LOG(LogTF, Error, TEXT("[%s] New tree, with root node: (%s)%s, total trees: %i"),
			*FString(__FUNCTION__), *InObject->GetName(), *InFrameId, TFTrees.Num());
	}

	// Try to add node 
	bool TryAddNode(UObject* InObject, const FString& InFrameId, const FString& InParentFrameId)
	{
		// Try to add node to tree
		FTFTree* ResultTree = nullptr;
		UTFNodeComp* ResultNode = nullptr;
		if (FindNode(InParentFrameId, ResultTree, ResultNode))
		{
			// Create new node
			UTFNodeComp* NewTFNode = NewObject<UTFNodeComp>(InObject);
			NewTFNode->RegisterComponent();
			NewTFNode->Init(InObject, InFrameId, this);

			// Add node to parent
			ResultTree->AddNode(NewTFNode, ResultNode);

			// Add node to array
			TFNodes.Emplace(NewTFNode);
			return true;
		}
		return false;
	}

	// Remove node
	void RemoveNode(UTFNodeComp* InNode)
	{		
		// Try to add node to tree
		FTFTree* ResultTree = nullptr;
		UTFNodeComp* ResultNode = nullptr;
		if (FindNode(InNode->FrameId, ResultTree, ResultNode))
		{
			// Node is root, break all children into tf trees
			if (InNode->Parent == nullptr)
			{
				// Remove tree from array
				//TFTrees.Remove(*ResultTree);
				int32 Index;
				if (TFTrees.Find(*ResultTree, Index))
				{
					TFTrees.RemoveAt(Index);
				}

				// Iterate children and add new trees
				for (auto& ChildItr : InNode->Children)
				{
					// Parent of child will be root
					ChildItr->Parent = nullptr;
					TFTrees.Emplace(FTFTree(ChildItr));
				}
			}
			else
			{
				// Remove yourself as child
				InNode->Parent->Children.Remove(InNode);

				// Link children parent to node parent
				for (auto& ChildItr : InNode->Children)
				{
					ChildItr->Parent = InNode->Parent;
				}
			}
		}

		// Remove node from array
		TFNodes.Remove(InNode);
	}

	// Find node
	bool FindNode(const FString& InFrameId, FTFTree*& OutTree, UTFNodeComp*& OutNode)
	{
		// Search for the parent node in every tree
		for (auto& TreeItr : TFTrees)
		{
			if (TreeItr.FindNode(InFrameId, OutNode))
			{
				OutTree = &TreeItr;
				return true;
			}
		}
		return false;
	}

	// TFNodeComp to tf2_msgs::TFMessage
	geometry_msgs::TransformStamped TFNodeToMsg(UTFNodeComp* InNode, const FROSTime& InTime, const uint32 InSeq = 0)
	{
		geometry_msgs::TransformStamped StampedTransformMsg;

		std_msgs::Header Header;
		Header.SetSeq(InSeq);
		const FString ParentFrameId = InNode->Parent != nullptr ?
			InNode->Parent->FrameId : RootFrameName;
		Header.SetFrameId(ParentFrameId);
		Header.SetStamp(InTime);

		// Transform to ROS coordinate system
		FTransform ROSTransf =	FCoordConvStatics::UToROS(InNode->GetRelativeTransform());

		geometry_msgs::Transform TransfMsg(
			geometry_msgs::Vector3(ROSTransf.GetLocation()),
			geometry_msgs::Quaternion(ROSTransf.GetRotation()));

		StampedTransformMsg.SetHeader(Header);
		StampedTransformMsg.SetChildFrameId(InNode->FrameId);
		StampedTransformMsg.SetTransform(TransfMsg);

		return StampedTransformMsg;
	}
	
	// Root frame name
	FString RootFrameName;

	// Array of TFTrees (since world cannot be a node)
	TArray<FTFTree> TFTrees;

	// Give access to private data / functions
	friend struct FTFTree;
	friend class UTFNodeComp;
};

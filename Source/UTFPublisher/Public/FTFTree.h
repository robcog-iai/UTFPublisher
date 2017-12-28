// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include "EngineMinimal.h"
#include "CoreTypes.h"
#include "CoreMinimal.h"

/**
*  FTFData - TF related data, containing:
*
*  - the name of the frame (child)
*  - pointer to an entity (AActor or USceneComponent) for FTransform access
*   for accessing FTransform)
*  - functionality to return the world transform between itself and its parent
*
*/
struct FTFData
{
private:
	// Name of the (child) frame id
	FString FrameId;

	// Base object type to get the transform
	AActor* ActorBaseObject;
	USceneComponent* SceneComponentBaseObject;

	// Get world transform function pointer variable type
	typedef FTransform(FTFData::*GetTransformFuncPtrType)();
	// Function pointer to the get the world transform function
	GetTransformFuncPtrType GetTransformFunctionPtr;

public:
	// Default constructor (no init)
	FTFData() { }

	// Constructor with initialization as UObject and frame id
	FTFData(UObject* InObject, const FString& InFrameId)
	{
		Init(InObject, InFrameId);
	}

	// Constructor with initialization as actor and frame id
	FTFData(AActor* InActor, const FString& InFrameId)
	{
		Init(InActor, InFrameId);
	}

	// Constructor with initialization as scene component and frame id
	FTFData(USceneComponent* InSceneComponent, const FString& InFrameId)
	{
		Init(InSceneComponent, InFrameId);
	}

	// Init data with UObject
	void Init(UObject* InObject, const FString& InFrameId)
	{
		if (InObject->IsA(AActor::StaticClass()))
		{
			Init(Cast<AActor>(InObject), InFrameId);
		}
		else if (InObject->IsA(USceneComponent::StaticClass()))
		{
			Init(Cast<USceneComponent>(InObject), InFrameId);
		}
	}

	// Init data with AActor
	void Init(AActor* InActor, const FString& InFrameId)
	{
		ActorBaseObject = InActor;
		FrameId = InFrameId;
		// Bind the transform function ptr
		GetTransformFunctionPtr = &FTFData::GetTransform_AsActor;
	}

	// Init data with USceneComponent
	void Init(USceneComponent* InSceneComponent, const FString& InFrameId)
	{
		SceneComponentBaseObject = InSceneComponent;
		FrameId = InFrameId;
		// Bind the transform function ptr
		GetTransformFunctionPtr = &FTFData::GetTransform_AsSceneComponent;
	}

	// Get the frame id name
	FString GetFrameId() const
	{
		return FrameId;
	}

	// TODO use const ref
	// Get transform
	FTransform GetTransform()
	{
		return (this->*GetTransformFunctionPtr)();
	}

private:
	// Get the world transform of actor
	FORCEINLINE FTransform GetTransform_AsActor()
	{
		return ActorBaseObject->GetActorTransform();
	}

	// Get the world transform of scene component
	FORCEINLINE FTransform GetTransform_AsSceneComponent()
	{
		return SceneComponentBaseObject->GetComponentTransform();
	}
};

/**
*  FTFNode - Unordered Tree structure for storing/calculating TF data, containing:
*
*  - a pointer to its parent node (nullptr) if it is the root of the tree
*  - an array of its children nodes
*  - functionality to return the transform between itself and its parent
*
*/
struct FTFTreeNode
{
private:
	// Pointer to parent
	FTFTreeNode* Parent;

	// Array of the children subtrees
	TArray<FTFTreeNode> Children;

	// TF data
	FTFData TFData;

	// Name of the (child) frame id
	FString FrameId;

	// Base object type to get the transform
	AActor* ActorBaseObject;
	USceneComponent* SceneComponentBaseObject;

	// Get relative transform function pointer variable type
	typedef FTransform(FTFTreeNode::*RelativeTransformFuncPtrType)();
	// Function pointer to the get the relative transform function
	RelativeTransformFuncPtrType RelativeTransformFunctionPtr;

	// Get world transform function pointer variable type
	typedef FTransform(FTFTreeNode::*RelativeTransformFuncPtrType)();
	// Function pointer to the get the world transform function
	RelativeTransformFuncPtrType WorldTransformFunctionPtr;

public:
	// Default constructor (no init)
	FTFTreeNode() : Parent(nullptr) { }

	// Constructor with initialization
	FTFTreeNode(FTFData InTFData) : TFData(InTFData), Parent(nullptr) { }
	
	// Destructor
	~FTFTreeNode()
	{
		Empty();
	}

	// Empty tree
	void Empty()
	{
		Children.Empty();
		Parent = nullptr;
	}
	
	// Check if tree is root
	FORCEINLINE bool IsRoot() const
	{
		return Parent == nullptr;
	}

	// Check if the tree has children
	FORCEINLINE bool HasChildren() const
	{
		return Children.Num() == 0;
	}
	
	// Set parent
	void SetParent(FTFTreeNode* InParent)
	{
		InParent->AddChild(*this);
		Parent = InParent;
	}
	
	// Get parent
	FTFTreeNode* GetParent() const
	{
		return Parent;
	}
	
	// Get the frame id name
	FString GetFrameId() const
	{
		return TFData.GetFrameId();
	}
	
	// Add child to node
	bool AddChild(FTFTreeNode InChild)
	{
		InChild.SetParent(this);
		Children.Add(InChild);
		return true;
	}
	
	// Get array children
	TArray<FTFTreeNode>& GetChildren()
	{
		return Children;
	}

	// Get the relative transform between the parent and the current item
	FORCEINLINE FTransform GetRelativeTransform()
	{
		// Check if we are root
		if (Parent == nullptr)
		{
			// Return world transform
			return TFData.GetTransform();
		}
		else
		{
			// Return transform relative to parent
			return TFData.GetTransform().GetRelativeTransform(
				Parent->TFData.GetTransform());
		}
	}

	bool FindNode(const FString& InFrameId, FTFTreeNode* OutNode)
	{
		// Check current frame id
		if (GetFrameId().Equals(InFrameId))
		{
			OutNode = this;
			return true;
		}
		else
		{
			// Check every child frame id
			for (auto& ChildItr : GetChildren())
			{
				return ChildItr.FindNode(InFrameId, OutNode);
			}
		}
		return false;
	}
};

/**
*  FTFTree - 
*
*  - Root
*  - functionality to add tf tree nodes to the needed frames
*
*/
struct FTFTree
{
private:
	// The root of the tree
	FTFTreeNode Root;

public:
	// Default constructor
	FTFTree() {}

	// Constructor with root init
	FTFTree(FTFTreeNode InRoot) : Root(InRoot) { }

	// Search and add child to the parent frame
	bool AddNode(FTFTreeNode InNode, const FString& InParentFrameId)
	{
		// Check current parent frame id
		if (Root.GetFrameId().Equals(InParentFrameId))
		{
			Root.AddChild(InNode);
			return true;
		}
		else
		{
			FTFTreeNode* FoundNode;
			if (Root.FindNode(InParentFrameId, FoundNode))
			{
				FoundNode->AddChild(InNode);
				return true;
			}
		}
		return false;
	}
};
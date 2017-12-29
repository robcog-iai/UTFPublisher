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
	FString ChildFrameId;

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
		if (auto AA = Cast<AActor>(InObject))
		{
			Init(AA, InFrameId);
		}
		else if (auto USC = Cast<USceneComponent>(InObject))
		{
			Init(USC, InFrameId);
		}
		//if (InObject->IsA(AActor::StaticClass()))
		//{
		//	Init(Cast<AActor>(InObject), InFrameId);
		//}
		//else if (InObject->IsA(USceneComponent::StaticClass()))
		//{
		//	Init(Cast<USceneComponent>(InObject), InFrameId);
		//}
	}

	// Init data with AActor
	void Init(AActor* InActor, const FString& InFrameId)
	{
		ActorBaseObject = InActor;
		ChildFrameId = InFrameId;
		// Bind the transform function ptr
		GetTransformFunctionPtr = &FTFData::GetTransform_AsActor;
	}

	// Init data with USceneComponent
	void Init(USceneComponent* InSceneComponent, const FString& InFrameId)
	{
		SceneComponentBaseObject = InSceneComponent;
		ChildFrameId = InFrameId;
		// Bind the transform function ptr
		GetTransformFunctionPtr = &FTFData::GetTransform_AsSceneComponent;
	}

	// Get the frame id name
	FString GetFrameId() const
	{
		return ChildFrameId;
	}

	// Get transform
	const FTransform& GetTransform()
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

public:
	// Default constructor (no init)
	FTFTreeNode() : Parent(nullptr) { }

	// Constructor with initialization
	FTFTreeNode(FTFData InTFData) : TFData(InTFData), Parent(nullptr) { }

	// Constructor with initialization as UObject and frame id
	FTFTreeNode(UObject* InObject, const FString& InFrameId) : Parent(nullptr)
	{
		TFData = FTFData(InObject, InFrameId);
	}

	// Constructor with initialization as actor and frame id
	FTFTreeNode(AActor* InActor, const FString& InFrameId) : Parent(nullptr)
	{
		TFData = FTFData(InActor, InFrameId);
	}

	// Constructor with initialization as scene component and frame id
	FTFTreeNode(USceneComponent* InSceneComponent, const FString& InFrameId) : Parent(nullptr)
	{
		TFData = FTFData(InSceneComponent, InFrameId);
	}
	
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
	void AddChild(FTFTreeNode InChild)
	{
		InChild.SetParent(this);
		Children.Add(InChild);
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

	// Find node with the given frame id
	bool FindNode(const FString& InFrameId, FTFTreeNode& OutNode)
	{
		// Check current frame id
		if (GetFrameId().Equals(InFrameId))
		{
			OutNode = *this;
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

	// Counts all elements in the tree
	int32 CountElements()
	{
		return CountAllChildren() + 1; // + the root node
	}

private:
	// Counts all the children in the tree recursively
	int32 CountAllChildren()
	{
		int32 Counter = 0;
		Counter = GetChildren().Num();
		// Check every child frame id
		for (auto& ChildItr : GetChildren())
		{
			Counter += ChildItr.CountAllChildren();
		}
		return Counter;
	}
};

/**
*  FTFTree  
*
*  - Array of root nodes representing a virtual connection with the
*    'world node', all nodes having the world as its parent will be a root node
*  - functionality to add tf tree nodes to the needed frames
*
*/
struct FTFTree
{
private:
	// Array of roots (since world cannot be a node)
	TArray<FTFTreeNode> Roots;

public:
	// Default constructor
	FTFTree() {}

	// Constructor with root init
	FTFTree(const TArray<FTFTreeNode>& InRoots) : Roots(InRoots) { }

	// Search and add child to the parent frame
	bool AddNodeAt(FTFTreeNode InNode, const FString& InParentFrameId)
	{
		if (InParentFrameId.Equals(TEXT("World")))
		{
			// Add as a root (parent is world)
			Roots.Add(InNode);
			return true;
		}
		else
		{
			// Search for the parent frame in every tree node
			for (auto& RootItr : Roots)
			{
				FTFTreeNode FoundNode;
				if (RootItr.FindNode(InParentFrameId, FoundNode))
				{
					FoundNode.AddChild(InNode);
					return true;
				}
			}
		}
		return false;
	}

	// Counts all elements in the tree
	int32 CountElements()
	{
		int32 Count = 0;
		for (auto& RootItr : Roots)
		{
			Count += RootItr.CountElements();
		}
		return Count;
	}
};
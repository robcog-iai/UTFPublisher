// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include "UTFPublisher.h"
#include "TFData.h"

/**
*  FTFData - TF related data, containing:
*
*  - the name of the frame (child)
*  - pointer to an entity (AActor or USceneComponent) for FTransform access
*  - functionality to return the world transform between itself and its parent
*
*/
//struct UTFPUBLISHER_API FTFData
//{
//private:
//	// Name of the (child) frame id
//	FString ChildFrameId;
//
//	// Base object type to get the transform
//	UPROPERTY()
//	AActor* ActorBaseObject;
//	UPROPERTY()
//	USceneComponent* SceneComponentBaseObject;
//
//	// Get world transform function pointer variable type
//	typedef FTransform(FTFData::*GetTransformFuncPtrType)();
//	// Function pointer to the get the world transform function
//	GetTransformFuncPtrType GetTransformFunctionPtr;
//
//public:
//	// Default constructor (no init)
//	FTFData() : ActorBaseObject(nullptr), SceneComponentBaseObject(nullptr)
//	{ }
//
//	// Constructor with initialization as UObject and frame id
//	FTFData(UObject* InObject, const FString& InFrameId) : 
//		ActorBaseObject(nullptr), SceneComponentBaseObject(nullptr)
//	{
//		Init(InObject, InFrameId);
//	}
//
//	// Constructor with initialization as actor and frame id
//	FTFData(AActor* InActor, const FString& InFrameId) :
//		ActorBaseObject(nullptr), SceneComponentBaseObject(nullptr)
//	{
//		Init(InActor, InFrameId);
//	}
//
//	// Constructor with initialization as scene component and frame id
//	FTFData(USceneComponent* InSceneComponent, const FString& InFrameId) :
//		ActorBaseObject(nullptr), SceneComponentBaseObject(nullptr)
//	{
//		Init(InSceneComponent, InFrameId);
//	}
//
//	// Init data with UObject
//	void Init(UObject* InObject, const FString& InFrameId)
//	{
//		if (const auto& AA = Cast<AActor>(InObject))
//		{
//			Init(AA, InFrameId);
//		}
//		else if (const auto& USC = Cast<USceneComponent>(InObject))
//		{
//			Init(USC, InFrameId);
//		}
//	}
//
//	// Init data with AActor
//	void Init(AActor* InActor, const FString& InFrameId)
//	{
//		ActorBaseObject = InActor;
//		ChildFrameId = InFrameId;
//		// Bind the transform function ptr
//		GetTransformFunctionPtr = &FTFData::GetTransform_AsActor;
//		//InActor->OnDestroyed.AddDynamic(this, &FTFData::OnActorDestroyed);
//	}
//
//	// Init data with USceneComponent
//	void Init(USceneComponent* InSceneComponent, const FString& InFrameId)
//	{
//		SceneComponentBaseObject = InSceneComponent;
//		ChildFrameId = InFrameId;
//		// Bind the transform function ptr
//		GetTransformFunctionPtr = &FTFData::GetTransform_AsSceneComponent;
//	}
//
//	// Get the frame id name
//	const FString& GetFrameId() const
//	{
//		return ChildFrameId;
//	}
//
//	// Get transform
//	const FTransform& GetTransform()
//	{
//		return (this->*GetTransformFunctionPtr)();
//	}
//
//	// Output the tf data as string
//	FString ToString() const
//	{
//		FString BaseObjName = TEXT("NONE");
//		if (ActorBaseObject != nullptr && !ActorBaseObject->IsPendingKill())
//		{
//			BaseObjName = ActorBaseObject->GetName();
//		}
//		else if(SceneComponentBaseObject != nullptr && !SceneComponentBaseObject->IsPendingKill())
//		{
//			BaseObjName = SceneComponentBaseObject->GetName();
//		}
//		return FString::Printf(TEXT("ChildFrameId=%s, BaseObjectName=%s"),
//			*ChildFrameId, *BaseObjName);
//	}
//
//private:
//	//UFUNCTION()
//	//void OnActorDestroyed(AActor* DestroyedActor)
//	//{
//	//	UE_LOG(LogTF, Warning, TEXT(" Actor destroyed !!! "));
//	//	ActorBaseObject = nullptr;
//	//}
//
//	// Get the world transform of actor
//	FORCEINLINE FTransform GetTransform_AsActor()
//	{
//		return ActorBaseObject->GetActorTransform();
//	}
//
//	// Get the world transform of scene component
//	FORCEINLINE FTransform GetTransform_AsSceneComponent()
//	{
//		return SceneComponentBaseObject->GetComponentTransform();
//	}
//};

/**
*  FTFTreeNode - Unordered Tree structure for storing/calculating TF data, containing:
*
*  - a pointer to its parent node (nullptr) if it is the root of the tree
*  - an array of its children nodes
*  - functionality to return the transform between itself and its parent
*
*/
struct UTFPUBLISHER_API FTFTreeNode
{
private:
	// Pointer to parent
	FTFTreeNode* Parent;

	// Array of the children subtrees
	TArray<FTFTreeNode> Children;

	// TF data
	UTFData* TFData;

public:
	// Default constructor (no init)
	FTFTreeNode() : Parent(nullptr), TFData(NewObject<UTFData>()) { }

	// Constructor with TFData
	FTFTreeNode(UTFData* InTFData) : Parent(nullptr), TFData(InTFData)	{ }

	// Constructor with initialization as UObject and frame id
	FTFTreeNode(UObject* InObject, const FString& InFrameId, UObject* InOuter = (UObject*)nullptr)
		: Parent(nullptr)
	{
		// Avoid garbage collection of the TF Data by setting its parent as root or the Outer object
		if (InOuter == nullptr)
		{
			TFData = NewObject<UTFData>(/*InObject, FName(*FString("TFData_").Append(InFrameId))*/); // Crashes at gameplay stop if other than default Outer
			TFData->AddToRoot();
		}
		else
		{
			TFData = NewObject<UTFData>(InOuter, FName(*FString("TFData_").Append(InFrameId)));
		}
		TFData->Init(InObject, InFrameId);
	}

	// Constructor with initialization as actor and frame id
	FTFTreeNode(AActor* InActor, const FString& InFrameId, UObject* InOuter = (UObject*)nullptr)
		: Parent(nullptr)
	{
		// Avoid garbage collection of the TF Data by setting its parent as root or the Outer object
		if (InOuter == nullptr)
		{
			TFData = NewObject<UTFData>(/*InActor, FName(*FString("TFData_").Append(InFrameId))*/); // Crashes at gameplay stop if other than default Outer
			TFData->AddToRoot();
		}
		else
		{
			TFData = NewObject<UTFData>(InOuter, FName(*FString("TFData_").Append(InFrameId)));
		}
		TFData->Init(InActor, InFrameId);
	}

	// Constructor with initialization as scene component and frame id
	FTFTreeNode(USceneComponent* InSceneComponent, const FString& InFrameId, UObject* InOuter = (UObject*)nullptr)
		: Parent(nullptr)
	{
		// Avoid garbage collection of the TF Data by setting its parent as root or the Outer object
		if (InOuter == nullptr)
		{
			TFData = NewObject<UTFData>(/*InSceneComponent, FName(*FString("TFData_").Append(InFrameId))*/); // Crashes at gameplay stop if other than default Outer
			TFData->AddToRoot();
		}
		else
		{
			TFData = NewObject<UTFData>(InOuter, FName(*FString("TFData_").Append(InFrameId)));
		}
		TFData->Init(InSceneComponent, InFrameId);
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
	
	// Set parent
	void SetParent(FTFTreeNode* InParent)
	{
		Parent = InParent;
	}
	
	// Get parent
	FORCEINLINE FTFTreeNode* GetParent() const
	{
		return Parent;
	}
		
	// Check if tree has parent
	FORCEINLINE bool HasParent() const
	{
		return Parent != nullptr;
	}

	// Get the tf data
	UTFData* GetTFData() const
	{
		return TFData;
	}
	
	// Get the frame id name
	const FString& GetFrameId() const
	{
		return TFData->GetFrameId();
	}
	
	// Add child to node
	void AddChild(FTFTreeNode& InChild)
	{
		InChild.SetParent(this);
		Children.Emplace(InChild);
	}
	
	// Get array children
	TArray<FTFTreeNode>& GetChildren()
	{
		return Children;
	}

	// Get the relative transform between the parent and the current item
	FORCEINLINE FTransform GetTransform()
	{
		// Check if we are root
		if (Parent == nullptr)
		{
			// Return world transform
			return TFData->GetTransform();
		}
		else
		{
			// Return transform relative to parent
			return TFData->GetTransform().GetRelativeTransform(
				Parent->GetTFData()->GetTransform());
		}
	}

	//// Find node with the given frame id
	//bool GetNode(const FString& InFrameId, FTFTreeNode* OutNode)
	//{
	//	// Check current frame id
	//	if (GetFrameId().Equals(InFrameId))
	//	{
	//		OutNode = this;
	//		return true;
	//	}
	//	else
	//	{
	//		// Check every child frame id
	//		for (auto& ChildItr : Children)
	//		{
	//			return ChildItr.GetNode(InFrameId, OutNode);
	//		}
	//	}
	//	return false;
	//}

	// Find node with the given frame id
	FTFTreeNode* GetNode(const FString& InFrameId)
	{
		// Check current frame id
		if (GetFrameId().Equals(InFrameId))
		{
			return this;
		}
		else
		{
			// Check every child frame id
			for (auto& ChildItr : Children)
			{
				return ChildItr.GetNode(InFrameId);
			}
		}
		return nullptr;
	}

	// Depth first traversal pre-order
	void DFSTraversalPre(FTFTreeNode& InNode)
	{
		UE_LOG(LogTF, Warning, TEXT("\t %s"), *InNode.ToString());
		for (auto& ChildItr : InNode.GetChildren())
		{
			DFSTraversalPre(ChildItr);
		}
	}

	// Depth first traversal post-order
	void DFSTraversalPost(FTFTreeNode& InNode)
	{
		for (auto& ChildItr : InNode.GetChildren())
		{
			DFSTraversalPost(ChildItr);
		}
		UE_LOG(LogTF, Warning, TEXT("\t %s"), *InNode.ToString());
	}

	// Output the tf tree node as string
	FString ToString() const
	{
		UE_LOG(LogTF, Warning, TEXT("[%s]"), *FString(__FUNCTION__));
		if (HasParent())
		{
			UE_LOG(LogTF, Warning, TEXT("[%s] HasParent "), *FString(__FUNCTION__));
			return FString::Printf(TEXT("Parent: %s; Node: %s"),
				*Parent->GetTFData()->ToString(), *TFData->ToString());
		}
		else
		{
			UE_LOG(LogTF, Warning, TEXT("[%s] NoParent"), *FString(__FUNCTION__));
			return FString::Printf(TEXT("Parent: NONE; Node: %s"),
				*TFData->ToString());
		}
	}
};

/**
*  FTFTree (World TF Tree)
*
*  - Array of root nodes representing a virtual connection with the
*    'World node', all nodes having the world as its parent will be a root node
*  - functionality to add tf tree nodes to the needed frames
*
*/
struct UTFPUBLISHER_API FTFWorldTree
{
private:
	// Array of roots (since world cannot be a node)
	TArray<FTFTreeNode> Roots;

public:
	// Default constructor
	FTFWorldTree() {}

	// Constructor with root init
	FTFWorldTree(const TArray<FTFTreeNode>& InRoots) : Roots(InRoots) { }

	// Add node to the world tf tree (by default root parent frame)
	bool AddNode(FTFTreeNode InNode, const FString& InParentFrameId = TEXT("World"))
	{
		if (InParentFrameId.Equals(TEXT("World")))
		{
			// Add as a root (parent is world)
			Roots.Emplace(InNode);
			UE_LOG(LogTF, Warning, TEXT(" \t\t [%s] added (as root)"),
				*InNode.ToString());
			return true;
		}
		else
		{
			// Search for the parent frame in every tree node
			for (auto& RootItr : Roots)
			{
				FTFTreeNode* FoundNode = nullptr;
				//if (RootItr.GetNode(InParentFrameId, FoundNode))
				//{
				//	if (FoundNode != nullptr)
				//	{
				//		UE_LOG(LogTF, Warning, TEXT(" found node: %s in root: %s"),
				//			*FoundNode->GetFrameId(), *RootItr.GetFrameId());
				//		FoundNode->AddChild(InNode);
				//		return true;
				//	}
				//	else 
				//	{
				//		UE_LOG(LogTF, Warning, TEXT(" found node: nullptr"));
				//	}
				//}

				FoundNode = RootItr.GetNode(InParentFrameId);
				if (FoundNode != nullptr)
				{
					FoundNode->AddChild(InNode);
					UE_LOG(LogTF, Warning, TEXT(" \t\t [%s] added"),
						*InNode.ToString());
					return true;
				}
			}
		}
		return false;
	}

	// Traverse world tf tree
	void Traverse()
	{
		for (auto& RootItr : Roots)
		{
			UE_LOG(LogTF, Warning, TEXT(" T \n"));
			RootItr.DFSTraversalPre(RootItr);
		}

		//for (auto& RootItr : Roots)
		//{
		//	UE_LOG(LogTF, Warning, TEXT(" T \n"));
		//	RootItr.DFSTraversalPost(RootItr);
		//}
	}
};
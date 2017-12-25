// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include "EngineMinimal.h"
#include "CoreTypes.h"
#include "CoreMinimal.h"

/**
* TFTree - Unordered Tree structure for storing/calculating TF data, containing:
*
*  - a pointer to its parent (nullptr if it is the root of the tree) 
*  - an array of its children sub-trees
*  - a valid AActor or USceneComponent for accessing its FTransform
*  - functionality to return the transform between itself and its parent
*
*/

struct /*UTFPUBLISHER_API*/ FTFTree
{
private:
	// Pointer to parent
	FTFTree * Parent;

	// Array of the children subtrees
	TArray<FTFTree> Children;

	// Name of the frame id
	FString FrameID;

	// Base object type to get the transform
	AActor* ActorBaseObject;
	USceneComponent* SceneComponentBaseObject;

	// Get relative transform function pointer variable type
	typedef FTransform(FTFTree::*RelativeTransformFuncPtrType)();

	// Function pointer to the get the relative transform function
	RelativeTransformFuncPtrType RelativeTransformFunctionPtr;

	// Get world transform function pointer variable type
	typedef FTransform(FTFTree::*RelativeTransformFuncPtrType)();

	// Function pointer to the get the world transform function
	RelativeTransformFuncPtrType WorldTransformFunctionPtr;

public:
	// Default constructor
	FTFTree() : Parent(nullptr) { }
	
	// Constructor with initialization as actor and frame id
	FTFTree(AActor* InActor, FString InFrameID)
		: ActorBaseObject(InActor), Parent(nullptr), FrameID(InFrameID)
	{
		// Bind the get transforms functions
		RelativeTransformFunctionPtr = &FTFTree::GetRelativeTransform_AsActor;
		WorldTransformFunctionPtr = &FTFTree::GetWorldTransform_AsActor;
	}
	
	// Constructor with initialization as scene component and frame id
	FTFTree(USceneComponent* InSceneComponent, FString InFrameID)
		: SceneComponentBaseObject(InSceneComponent), Parent(nullptr), FrameID(InFrameID)
	{
		// Bind the get transforms functions
		RelativeTransformFunctionPtr = &FTFTree::GetRelativeTransform_AsSceneComponent;
		WorldTransformFunctionPtr = &FTFTree::GetWorldTransform_AsSceneComponent;
	}
	
	// Destructor
	~FTFTree()
	{
		Empty();
	}
	
	// Empty tree
	void Empty()
	{
		Children.Empty();
		Parent = nullptr;
	}
	
	// Set pointer to parent
	void SetParent(FTFTree* InParent)
	{
		Parent = InParent;
	}
	
	// Get pointer to parent
	FTFTree* GetParent() const
	{
		return Parent;
	}
	
	// Get the frame id name
	FString GetFrameID() const
	{
		return FrameID;
	}
	
	// Set frame id
	void SetSetFrameID(FString InFrameID)
	{
		FrameID = InFrameID;
	}
	
	// Add child to array
	void AddChild(FTFTree InChild)
	{
		InChild.SetParent(this);
		Children.Add(InChild);
	}
	
	// Add child to the parent frame
	bool AddChildAtFrameID(FTFTree InChild, const FString& InParentFrameID)
	{
		// Check current parent frame id
		if (FrameID.Equals(InParentFrameID))
		{
			AddChild(InChild);
			return true;
		}
		else
		{
			// Check every child frame id
			for (auto& Child : Children)
			{
				Child.AddChildAtFrameID(InChild, InParentFrameID);
			}
		}
		return false;
	}
	
	// Get array children
	TArray<FTFTree> GetChildren() const
	{
		return Children;
	}
	
	// Get the relative transform between the parent and the current item
	FORCEINLINE FTransform GetRelativeTransform()
	{
		return (this->*RelativeTransformFunctionPtr)();
	}
	
	// Get the world transform 
	FORCEINLINE FTransform GetWorldTransform()
	{
		return (this->*WorldTransformFunctionPtr)();
	}

private:
	// Get the relative transform of actor
	FORCEINLINE FTransform GetRelativeTransform_AsActor()
	{
		// Check if we are root
		if (Parent == nullptr)
		{
			// Return world transform
			return GetWorldTransform();
		}
		else
		{
			// Return transform relative to parent
			return ActorBaseObject->GetActorTransform().GetRelativeTransform(
				Parent->GetWorldTransform());
		}
	}
	
	// Get the relative transform of scene component
	FORCEINLINE FTransform GetRelativeTransform_AsSceneComponent()
	{
		// Check if we are root
		if (Parent == nullptr)
		{
			// Return world transform
			return GetWorldTransform();
		}
		else
		{
			// Return transform relative to parent
			return SceneComponentBaseObject->GetComponentTransform().GetRelativeTransform(
				Parent->GetWorldTransform());
		}
	}
	
	// Get the world transform of actor
	FORCEINLINE FTransform GetWorldTransform_AsActor()
	{
		return ActorBaseObject->GetActorTransform();
	}
	
	// Get the world transform of scene component
	FORCEINLINE FTransform GetWorldTransform_AsSceneComponent()
	{
		return SceneComponentBaseObject->GetComponentTransform();
	}
};


//===============================================================================
// TODO
// Linker issues if defining the functions in the header file,
// and including the FTFTree.h in a header file (.cpp is fine),
// this however does not seem to be an issue for Vector4 for example
//===============================================================================
//struct /*UTFPUBLISHER_API*/ FTFTree
//{
//public:
//	// Default constructor
//	FTFTree();
//
//	// Constructor with initialization as actor and frame id
//	FTFTree(AActor* InActor, FString InFrameID);
//
//	// Constructor with initialization as scene component and frame id
//	FTFTree(USceneComponent* InSceneComponent, FString InFrameID);
//
//	// Destructor
//	~FTFTree();
//
//	// Empty tree
//	void Empty();
//
//	// Set pointer to parent
//	void SetParent(FTFTree* InParent);
//
//	// Get pointer to parent
//	FTFTree* GetParent() const;
//
//	// Set frame id
//	void SetSetFrameID(FString InFrameID);
//
//	// Get the frame id
//	FString GetFrameID() const;
//
//	// Add child to array
//	void AddChild(FTFTree InChild);
//
//	// Add child to the parent frame
//	bool AddChildAtFrameID(FTFTree InChild, const FString& InParentFrameID);
//
//	// Get the array of children
//	TArray<FTFTree> GetChildren() const;
//
//	// Get the relative transform between the parent and the current item
//	FORCEINLINE FTransform GetRelativeTransform();
//
//	// Get the world transform 
//	FORCEINLINE FTransform GetWorldTransform();
//
//private:
//	// Get the relative transform of actor
//	FORCEINLINE FTransform GetRelativeTransform_AsActor();
//
//	// Get the relative transform of scene component
//	FORCEINLINE FTransform GetRelativeTransform_AsSceneComponent();
//
//	// Get the world transform of actor
//	FORCEINLINE FTransform GetWorldTransform_AsActor();
//
//	// Get the world transform of scene component
//	FORCEINLINE FTransform GetWorldTransform_AsSceneComponent();
//
//	// Get relative transform function pointer variable type
//	typedef FTransform(FTFTree::*RelativeTransformFuncPtrType)();
//
//	// Function pointer to the get the relative transform function
//	RelativeTransformFuncPtrType RelativeTransformFunctionPtr;
//
//	// Get world transform function pointer variable type
//	typedef FTransform(FTFTree::*RelativeTransformFuncPtrType)();
//
//	// Function pointer to the get the world transform function
//	RelativeTransformFuncPtrType WorldTransformFunctionPtr;
//
//	// Pointer to parent
//	FTFTree* Parent;
//
//	// Array of the children subtrees
//	TArray<FTFTree> Children;
//
//	// Name of the frame id
//	FString FrameID;
//
//	// Base object type to get the transform
//	AActor* ActorBaseObject;
//	USceneComponent* SceneComponentBaseObject;
//};
//
//
///* FTFTree functions
//*****************************************************************************/
//// Default constructor
//FTFTree::FTFTree() : Parent(nullptr) { }
//
//// Constructor with initialization as actor and frame id
//FTFTree::FTFTree(AActor* InActor, FString InFrameID)
//	: ActorBaseObject(InActor), Parent(nullptr), FrameID(InFrameID)
//{
//	// Bind the get transforms functions
//	RelativeTransformFunctionPtr = &FTFTree::GetRelativeTransform_AsActor;
//	WorldTransformFunctionPtr = &FTFTree::GetWorldTransform_AsActor;
//}
//
//// Constructor with initialization as scene component and frame id
//FTFTree::FTFTree(USceneComponent* InSceneComponent, FString InFrameID)
//	: SceneComponentBaseObject(InSceneComponent), Parent(nullptr), FrameID(InFrameID)
//{
//	// Bind the get transforms functions
//	RelativeTransformFunctionPtr = &FTFTree::GetRelativeTransform_AsSceneComponent;
//	WorldTransformFunctionPtr = &FTFTree::GetWorldTransform_AsSceneComponent;
//}
//
//// Destructor
//FTFTree::~FTFTree()
//{
//	Empty();
//}
//
//// Empty tree
//void FTFTree::Empty()
//{
//	Children.Empty();
//	Parent = nullptr;
//}
//
//// Set pointer to parent
//void FTFTree::SetParent(FTFTree* InParent)
//{
//	Parent = InParent;
//}
//
//// Get pointer to parent
//FTFTree* FTFTree::GetParent() const
//{
//	return Parent;
//}
//
//// Get the frame id name
//FString FTFTree::GetFrameID() const
//{
//	return FrameID;
//}
//
//// Set frame id
//void FTFTree::SetSetFrameID(FString InFrameID)
//{
//	FrameID = InFrameID;
//}
//
//// Add child to array
//void FTFTree::AddChild(FTFTree InChild)
//{
//	InChild.SetParent(this);
//	Children.Add(InChild);
//}
//
//// Add child to the parent frame
//bool FTFTree::AddChildAtFrameID(FTFTree InChild, const FString& InParentFrameID)
//{
//	// Check current parent frame id
//	if (FrameID.Equals(InParentFrameID))
//	{
//		AddChild(InChild);
//		return true;
//	}
//	else
//	{
//		// Check every child frame id
//		for (auto& Child : Children)
//		{
//			Child.AddChildAtFrameID(InChild, InParentFrameID);
//		}
//	}
//	return false;
//}
//
//// Get array children
//TArray<FTFTree> FTFTree::GetChildren() const
//{
//	return Children;
//}
//
//// Get the relative transform between the parent and the current item
//FORCEINLINE FTransform FTFTree::GetRelativeTransform()
//{
//	return (this->*RelativeTransformFunctionPtr)();
//}
//
//// Get the world transform 
//FORCEINLINE FTransform FTFTree::GetWorldTransform()
//{
//	return (this->*WorldTransformFunctionPtr)();
//}
//
//// Get the relative transform of actor
//FORCEINLINE FTransform FTFTree::GetRelativeTransform_AsActor()
//{
//	// Check if we are root
//	if (Parent == nullptr)
//	{
//		// Return world transform
//		return FTFTree::GetWorldTransform();
//	}
//	else
//	{
//		// Return transform relative to parent
//		return ActorBaseObject->GetActorTransform().GetRelativeTransform(
//			Parent->GetWorldTransform());
//	}
//}
//
//// Get the relative transform of scene component
//FORCEINLINE FTransform FTFTree::GetRelativeTransform_AsSceneComponent()
//{
//	// Check if we are root
//	if (Parent == nullptr)
//	{
//		// Return world transform
//		return FTFTree::GetWorldTransform();
//	}
//	else
//	{
//		// Return transform relative to parent
//		return SceneComponentBaseObject->GetComponentTransform().GetRelativeTransform(
//			Parent->GetWorldTransform());
//	}
//}
//
//// Get the world transform of actor
//FORCEINLINE FTransform FTFTree::GetWorldTransform_AsActor()
//{
//	return ActorBaseObject->GetActorTransform();
//}
//
//// Get the world transform of scene component
//FORCEINLINE FTransform FTFTree::GetWorldTransform_AsSceneComponent()
//{
//	return SceneComponentBaseObject->GetComponentTransform();
//}
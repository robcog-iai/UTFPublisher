// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "UObject/NoExportTypes.h"
#include "TFNode.generated.h"

/**
* UTFNode - TF node, containing:
*  
*  - pointer to parent node
*  - array of children nodes
*  - node data:
*    - frame name
*    - pointer to an entity (AActor or USceneComponent) with access to FTransform
*/
UCLASS()
class UTFPUBLISHER_API UTFNode : public UObject
{
	GENERATED_BODY()

public:	
	// Default constructor
	UTFNode();

	// Default destructor
	virtual ~UTFNode();

	// Init class with UObject
	void Init(UObject* InObject, const FString& InFrameId);

	// Init class with AActor
	void Init(AActor* InActor, const FString& InFrameId);

	// Init class with USceneComponent
	void Init(USceneComponent* InSceneComponent, const FString& InFrameId);

	// Get relative transform
	FTransform GetRelativeTransform() const;

	// Get relative transform
	FTransform GetWorldTransform() const;

	// Get frame id
	FString GetFrameId() const { return ChildFrameId; }

	// Get parent
	UTFNode* GetParent() const { return Parent; }

	// String output of the tf node data
	FString ToString() const;

private:
	// Add child
	void AddChild(UTFNode* InChildNode);

	// Called when the actor base object or the components parent gets destroyed
	UFUNCTION()
	void OnEntityDestroyed(AActor* DestroyedActor);

	// Get the relative transform of actor
	FORCEINLINE FTransform GetRelativeTransform_FromActor() const;

	// Get the relative transform of scene component
	FORCEINLINE FTransform GetRelativeTransform_FromSceneComponent() const;

	// Get the world transform of actor
	FORCEINLINE FTransform GetWorldTransform_FromActor() const;

	// Get the world transform of scene component
	FORCEINLINE FTransform GetWorldTransform_FromSceneComponent() const;

	// Get relative transform function pointer variable type
	typedef FTransform(UTFNode::*GetRelativeTransformFuncPtrType)() const;
	// Function pointer to the get the relative transform function
	GetRelativeTransformFuncPtrType GetRelativeTransformFunctionPtr;

	// Get world transform function pointer variable type
	typedef FTransform(UTFNode::*GetWorldTransformFuncPtrType)() const;
	// Function pointer to the get the world transform function
	GetWorldTransformFuncPtrType GetWorldTransformFunctionPtr;
	
	// Parent
	UPROPERTY() // avoid GC
	UTFNode* Parent;

	// Array of children nodes
	UPROPERTY() // avoid GC
	TArray<UTFNode*> Children;

	// Name of the (child) frame id
	FString ChildFrameId;

	// Base object type to get the FTransform (of AACtor or USceneComponent)
	AActor* ActorBaseObject;
	USceneComponent* SceneComponentBaseObject;

	// Give access to private data 
	friend class UTFTree;
	friend class UTFWorldTree;
};

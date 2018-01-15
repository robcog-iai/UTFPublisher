// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "Components/ActorComponent.h"
#include "TFNodeComp.generated.h"

// Forward declaration of the owner tf world tree in order to avoid circular dependency
struct FTFWorldTree;
struct FTFTree;

/**
* UTFNodeComp - TF Node, inherits from UActorComponent to have life duration synced
*
*  - pointer to parent node
*  - array of children nodes
*  - node data:
*    - frame name
*    - pointer to parent entity (AActor or USceneComponent) with access to FTransform
*/
UCLASS(ClassGroup = (TF), meta = (BlueprintSpawnableComponent))
class UTFPUBLISHER_API UTFNodeComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTFNodeComp();

	// Destructor
	virtual ~UTFNodeComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called when the destroy process begins
	virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Init class with UObject
	void Init(UObject* InObject, const FString& InFrameId, FTFWorldTree* InOwnerWorldTree = nullptr);

	// Init class with AActor
	void Init(AActor* InActor, const FString& InFrameId, FTFWorldTree* InOwnerWorldTree = nullptr);

	// Init class with USceneComponent
	void Init(USceneComponent* InSceneComponent, const FString& InFrameId, FTFWorldTree* InOwnerWorldTree = nullptr);

	// Get relative transform
	FTransform GetRelativeTransform() const;

	// Get relative transform
	FTransform GetWorldTransform() const;

	// String output of the tf node data
	FString ToString() const;

private:
	// Add child
	void AddChild(UTFNodeComp* InChildNode);

	// Get the relative transform of actor
	FORCEINLINE FTransform GetRelativeTransform_FromActor() const;

	// Get the relative transform of scene component
	FORCEINLINE FTransform GetRelativeTransform_FromSceneComponent() const;

	// Get the world transform of actor
	FORCEINLINE FTransform GetWorldTransform_FromActor() const;

	// Get the world transform of scene component
	FORCEINLINE FTransform GetWorldTransform_FromSceneComponent() const;

	// Get relative transform function pointer variable type
	typedef FTransform(UTFNodeComp::*GetRelativeTransformFuncPtrType)() const;
	// Function pointer to the get the relative transform function
	GetRelativeTransformFuncPtrType GetRelativeTransformFunctionPtr;

	// Get world transform function pointer variable type
	typedef FTransform(UTFNodeComp::*GetWorldTransformFuncPtrType)() const;
	// Function pointer to the get the world transform function
	GetWorldTransformFuncPtrType GetWorldTransformFunctionPtr;

	// Parent
	UTFNodeComp* Parent;

	// Array of children nodes
	TArray<UTFNodeComp*> Children;

	// Name of the frame id (tf equivalent of child_frame_id) 
	FString FrameId;

	// Base object type to get the FTransform (of AACtor or USceneComponent)
	AActor* ActorBaseObject;
	USceneComponent* SceneComponentBaseObject;

	// Pointer to the owner world tree (to remove itself from tree in case of destruction)
	FTFWorldTree* OwnerWorldTree;
	FTFTree* OwnerTree;

	// Give access to private data 
	friend struct FTFTree;
	friend struct FTFWorldTree;
};

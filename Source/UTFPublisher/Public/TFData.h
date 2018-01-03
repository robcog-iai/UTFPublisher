// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "UObject/NoExportTypes.h"
#include "TFData.generated.h"

/**
 * UTFData - TF related data, containing:
 *
 *  - child frame name
 *  - pointer to an entity (AActor or USceneComponent) with FTransform access
 *
 */
UCLASS()
class UTFPUBLISHER_API UTFData : public UObject
{
	GENERATED_BODY()
	
public:
	// Default constructor
	UTFData();

	// Default destructor
	virtual ~UTFData();

	// Init class with UObject
	void Init(UObject* InObject, const FString& InFrameId);

	// Init class with AActor
	void Init(AActor* InActor, const FString& InFrameId);

	// Init class with USceneComponent
	void Init(USceneComponent* InSceneComponent, const FString& InFrameId);
	
	// Get the frame id name
	const FString& GetFrameId() const;

	// Get transform
	FTransform GetTransform();

	// String output of the tf data
	FString ToString() const;

private:
	// Called when the actor base object or the components parent gets destroyed
	UFUNCTION()
	void OnEntityDestroyed(AActor* DestroyedActor);

	// Get the world transform of actor
	FORCEINLINE FTransform GetTransform_AsActor();

	// Get the world transform of scene component
	FORCEINLINE FTransform GetTransform_AsSceneComponent();

	// Name of the (child) frame id
	FString ChildFrameId;

	// Base object type to get the transform (AACtor or USceneComponent)
	AActor* ActorBaseObject;
	USceneComponent* SceneComponentBaseObject;

	// Get world transform function pointer variable type
	typedef FTransform(UTFData::*GetTransformFuncPtrType)();
	// Function pointer to the get the world transform function
	GetTransformFuncPtrType GetTransformFunctionPtr;
};

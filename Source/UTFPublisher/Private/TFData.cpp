// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFData.h"

// Default constructor
UTFData::UTFData()
{
	UE_LOG(LogTF, Warning, TEXT("[%s] Constructor of %s"),
		*FString(__FUNCTION__), *GetName());
	ActorBaseObject = nullptr;
	SceneComponentBaseObject = nullptr;
}

// Destructor
UTFData::~UTFData()
{
	UE_LOG(LogTF, Warning, TEXT("[%s] Desctructor of %s"),
		*FString(__FUNCTION__), *GetName());
}

// Init data with UObject
void UTFData::Init(UObject* InObject, const FString& InFrameId)
{
	UE_LOG(LogTF, Warning, TEXT("[%s] InObject Ptr %p"),
		*FString(__FUNCTION__), InObject);
	if (auto AA = Cast<AActor>(InObject))
	{
		if (AA->IsA(AActor::StaticClass()))
		{
			UE_LOG(LogTF, Warning, TEXT("[%s] %s is of type AActor"),
				*FString(__FUNCTION__), *AA->GetName());;
		}
		Init(AA, InFrameId);
	}
	else if (auto USC = Cast<USceneComponent>(InObject))
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] USC Ptr %p"),
			*FString(__FUNCTION__), USC);

		if (USC->IsA(USceneComponent::StaticClass()))
		{
			UE_LOG(LogTF, Warning, TEXT("[%s] %s is of type USceneComponent"),
				*FString(__FUNCTION__), *USC->GetName());;
		}
		Init(USC, InFrameId);
	}
}

// Init data with AActor
void UTFData::Init(AActor* InActor, const FString& InFrameId)
{
	ActorBaseObject = InActor;
	ChildFrameId = InFrameId;
	// Bind the transform function ptr
	GetTransformFunctionPtr = &UTFData::GetTransform_AsActor;
	InActor->OnDestroyed.AddDynamic(this, &UTFData::OnEntityDestroyed);
}

// Init data with USceneComponent
void UTFData::Init(USceneComponent* InSceneComponent, const FString& InFrameId)
{
	SceneComponentBaseObject = InSceneComponent;
	UE_LOG(LogTF, Warning, TEXT("[%s] In USCENCPOMNT INIT, comp name: %s "),
		*FString(__FUNCTION__), *SceneComponentBaseObject->GetName());
	ChildFrameId = InFrameId;
	// Bind the transform function ptr
	GetTransformFunctionPtr = &UTFData::GetTransform_AsSceneComponent;
	SceneComponentBaseObject->GetOwner()->OnDestroyed.AddDynamic(this, &UTFData::OnEntityDestroyed);

	UE_LOG(LogTF, Warning, TEXT("[%s] SceneComponentBaseObject Ptr %p \n InSceneComponent Ptr %p"),
		*FString(__FUNCTION__), SceneComponentBaseObject, InSceneComponent);

	if (SceneComponentBaseObject->IsValidLowLevel())
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] VAlid ll %s %p"),
			*FString(__FUNCTION__), *SceneComponentBaseObject->GetName(), SceneComponentBaseObject);
	}
	else
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] NOOT VAlid ll %s  %p"),
			*FString(__FUNCTION__), *SceneComponentBaseObject->GetName(), SceneComponentBaseObject);
	}

	if (SceneComponentBaseObject == nullptr)
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] NULLPTR %s  %p"),
			*FString(__FUNCTION__), *SceneComponentBaseObject->GetName(), SceneComponentBaseObject);
	}
	else
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] NOOT NULLPTR ll %s  %p"),
			*FString(__FUNCTION__), *SceneComponentBaseObject->GetName(), SceneComponentBaseObject);
	}
}

void UTFData::OnEntityDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogTF, Warning, TEXT(" Actor %s destroyed !!! "),
		*DestroyedActor->GetName());
	ActorBaseObject = nullptr;
	SceneComponentBaseObject = nullptr;
}

// Get the frame id name
const FString& UTFData::GetFrameId() const
{
	return ChildFrameId;
}

// Get transform
FTransform UTFData::GetTransform()
{
	return (this->*GetTransformFunctionPtr)();
}

// Get the world transform of actor
FORCEINLINE FTransform UTFData::GetTransform_AsActor()
{
	return ActorBaseObject->GetTransform();
}

// Get the world transform of scene component
FORCEINLINE FTransform UTFData::GetTransform_AsSceneComponent()
{
	return SceneComponentBaseObject->GetComponentTransform();
}

// Output the tf data as string
FString UTFData::ToString() const
{
	UE_LOG(LogTF, Warning, TEXT("[%s] ActorBaseObject: %p, SceneComponentBaseObject: %p"),
		*FString(__FUNCTION__), ActorBaseObject, SceneComponentBaseObject);
	FString BaseObjName = TEXT("NONE");
	if (ActorBaseObject && ActorBaseObject->IsValidLowLevel())
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] ActorBaseObject->IsValidLowLevel()"), *FString(__FUNCTION__));
		BaseObjName = ActorBaseObject->GetName();
	}
	else if (SceneComponentBaseObject && SceneComponentBaseObject->IsValidLowLevel())
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] SceneComponentBaseObject->IsValidLowLevel()"), *FString(__FUNCTION__));
		BaseObjName = SceneComponentBaseObject->GetName();
	}
	else
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] Nope RM)"), *FString(__FUNCTION__));
		// TODO else remove from tree
		UE_LOG(LogTF, Warning, TEXT("[%s] TODO RM Node from tree "),
			*FString(__FUNCTION__));
	}


	if (SceneComponentBaseObject == nullptr)
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] NULLPTR  %p"),
			*FString(__FUNCTION__), SceneComponentBaseObject);
	}
	else
	{
		UE_LOG(LogTF, Warning, TEXT("[%s] NOOT NULLPTR ll %s  %p"),
			*FString(__FUNCTION__), *SceneComponentBaseObject->GetName(), SceneComponentBaseObject);
	}

	return FString::Printf(TEXT("ChildFrameId=%s, BaseObjectName=%s"),
		*ChildFrameId, *BaseObjName);
}
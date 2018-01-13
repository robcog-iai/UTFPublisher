// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFNodeComp.h"

// Sets default values for this component's properties
UTFNodeComp::UTFNodeComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Destructor
UTFNodeComp::~UTFNodeComp()
{
	UE_LOG(LogTF, Error, TEXT("[%s]"), *FString(__FUNCTION__));
}

// Called when the game starts
void UTFNodeComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called when the destroy process begins
void UTFNodeComp::BeginDestroy()
{
	Super::BeginDestroy();

	UE_LOG(LogTF, Error, TEXT("[%s]"), *FString(__FUNCTION__));

}

// Called every frame
void UTFNodeComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// Init data with UObject
void UTFNodeComp::Init(UObject* InObject, const FString& InFrameId)
{
	if (auto AA = Cast<AActor>(InObject))
	{
		Init(AA, InFrameId);
	}
	else if (auto USC = Cast<USceneComponent>(InObject))
	{
		Init(USC, InFrameId);
	}
}

// Init data with AActor
void UTFNodeComp::Init(AActor* InActor, const FString& InFrameId)
{
	ActorBaseObject = InActor;
	ChildFrameId = InFrameId;

	// Bind the transform function ptr
	GetRelativeTransformFunctionPtr = &UTFNodeComp::GetRelativeTransform_FromActor;
	GetWorldTransformFunctionPtr = &UTFNodeComp::GetWorldTransform_FromActor;
}

// Init data with USceneComponent
void UTFNodeComp::Init(USceneComponent* InSceneComponent, const FString& InFrameId)
{
	SceneComponentBaseObject = InSceneComponent;
	ChildFrameId = InFrameId;

	// Bind the transform function ptr
	GetRelativeTransformFunctionPtr = &UTFNodeComp::GetRelativeTransform_FromSceneComponent;
	GetWorldTransformFunctionPtr = &UTFNodeComp::GetWorldTransform_FromSceneComponent;
}

// Get relative transform
FTransform UTFNodeComp::GetRelativeTransform() const
{
	return (this->*GetRelativeTransformFunctionPtr)();
}

// Get world transform
FTransform UTFNodeComp::GetWorldTransform() const
{
	return (this->*GetWorldTransformFunctionPtr)();
}


// Add child
void UTFNodeComp::AddChild(UTFNodeComp* InChildNode)
{
	Children.Emplace(InChildNode);
	InChildNode->Parent = this;
}

// Get the world transform of actor
FORCEINLINE FTransform UTFNodeComp::GetRelativeTransform_FromActor() const
{
	if (!ActorBaseObject->IsValidLowLevel())
	{
		return FTransform();
	}

	if (Parent != nullptr)
	{
		// Get the relative transform to the parent
		return Parent->GetWorldTransform().GetRelativeTransform(
			ActorBaseObject->GetTransform());
	}
	else
	{
		// Get world transform
		return ActorBaseObject->GetTransform();
	}
}

// Get the world transform of scene component
FORCEINLINE FTransform UTFNodeComp::GetRelativeTransform_FromSceneComponent() const
{
	if (!SceneComponentBaseObject->IsValidLowLevel())
	{
		return FTransform();
	}

	if (Parent != nullptr)
	{
		// Get the relative transform to the parent
		return Parent->GetWorldTransform().GetRelativeTransform(
			SceneComponentBaseObject->GetComponentTransform());
	}
	else
	{
		// Get world transform
		return SceneComponentBaseObject->GetComponentTransform();
	}
}

// Get the world transform of actor
FORCEINLINE FTransform UTFNodeComp::GetWorldTransform_FromActor() const
{
	if (!ActorBaseObject->IsValidLowLevel())
	{
		return FTransform();
	}
	return ActorBaseObject->GetTransform();
}

// Get the world transform of scene component
FORCEINLINE FTransform UTFNodeComp::GetWorldTransform_FromSceneComponent() const
{
	if (!SceneComponentBaseObject->IsValidLowLevel())
	{
		return FTransform();
	}
	return SceneComponentBaseObject->GetComponentTransform();
}

// Output the tf node data as string
FString UTFNodeComp::ToString() const
{
	FString BaseObjName;
	if (ActorBaseObject)
	{
		BaseObjName = ActorBaseObject->GetName();
	}
	else if (SceneComponentBaseObject)
	{
		BaseObjName = SceneComponentBaseObject->GetName();
	}

	if (Parent != nullptr)
	{
		FString ParentBaseObjName;
		if (Parent->ActorBaseObject)
		{
			ParentBaseObjName = Parent->ActorBaseObject->GetName();
		}
		else if (Parent->SceneComponentBaseObject)
		{
			ParentBaseObjName = Parent->SceneComponentBaseObject->GetName();
		}
		//return FString::Printf(TEXT("\t[%s(%s)] -> [%s(%s)]; T=%s \n"),
		//	*Parent->ChildFrameId, *ParentBaseObjName, *ChildFrameId, *BaseObjName,
		//	*GetRelativeTransform().ToString());
		return FString::Printf(TEXT("\t[%s(%s)] -> [%s(%s)]; \n"),
			*Parent->ChildFrameId, *ParentBaseObjName, *ChildFrameId, *BaseObjName);
	}
	else
	{
		//return FString::Printf(TEXT("\t[None(none)] -> [%s(%s)]; T=%s \n"),
		//	*ChildFrameId, *BaseObjName, *GetRelativeTransform().ToString());
		return FString::Printf(TEXT("\t[None(none)] -> [%s(%s)];\n"),
			*ChildFrameId, *BaseObjName);
	}
}
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFNode.h"
#include "TFWorldTree.h"

// Default constructor
UTFNode::UTFNode() : Parent(nullptr), ActorBaseObject(nullptr), SceneComponentBaseObject(nullptr)
{
}

// Destructor
UTFNode::~UTFNode()
{
	UE_LOG(LogTF, Error, TEXT("[%s]"), *FString(__FUNCTION__));
}

// Init data with UObject
void UTFNode::Init(UObject* InObject, const FString& InFrameId)
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
void UTFNode::Init(AActor* InActor, const FString& InFrameId)
{
	ActorBaseObject = InActor;
	ChildFrameId = InFrameId;

	// Bind the transform function ptr
	GetRelativeTransformFunctionPtr = &UTFNode::GetRelativeTransform_FromActor;
	GetWorldTransformFunctionPtr = &UTFNode::GetWorldTransform_FromActor;

	// Bind to on destroyed function
	InActor->OnDestroyed.AddDynamic(this, &UTFNode::OnEntityDestroyed);
}

// Init data with USceneComponent
void UTFNode::Init(USceneComponent* InSceneComponent, const FString& InFrameId)
{
	SceneComponentBaseObject = InSceneComponent;
	ChildFrameId = InFrameId;

	// Bind the transform function ptr
	GetRelativeTransformFunctionPtr = &UTFNode::GetRelativeTransform_FromSceneComponent;
	GetWorldTransformFunctionPtr = &UTFNode::GetWorldTransform_FromSceneComponent;

	// Bind to on destroyed function
	SceneComponentBaseObject->GetOwner()->OnDestroyed.AddDynamic(this, &UTFNode::OnEntityDestroyed);
}

// Get relative transform
FTransform UTFNode::GetRelativeTransform() const
{
	return (this->*GetRelativeTransformFunctionPtr)();
}

// Get world transform
FTransform UTFNode::GetWorldTransform() const
{
	return (this->*GetWorldTransformFunctionPtr)();
}

// Add child
void UTFNode::AddChild(UTFNode* InChildNode)
{
	Children.Emplace(InChildNode);
	InChildNode->Parent = this;
}

// Get the world transform of actor
FORCEINLINE FTransform UTFNode::GetRelativeTransform_FromActor() const
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
FORCEINLINE FTransform UTFNode::GetRelativeTransform_FromSceneComponent() const
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
FORCEINLINE FTransform UTFNode::GetWorldTransform_FromActor() const
{
	if (!ActorBaseObject->IsValidLowLevel())
	{
		return FTransform();
	}
	return ActorBaseObject->GetTransform();
}

// Get the world transform of scene component
FORCEINLINE FTransform UTFNode::GetWorldTransform_FromSceneComponent() const
{
	if (!SceneComponentBaseObject->IsValidLowLevel())
	{
		return FTransform();
	}
	return SceneComponentBaseObject->GetComponentTransform();
}

// Called when the representing entity gets destroyed
void UTFNode::OnEntityDestroyed(AActor* DestroyedActor)
{
	if (auto TFWorldTree = Cast<UTFWorldTree>(GetOuter()))
	{
		UE_LOG(LogTF, Error, TEXT("[%s] Actor %s destroyed, removing node %s from %s"),
			*FString(__FUNCTION__), *DestroyedActor->GetName(), *ToString(), *GetOuter()->GetName());
		TFWorldTree->RemoveNode(this);
	}
	//Children.Empty();
	//Parent = nullptr;
	ConditionalBeginDestroy();
}

// Output the tf node data as string
FString UTFNode::ToString() const
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

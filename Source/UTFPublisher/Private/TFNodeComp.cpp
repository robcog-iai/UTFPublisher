// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFNodeComp.h"
#include "TFWorldTree.h"
#include "TFTree.h"
#include "geometry_msgs/TransformStamped.h"

// Sets default values for this component's properties
UTFNodeComp::UTFNodeComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

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
	// Remove itself from the TF world tree
	if (OwnerWorldTree != nullptr)
	{
		OwnerWorldTree->RemoveNode(this);
		/*OwnerTree->RemoveNode(this);*/
	}
	UE_LOG(LogTF, Error, TEXT("[%s]"), *FString(__FUNCTION__));

}

// Called every frame
void UTFNodeComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// Init data with UObject
void UTFNodeComp::Init(UObject* InObject, const FString& InFrameId, FTFWorldTree* InOwnerWorldTree)
{
	if (auto AA = Cast<AActor>(InObject))
	{
		Init(AA, InFrameId, InOwnerWorldTree);
	}
	else if (auto USC = Cast<USceneComponent>(InObject))
	{
		Init(USC, InFrameId, InOwnerWorldTree);
	}
	// TODO
	//else
	//{
	//	// World node attached to the TFPublisher
	//}
}

// Init data with AActor
void UTFNodeComp::Init(AActor* InActor, const FString& InFrameId, FTFWorldTree* InOwnerWorldTree)
{
	ActorBaseObject = InActor;
	FrameId = InFrameId;
	OwnerWorldTree = InOwnerWorldTree;

	// Bind the transform function ptr
	GetRelativeTransformFunctionPtr = &UTFNodeComp::GetRelativeTransform_FromActor;
	GetWorldTransformFunctionPtr = &UTFNodeComp::GetWorldTransform_FromActor;
}

// Init data with USceneComponent
void UTFNodeComp::Init(USceneComponent* InSceneComponent, const FString& InFrameId, FTFWorldTree* InOwnerWorldTree)
{
	SceneComponentBaseObject = InSceneComponent;
	FrameId = InFrameId;

	// Bind the transform function ptr
	GetRelativeTransformFunctionPtr = &UTFNodeComp::GetRelativeTransform_FromSceneComponent;
	GetWorldTransformFunctionPtr = &UTFNodeComp::GetWorldTransform_FromSceneComponent;
}

// TODO when parent is a world node generated from TFPublisher.cpp
//// Get geometry_msgs::TransformStamped message
//geometry_msgs::TransformStamped GetTransformStamped(const FROSTime& InTime, const uint32 InSeq = 0)
//{
//	geometry_msgs::TransformStamped StampedTransformMsg;
//
//	std_msgs::Header Header;
//	Header.SetSeq(InSeq);
//	const FString ParentFrameId = Parent != nullptr ?
//		Parent->FrameId : RootFrameName;
//	Header.SetFrameId(ParentFrameId);
//	Header.SetStamp(InTime);
//
//	// Transform to ROS coordinate system
//	FTransform ROSTransf = FCoordConvStatics::UToROS(GetRelativeTransform());
//
//	geometry_msgs::Transform TransfMsg(
//		geometry_msgs::Vector3(ROSTransf.GetLocation()),
//		geometry_msgs::Quaternion(ROSTransf.GetRotation()));
//
//	StampedTransformMsg.SetHeader(Header);
//	StampedTransformMsg.SetChildFrameId(FrameId);
//	StampedTransformMsg.SetTransform(TransfMsg);
//
//	return StampedTransformMsg;
//}

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
			*Parent->FrameId, *ParentBaseObjName, *FrameId, *BaseObjName);
	}
	else
	{
		//return FString::Printf(TEXT("\t[None(none)] -> [%s(%s)]; T=%s \n"),
		//	*ChildFrameId, *BaseObjName, *GetRelativeTransform().ToString());
		return FString::Printf(TEXT("\t[None(none)] -> [%s(%s)];\n"),
			*FrameId, *BaseObjName);
	}
}
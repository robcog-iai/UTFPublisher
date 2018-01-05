// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFPublisher.h"
#include "TagStatics.h"
#include "CoordConvStatics.h"

// Sets default values
ATFPublisher::ATFPublisher()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Update on tick by default
	bUseStaticPublishRate = false;

	// Default timer delta time (s) (0 = on Tick)
	StaticPublishRate = 0.0f;

	// ROSBridge server default values
	ServerIP = "127.0.0.1";
	ServerPORT = 9090;
}

// Called when the game starts or when spawned
void ATFPublisher::BeginPlay()
{
	Super::BeginPlay();

	// Create the tf world tree object
	TFWWorldTree = NewObject<UTFWorldTree>(this, TEXT("TFWorldTree"));

	// Build the tf tree
	BuildTFTree();

	// Create the ROSBridge handler for connecting with ROS
	ROSBridgeHandler = MakeShareable<FROSBridgeHandler>(
		new FROSBridgeHandler(ServerIP, ServerPORT));
	
	// Create the publisher
	TFPublisher = MakeShareable<FROSBridgePublisher>(
		new FROSBridgePublisher("tf2_msgs/TFMessage", "/tf"));

	// Add publisher
	ROSBridgeHandler->AddPublisher(TFPublisher);

	// Connect to ROS
	ROSBridgeHandler->Connect();

	// Bind publish function to timer
	if (bUseStaticPublishRate)
	{
		if (StaticPublishRate > 0.f)
		{
			// Disable tick
			SetActorTickEnabled(false);
			// Setup timer
			GetWorldTimerManager().SetTimer(TFPubTimer, this, &ATFPublisher::PublishTF, StaticPublishRate, true);
		}
	}
	else
	{
		// Take into account the PublishRate Tag key value pair (if missing, publish on tick)
	}
}

// Called when destroyed or game stopped
void ATFPublisher::EndPlay(const EEndPlayReason::Type Reason)
{
	//ROSBridgeHandler->Disconnect();

	Super::EndPlay(Reason);
}

// Called every frame
void ATFPublisher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PublishTF();
}

// Build the tf tree, ~ O(n^2) complexity
void ATFPublisher::BuildTFTree()
{
	// Get all objects with TF tags
	auto ObjToTagData = FTagStatics::GetObjectsToKeyValuePairs(GetWorld(), TEXT("TF"));

	// Try adding objects to the tf tree until no more objects in the map
	// and the map size has changed
	bool bMapSizeChanged = true;
	while (ObjToTagData.Num() > 0 && bMapSizeChanged)
	{
		int32 MapSize = ObjToTagData.Num();
		// Iterate map and try adding objects to tree
		for (auto MapItr(ObjToTagData.CreateIterator()); MapItr; ++MapItr)
		{
			// Frame Ids default values
			FString ChildFrameId = MapItr->Key->GetName();
			FString ParentFrameId = TEXT("World");

			// Set child frame id from tag
			if (MapItr->Value.Contains(TEXT("ChildFrameId")))
			{
				ChildFrameId = MapItr->Value["ChildFrameId"];
			}
			// Set parent frame id from tag
			if (MapItr->Value.Contains(TEXT("ParentFrameId")))
			{
				ParentFrameId = MapItr->Value["ParentFrameId"];
			}

			// Try to add node to tree
			if (TFWWorldTree->AddNode(MapItr->Key, ChildFrameId, ParentFrameId))
			{
				UE_LOG(LogTF, Warning, TEXT(" \t Added %s to %s \n"),
					*ChildFrameId, *ParentFrameId);
				MapItr.RemoveCurrent();
			}
		}

		// Check if the map size has changed
		if (MapSize == ObjToTagData.Num())
		{
			bMapSizeChanged = false;
		}
	}

	UE_LOG(LogTF, Warning, TEXT(" Current TF trees: \n %s \n "), *TFWWorldTree->ToString());
	UE_LOG(LogTF, Error, TEXT("%i could not be added, will be added as separate nodes with world parents: "),
		ObjToTagData.Num());
	
	for (auto MapItr(ObjToTagData.CreateIterator()); MapItr; ++MapItr)
	{
		UE_LOG(LogTF, Warning, TEXT(" \t %s's tags:"), *MapItr->Key->GetName());
		for (const auto& KeyValItr : MapItr->Value)
		{
			UE_LOG(LogTF, Warning, TEXT(" \t \t %s - %s:"), *KeyValItr.Key, *KeyValItr.Value);
		}
		UE_LOG(LogTF, Warning, TEXT(" ---- \n"), *MapItr->Key->GetName());

		// Frame Ids default values
		FString ChildFrameId = MapItr->Key->GetName();
		// Parent will be forced as World
		const FString ParentFrameId = TEXT("World");

		// Set child frame id from tag
		if (MapItr->Value.Contains(TEXT("ChildFrameId")))
		{
			ChildFrameId = MapItr->Value["ChildFrameId"];
		}

		if (TFWWorldTree->AddNode(MapItr->Key, ChildFrameId, ParentFrameId))
		{
			MapItr.RemoveCurrent();
		}
	}
	TFWWorldTree->GetNodesAsArray(TFNodes);
}

// Publish tf tree
void ATFPublisher::PublishTF()
{
	// Current time as ROS time
	FROSTime TimeNow = FROSTime::Now();
		
	// Create TFMessage
	TSharedPtr<tf2_msgs::TFMessage> TFMsgPtr =
		MakeShareable(new tf2_msgs::TFMessage());

	// Iterate TF nodes, generate and add StampedTransform msgs to TFMessage
	for (const auto& NodeItr : TFNodes)
	{
		TFMsgPtr->AddTransform(TFNodeToMsg(NodeItr, TimeNow));
		//UE_LOG(LogTF, Warning, TEXT(" %s \n "), *TFNodeToMsg(NodeItr, TimeNow).ToString());
	}
	UE_LOG(LogTF, Warning, TEXT(" %s \n "), *TFMsgPtr->ToString());
	
	// PUB
	ROSBridgeHandler->PublishMsg("/tf", TFMsgPtr);

	ROSBridgeHandler->Render();

	// Update message sequence
	Seq++;
}

// TFNode to tf2_msgs::TFMessage
geometry_msgs::TransformStamped ATFPublisher::TFNodeToMsg(UTFNode* InNode, const FROSTime InTime)
{
		geometry_msgs::TransformStamped StampedTransformMsg;

		std_msgs::Header Header;
		Header.SetSeq(Seq);
		const FString ParentFrameId = InNode->GetParent() != nullptr ?
			InNode->GetParent()->GetFrameId() : TEXT("World");
		Header.SetFrameId(ParentFrameId);
		Header.SetStamp(InTime);

		FTransform ROSTransf = FCoordConvStatics::UToROS(InNode->GetRelativeTransform());

		geometry_msgs::Transform TransfMsg(
			geometry_msgs::Vector3(ROSTransf.GetLocation()),
			geometry_msgs::Quaternion(ROSTransf.GetRotation()));

		StampedTransformMsg.SetHeader(Header);
		StampedTransformMsg.SetChildFrameId(InNode->GetFrameId());
		StampedTransformMsg.SetTransform(TransfMsg);

	return StampedTransformMsg;
}
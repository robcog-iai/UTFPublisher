// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFPublisher.h"
#include "TagStatics.h"
#include "CoordConvStatics.h"
//#include "FTFTree.h"
#include "tf2_msgs/TFMessage.h"

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

	//// Create the ROSBridge handler for connecting with ROS
	//ROSBridgeHandler = MakeShareable<FROSBridgeHandler>(
	//	new FROSBridgeHandler(ServerIP, ServerPORT));
	//
	//// Create the publisher
	//TFPublisher = MakeShareable<FROSBridgePublisher>(
	//	new FROSBridgePublisher("tf2_msgs/TFMessage", "/tf"));

	//// Add publisher
	//ROSBridgeHandler->AddPublisher(TFPublisher);

	//// Connect to ROS
	//ROSBridgeHandler->Connect();

	//// Bind publish function to timer
	//if (bUseStaticPublishRate)
	//{
	//	if (StaticPublishRate > 0.f)
	//	{
	//		// Disable tick
	//		SetActorTickEnabled(false);
	//		// Setup timer
	//		GetWorldTimerManager().SetTimer(TFPubTimer, this, &ATFPublisher::PublishTF, StaticPublishRate, true);
	//	}
	//}
	//else
	//{
	//	// Take into account the PublishRate Tag key value pair (if missing, publish on tick)
	//}
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

	//for (auto MapItr(ObjToTagData.CreateIterator()); MapItr; ++MapItr)
	//{
	//	UE_LOG(LogTF, Warning, TEXT(" \t %s's tags:"), *MapItr->Key->GetName());
	//	for (const auto& KeyValItr : MapItr->Value)
	//	{
	//		UE_LOG(LogTF, Warning, TEXT(" \t \t %s - %s:"), *KeyValItr.Key, *KeyValItr.Value);
	//	}
	//	UE_LOG(LogTF, Warning, TEXT(" ---- \n"), *MapItr->Key->GetName());
	//}

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

	UE_LOG(LogTF, Error, TEXT("%i items have no parents, adding them as separate root tf trees: "),
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
			UE_LOG(LogTF, Warning, TEXT(" \t %s - Successfully added to parent: %s \n\n"),
				*ChildFrameId, *ParentFrameId);
			MapItr.RemoveCurrent();
		}
	}

	UE_LOG(LogTF, Error, TEXT("%i items could not pe added to the tf trees"),
		ObjToTagData.Num());

	TFNodes = TFWWorldTree->GetNodesAsArray();
}

// Publish tf tree
void ATFPublisher::PublishTF()
{
	UE_LOG(LogTF, Warning, TEXT(" \t\tSTART TREES --> "));
	UE_LOG(LogTF, Warning, TEXT(" %s \n "), *TFWWorldTree->ToString());
	UE_LOG(LogTF, Warning, TEXT(" \t\t AS ARRAY : "));
	for (const auto& NodeItr : TFNodes)
	{
		UE_LOG(LogTF, Warning, TEXT(" %s \n \t "), *NodeItr->ToString());
	}
	UE_LOG(LogTF, Warning, TEXT(" \t\t <-- END TREES *** "));

	//// Current time as ROS time
	//FROSTime TimeNow = FROSTime::Now();

	//TSharedPtr<tf2_msgs::TFMessage> NsTFMsgPtr =
	//	MakeShareable(new tf2_msgs::TFMessage());

	//for (uint32 i = 0; i < NrOfTFMsgTEST; i++)
	//{
	//	geometry_msgs::TransformStamped StampedTransformMsg;

	//	std_msgs::Header Header;
	//	Header.SetSeq(Seq);
	//	Header.SetFrameId(TEXT("World"));
	//	Header.SetStamp(TimeNow);

	//	geometry_msgs::Transform TransfMsg(
	//		geometry_msgs::Vector3(
	//			FMath::RandRange(-0.1f, 0.1f) + (0.2*i),
	//			FMath::RandRange(-0.2f, 0.2f) + (0.2*i),
	//			FMath::RandRange(-0.1f, 0.1f) + (0.2*i)),
	//		geometry_msgs::Quaternion(FRotator(
	//			FMath::RandRange(-10.f, 10.f) + (0.2*i),
	//			FMath::RandRange(-10.f, 10.f) + (0.2*i),
	//			FMath::RandRange(-10.f, 10.f) + (0.2*i)).Quaternion()));

	//	StampedTransformMsg.SetHeader(Header);
	//	StampedTransformMsg.SetChildFrameId(FString("child_").Append(FString::FromInt(i)));
	//	StampedTransformMsg.SetTransform(TransfMsg);

	//	NsTFMsgPtr->AddTransform(StampedTransformMsg);
	//}

	//// PUB
	//ROSBridgeHandler->PublishMsg("/tf", NsTFMsgPtr);


	//ROSBridgeHandler->Render();

	//// Update message sequence
	//Seq++;
}

void ATFPublisher::OnActorDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogTF, Warning, TEXT(" Actor destroyed !!! "));
}
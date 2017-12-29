// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "TFPublisher.h"
#include "TagStatics.h"
#include "CoordConvStatics.h"
#include "FTFTree.h"
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

	// Build the tf tree
	BuildTFTree();
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

// Build the tf tree
void ATFPublisher::BuildTFTree()
{
	// Get all objects with TF tags
	auto ObjToTagData = FTagStatics::GetObjectsToKeyValuePairs(GetWorld(), TEXT("TF"));

	for (auto MapItr(ObjToTagData.CreateIterator()); MapItr; ++MapItr)
	{
		UE_LOG(LogTemp, Warning, TEXT(" \t Curr object name: %s"), *MapItr->Key->GetName());
		UE_LOG(LogTemp, Warning, TEXT(" \t \t NR Key Val pairs: %i"), MapItr->Value.Num());

		for (const auto& TagItr : MapItr->Value)
		{
			UE_LOG(LogTemp, Warning, TEXT(" \t \t \t Key %s , Val %s"), *TagItr.Key, *TagItr.Value);
		}

		MapItr.RemoveCurrent();
	}



	UE_LOG(LogTemp, Warning, TEXT(" \t\t *** START *** "));
	UE_LOG(LogTemp, Warning, TEXT(" TF tree size: %i"), TFTree.CountElements());
	UE_LOG(LogTemp, Warning, TEXT(" Total tf data objects: %i"), ObjToTagData.Num());
	
	// Add root nodes to tf tree (nodes with parent id "World", or no parent id)
	for (const auto& MapItr : ObjToTagData)
	{
		UE_LOG(LogTemp, Warning, TEXT(" \t Curr object name: %s"), *MapItr.Key->GetName());
		UE_LOG(LogTemp, Warning, TEXT(" \t \t NR Key Val pairs: %i"), MapItr.Value.Num());

		if (MapItr.Value.Contains(TEXT("ChildFrameId")))
		{
			// Save the child frame id data
			const FString ChildFrameId = MapItr.Value["ChildFrameId"];

			UE_LOG(LogTemp, Warning, TEXT(" \t \t ChildFrameId: %s"), *ChildFrameId);

			if (MapItr.Value.Contains(TEXT("ParentFrameId")))
			{
				// Save the parent frame id
				const FString ParentFrameId = MapItr.Value["ParentFrameId"];

				UE_LOG(LogTemp, Warning, TEXT(" \t \t ParentFrameId: %s"), *ParentFrameId);

				// Check if it is of type "World"
				if (ParentFrameId.Equals(TEXT("World")))
				{
					UE_LOG(LogTemp, Warning, TEXT(" \t \t ParentFrameId: %s equals World"), *ParentFrameId);
					// Add as tf tree root
					if (TFTree.AddNodeAt(FTFTreeNode(FTFData(MapItr.Key, ChildFrameId)), TEXT("World")))
					{
						UE_LOG(LogTemp, Warning, TEXT(" \t \t \t Successfully added root node (explicit world)"));
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT(" \t \t \t Could not add root node (explicit world)"));
					}
					//ObjToTagData.Remove(MapItr.Key);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT(" \t \t ParentFrameId: %s DOES NOT equal World"), *ParentFrameId);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT(" \t \t NO ParentFrameId: --- "));

				// No parent frame, take default as "World"
				// Add as tf tree root
				if (TFTree.AddNodeAt(FTFTreeNode(FTFData(MapItr.Key, ChildFrameId)), TEXT("World")))
				{
					UE_LOG(LogTemp, Warning, TEXT(" \t \t \t Successfully added root node (implicit world)"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT(" \t \t \t Could not add root node (implicit world)"));
				}
				//ObjToTagData.Remove(MapItr.Key);
			}
		}
		else
		{
			// Invalid TF tag, remove from map
			UE_LOG(LogTemp, Error, TEXT(" \t \t NO ChildFrameId: ---"));
			//ObjToTagData.Remove(MapItr.Key);
		}
		UE_LOG(LogTemp, Warning, TEXT(" ** "));
	}

	UE_LOG(LogTemp, Warning, TEXT(" \t\t *** End *** "));
	UE_LOG(LogTemp, Warning, TEXT(" TF tree size: %i"), TFTree.CountElements());
	UE_LOG(LogTemp, Warning, TEXT(" Total tf data objects: %i"), ObjToTagData.Num());
}

// Publish tf tree
void ATFPublisher::PublishTF()
{
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

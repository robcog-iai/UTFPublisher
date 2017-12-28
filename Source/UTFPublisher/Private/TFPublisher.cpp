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
	ROSBridgeHandler->Disconnect();

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
	TMap<AActor*, TMap<FString, FString>> TFActorMap =
		FTagStatics::GetActorsToKeyValuePairs(GetWorld(), "TF");

	FTFTreeNode Tree;

	TMultiMap<FString, UObject*> ParentFrameToObj;

	/*Tree.Init()*/

	TMap<UActorComponent*, TMap<FString, FString>> TFCompMap = FTagStatics::GetComponentsToKeyValuePairs(GetWorld(), "TF");
}

// Publish tf tree
void ATFPublisher::PublishTF()
{
	FROSTime TimeNow = FROSTime::Now();

	//// TF
	//TSharedPtr<FROSBridgeMsgTF2msgsTFMessage> TFMsgPtr =
	//	MakeShareable(new FROSBridgeMsgTF2msgsTFMessage());

	//for (uint32 i = 0; i < NrOfTFMsgTEST; i++)
	//{
	//	FROSBridgeMsgGeometrymsgsTransformStamped StampedTransformMsg;

	//	FROSBridgeMsgStdmsgsHeader Header;
	//	Header.SetSeq(Seq);
	//	Header.SetFrameId(TEXT("world"));
	//	Header.SetStamp(TimeNow);

	//	FROSBridgeMsgGeometrymsgsTransform TransfMsg(
	//		FROSBridgeMsgGeometrymsgsVector3(
	//			FMath::RandRange(-0.1f, 0.1f) + (0.2*i),
	//			FMath::RandRange(-0.2f, 0.2f) + (0.2*i),
	//			FMath::RandRange(-0.1f, 0.1f) + (0.2*i)),
	//		FROSBridgeMsgGeometrymsgsQuaternion(FRotator(
	//			FMath::RandRange(-10.f, 10.f) + (0.2*i),
	//			FMath::RandRange(-10.f, 10.f) + (0.2*i),
	//			FMath::RandRange(-10.f, 10.f) + (0.2*i)).Quaternion()));

	//	StampedTransformMsg.SetHeader(Header);
	//	StampedTransformMsg.SetChildFrameId(FString("child_").Append(FString::FromInt(i)));
	//	StampedTransformMsg.SetTransform(TransfMsg);

	//	TFMsgPtr->AddTransform(StampedTransformMsg);
	//}

	//// PUB
	//ROSBridgeHandler->PublishMsg("/tf", TFMsgPtr);


	/////////////////////////////////////////////////////////////////
	// NS version
	TSharedPtr<tf2_msgs::TFMessage> NsTFMsgPtr =
		MakeShareable(new tf2_msgs::TFMessage());

	for (uint32 i = 0; i < NrOfTFMsgTEST; i++)
	{
		geometry_msgs::TransformStamped StampedTransformMsg;

		std_msgs::Header Header;
		Header.SetSeq(Seq);
		Header.SetFrameId(TEXT("world"));
		Header.SetStamp(TimeNow);

		geometry_msgs::Transform TransfMsg(
			geometry_msgs::Vector3(
				FMath::RandRange(-0.1f, 0.1f) + (0.2*i),
				FMath::RandRange(-0.2f, 0.2f) + (0.2*i),
				FMath::RandRange(-0.1f, 0.1f) + (0.2*i)),
			geometry_msgs::Quaternion(FRotator(
				FMath::RandRange(-10.f, 10.f) + (0.2*i),
				FMath::RandRange(-10.f, 10.f) + (0.2*i),
				FMath::RandRange(-10.f, 10.f) + (0.2*i)).Quaternion()));

		StampedTransformMsg.SetHeader(Header);
		StampedTransformMsg.SetChildFrameId(FString("child_").Append(FString::FromInt(i)));
		StampedTransformMsg.SetTransform(TransfMsg);

		NsTFMsgPtr->AddTransform(StampedTransformMsg);
	}

	// PUB
	ROSBridgeHandler->PublishMsg("/tf", NsTFMsgPtr);

	/////////////////////////////////////////////////////////////////

	ROSBridgeHandler->Render2();

	// Update message sequence
	Seq++;
}

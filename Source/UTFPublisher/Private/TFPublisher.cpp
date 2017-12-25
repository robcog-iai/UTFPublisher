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
	bUseCustomUpdateRate = false;

	// Default timer delta time (s)
	PublishRate = 0.05f;

	// ROSBridge server default values
	ServerIP = "127.0.0.1";
	ServerPORT = 9090;

	FTFTree A;
}

// Called when the game starts or when spawned
void ATFPublisher::BeginPlay()
{
	Super::BeginPlay();

	// Create the ROSBridge handler for connecting with ROS
	ROSBridgeHandler = MakeShareable<FROSBridgeHandler>(
		new FROSBridgeHandler(ServerIP, ServerPORT));
	
	// Create the publisher
	ROSBridgePublisher = MakeShareable<FROSBridgePublisher>(
		new FROSBridgePublisher("geometry_msgs/TransformStamped", "/tf"));

	// Connect to ROS
	ROSBridgeHandler->Connect();

	// Bind publish function to timer
	if (bUseCustomUpdateRate)
	{
		// Disable tick
		SetActorTickEnabled(false);

		// Setup timer
		GetWorldTimerManager().SetTimer(TFPubTimer, this, &ATFPublisher::PublishTF, PublishRate, true);
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
	FTagStatics::GetActorsToKeyValuePairs(GetWorld(), "TF");
}

// Publish tf tree
void ATFPublisher::PublishTF()
{
	//UE_LOG(LogTemp, Warning, TEXT("Pub"));

	TSharedPtr<FROSBridgeMsgGeometrymsgsTransformStamped> StampedTransformMsg =
		MakeShareable(new FROSBridgeMsgGeometrymsgsTransformStamped());





	TSharedPtr<FROSBridgeMsgGeometrymsgsTransform> TrsanformMsg =
		MakeShareable(new FROSBridgeMsgGeometrymsgsTransform());

	TSharedPtr<FROSBridgeMsgGeometrymsgsVector3> TranslationMsg =
		MakeShareable(new FROSBridgeMsgGeometrymsgsVector3());
	FROSBridgeMsgGeometrymsgsVector3 TM(1,2,3);

	TSharedPtr<FROSBridgeMsgGeometrymsgsQuaternion> RotationMsg =
		MakeShareable(new FROSBridgeMsgGeometrymsgsQuaternion());


	
	TranslationMsg->SetVector(FVector(0));
	RotationMsg->SetQuat(FQuat(FQuat::Identity));

	TrsanformMsg->SetTranslation(TM);
	TrsanformMsg->SetTranslation(FROSBridgeMsgGeometrymsgsVector3(0.1f, 0.2f, 0.3f));
	TrsanformMsg->SetTranslation(*TranslationMsg.Get()); // TODO  PTSharedPtr<FROSBridgeMsg> InMsg
	TrsanformMsg->SetRotation(*RotationMsg.Get()); // TODO  PTSharedPtr<FROSBridgeMsg> InMsg

	StampedTransformMsg->SetTransform(*TrsanformMsg.Get());
	//StampedTransformMsg->SetHeader(FROSBridgeMsgStdmsgsHeader();

	ROSBridgeHandler->PublishMsg("/tf", StampedTransformMsg);

	ROSBridgeHandler->Render();
}

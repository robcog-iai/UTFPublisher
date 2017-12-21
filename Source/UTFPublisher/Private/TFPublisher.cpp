// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#include "TFPublisher.h"
#include "TagStatics.h"
#include "CoordConvStatics.h"


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
		UE_LOG(LogTemp, Warning, TEXT("SET TIMER"));
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
	ROSBridgeHandler->Render();
}

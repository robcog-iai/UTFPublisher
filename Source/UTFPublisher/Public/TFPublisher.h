// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgePublisher.h"
#include "Math/Vector4.h"
#include "FTFTree.h"
#include "TFPublisher.generated.h"

UCLASS()
class UTFPUBLISHER_API ATFPublisher : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATFPublisher();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when destroyed or game stopped
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// ROSBridge server IP
	UPROPERTY(EditAnywhere, Category = TF)
	FString ServerIP;

	// ROSBridge server PORT
	UPROPERTY(EditAnywhere, Category = TF, meta = (ClampMin = 0, ClampMax = 65535))
	int32 ServerPORT;

	// Choose between dynamic (various publish rates for the frames) 
	// or static publish rates (all frames updated at the same time)
	UPROPERTY(EditAnywhere, Category = TF)
	bool bUseStaticPublishRate;

	// Delta time (s) between publishing (0 = on Tick)
	UPROPERTY(EditAnywhere, Category = TF, meta = (editcondition = "bUseStaticPublishRate", ClampMin = "0.0"))
	float StaticPublishRate;
	
	// TODO RM TEST
	// Delta time between timer publishing (s)
	UPROPERTY(EditAnywhere, Category = TF)
	uint32 NrOfTFMsgTEST;
	
private:
	// Build the tf tree
	void BuildTFTree();

	// Publish tf tree
	void PublishTF();

	// ROSBridge handler for ROS connection
	TSharedPtr<FROSBridgeHandler> ROSBridgeHandler;

	// ROSPublisher for publishing TF
	TSharedPtr<FROSBridgePublisher> TFPublisher;

	// Publisher timer handle (in case of custom publish rate)
	FTimerHandle TFPubTimer;

	// TFTree
	FTFTree TFTree;

	// TF header sequence
	uint32 Seq;
};

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

	// Check if the update rate should be custom or on every tick
	UPROPERTY(EditAnywhere, Category = TF)
	bool bUseCustomUpdateRate;

	// Delta time between timer publishing (s)
	UPROPERTY(EditAnywhere, Category = TF, meta = (editcondition = "bUseCustomUpdateRate", ClampMin = "0.0"))
	float PublishRate;
	
private:
	// Build the tf tree
	void BuildTFTree();

	// Publish tf tree
	void PublishTF();

	// ROSBridge handler for ROS connection
	TSharedPtr<FROSBridgeHandler> ROSBridgeHandler;

	// ROSPublisher for publishing TF
	TSharedPtr<FROSBridgePublisher> ROSBridgePublisher;

	// Publisher timer handle (in case of custom publish rate)
	FTimerHandle TFPubTimer;

	// TFTree
	FTFTree TFTree;
};

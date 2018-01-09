// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#include "TFNodeComp.h"


// Sets default values for this component's properties
UTFNodeComp::UTFNodeComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTFNodeComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTFNodeComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


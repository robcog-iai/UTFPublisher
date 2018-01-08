// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "UObject/NoExportTypes.h"
#include "TFTree.h"
#include "TFWorldTree.generated.h"

/**
 * UTFWorldTree - TF trees connected to World, 
 *  (World does not have an FTransform, so it cannot be used as a TFNode)
 *  containing:
 *
 *  - array of pointers to tf trees
 *  - functionality to add tf tree nodes to the needed frames
 */
UCLASS()
class UTFPUBLISHER_API UTFWorldTree : public UObject
{
	GENERATED_BODY()
	
public:
	// Default constructor
	UTFWorldTree();

	// Default destructor
	virtual ~UTFWorldTree();

	// Set root frame name
	void SetRootFrameName(const FString& InRootFrameName);

	// Create and add node to the world tf tree
	bool AddNode(UObject* InObject, const FString& InFrameId, const FString& InParentFrameId = TEXT("World"));
	
	// Get all tf nodes as array (array will be emptied first)
	void GetNodesAsArray(TArray<UTFNode*>& OutNodes) const;

	// String output of the tf trees
	FString ToString() const;

private:
	// Create and add new tree
	void AddTree(UTFNode* InRoot);

	// Find and remove node from tree
	void RemoveNode(UTFNode* InNode);

	// Remove root node (generate new trees from the children)
	void RemoveRootNode(UTFTree* InTFTree);

	// Array of UTFTrees (since world is not a node)
	UPROPERTY() // avoid GC
	TArray<UTFTree*> TFTrees;

public:
	// Root frame name
	FString RootFrameName;

	// Array of all TF Nodes (convenient iteration)
	UPROPERTY() // avoid GC
	TArray<UTFNode*> TFNodesAsArray;

	// Give access to private data
	friend class UTFTree;
	friend class UTFNode;
};

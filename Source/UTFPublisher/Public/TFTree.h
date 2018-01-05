// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UTFPublisher.h" // CoreMinimal, TFLog
#include "UObject/NoExportTypes.h"
#include "TFNode.h"
#include "TFTree.generated.h"

/**
 * UTFNode - TF tree, containing:
 *
 *  - pointer to the root node
 *  - size of the tree
 */
UCLASS()
class UTFPUBLISHER_API UTFTree : public UObject
{
	GENERATED_BODY()

public:
	// Default constructor
	UTFTree();

	// Default destructor
	virtual ~UTFTree();

	// Find Node
	UTFNode* FindNode(const FString& InFrameId);

	// Add root
	bool AddRoot(UTFNode* InNode);

	// Add Node at the given frame
	bool AddNodeAt(UTFNode* InNode, const FString& InFrameId);

	// Remove only the node, connect its children to its parent
	bool RemoveNode(UTFNode* InNode);

	// Remove node with its branch
	bool RemoveBranch(UTFNode* InRoot);

	// Get all tf nodes as array
	void AddNodesToArray(TArray<UTFNode*>& OutNodes) const;

	// String output of the tf tree
	FString ToString() const;

private:
	// Root node
	UTFNode* Root;
};

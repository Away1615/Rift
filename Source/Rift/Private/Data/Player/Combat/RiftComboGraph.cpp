#include "Data/Player/Combat/RiftComboGraph.h"

const FRiftComboNode* URiftComboGraph::FindNode(const FName SectionName) const
{
	for (const FRiftComboNode& Node : Nodes)
	{
		if (Node.SectionName == SectionName)
		{
			return &Node;
		}
	}

	return nullptr;
}

FName URiftComboGraph::GetEntrySection(const FGameplayTag& InputTag) const
{
	for (const FRiftComboEntry& Entry : Entries)
	{
		if (Entry.InputTag == InputTag)
		{
			return Entry.EntrySection;
		}
	}

	return NAME_None;
}

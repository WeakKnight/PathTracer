#include "node.h"
#include "lightcomponent.h"

void Node::SetLight(LightComponent* com)
{
	if (com != nullptr)
	{
		com->parent = this;
	}
	light = com;
}
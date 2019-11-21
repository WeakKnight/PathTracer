#include "node.h"
#include "lightcomponent.h"

void Node::SetLight(LightComponent* com)
{
	com->parent = this;
	light = com;
}
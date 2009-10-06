/**
 * File: edge.cpp - Edge class implementation, part of
 * Graph library, internal representation of graphs in MiptVis tool.
 * Copyright (C) 2009  Boris Shurygin
 */
#include "graph_impl.h"

/**
 * Edge destructor.
 * delete edge from graph's list of edges
 */

Edge::~Edge()
{
    graph->detachEdge( this);
}

/**
 * Print edge in DOT format to stdout
 */

void
Edge::debugPrint()
{
    /**
     * Check that edge is printable
     * TODO: Implements graph states and in 'in process' state print node as '?'
     *       Examples of such prints: 4->? ?->3 ?->?
     */
    graphassert( isNotNullP( pred()));
    graphassert( isNotNullP( succ()));

    out("%u->%u;", pred()->id(), succ()->id());
}

/**
 * Low level correction of node's edge list in corresponding direction
 */

void
Edge::detachFromNode( GraphDir dir)
{
    Node *n = getNode( dir);
    n->deleteEdgeInDir( revDir( dir), &n_it[ dir]);
}

/**
 *  Read edge points
 */
void
Edge::readEdgePointsFromXMLDoc( xmlNode * a_node)
{
	xmlNode * cur_node;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if ( cur_node->type == XML_ELEMENT_NODE
			 && xmlStrEqual( cur_node->name, xmlCharStrdup("point")))
		{
			xmlAttr * props;
			int n = -1;
			for( props = cur_node->properties; props; props = props->next)
			{
				if ( xmlStrEqual( props->name, xmlCharStrdup("n")))
				{
					n = strtoul( ( const char *)( props->children->content), NULL, 0);
				}
			}

			if ( n == -1) continue;

			setPoint( new EdgePoint, n);

			for( props = cur_node->properties; props; props = props->next)
			{
				if ( xmlStrEqual( props->name, xmlCharStrdup("x")))
				{
					point( n)->x = strtoul( ( const char *)( props->children->content), NULL, 0);
				} else if ( xmlStrEqual( props->name, xmlCharStrdup("y")))
				{
					point( n)->y = strtoul( ( const char *)( props->children->content), NULL, 0);
				}
			}
		}
	}

}

/**
 * Write edge points by xml writer
 */
void
Edge::writePointsByXMLWriter( xmlTextWriterPtr writer)
{
	int i;
	for ( i = 1; i <= pointsNum(); i++)
	{
		xmlTextWriterWriteString( writer, BAD_CAST "\t\t");
		xmlTextWriterStartElement( writer, BAD_CAST "point");

		xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "n", "%d", i);
		xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "x", "%d", point(i)->x);
		xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "y", "%d", point(i)->y);
		
		xmlTextWriterEndElement( writer);
		xmlTextWriterWriteString( writer, BAD_CAST "\n");
	}
}

/**
 * Write edge by xml writer
 */
void
Edge::writeByXMLWriter( xmlTextWriterPtr writer)
{
	xmlTextWriterWriteString( writer, BAD_CAST "\t");
	xmlTextWriterStartElement( writer, BAD_CAST "edge");

	xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "id", "%d", id());
	xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "from", "%d", pred()->id());
	xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "to", "%d", succ()->id());
	xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "points_num", "%d", pointsNum());
	xmlTextWriterWriteAttribute( writer, BAD_CAST "label", BAD_CAST label());
	xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "prob", "%d", prob());
	xmlTextWriterWriteFormatAttribute( writer, BAD_CAST "thickness", "%d", thickness());
	xmlTextWriterWriteAttribute( writer, BAD_CAST "color", BAD_CAST color());
	xmlTextWriterWriteAttribute( writer, BAD_CAST "style", BAD_CAST style());

    xmlTextWriterWriteString( writer, BAD_CAST "\n");
	writePointsByXMLWriter( writer);

	xmlTextWriterWriteString( writer, BAD_CAST "\t");
	xmlTextWriterEndElement( writer);
    xmlTextWriterWriteString( writer, BAD_CAST "\n");
}
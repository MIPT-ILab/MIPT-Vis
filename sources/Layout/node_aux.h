#ifndef NODE_AUX_H
#define NODE_AUX_H

/**
 * Representation of various auxiliary node attributes
 */
class NodePropertiesAux
{
    int rang_priv; // number of node's layer
    int pos_priv;  // absolute position in layer
public:
    // Default constructor
    NodePropertiesAux()
    {    
        NodeProperties::NodeProperties();
        // Default position is first in first layer
        rang_priv = 1;
        pos_priv = 1;
    }
    // Gets for auxiliary properties
    inline int rang() const
    {
        return rang_priv;
    }
    inline int pos() const
    {
        return pos_priv;
    }
    // Sets for auxiliary properties
    inline void setRang( int rang)
    {
        rang_priv = rang;
    }
    inline void setPos( int pos)
    {
        pos_priv = pos;
    }
};

/**
 * Auxiliary node representation class. 
 */
class NodeAux: public Node, public NodePropertiesAux
{
protected:
    NodeAux( GraphAux *graph_p, int _id);
    friend class GraphAux;
public:
     /**
     * Get node's corresponding auxiliary graph
     */
    inline GraphAux * getGraph() const;

    /**
       * Add an edge to this node in specified direction
     */
    void addEdgeInDir( EdgeAux *edge, GraphDir dir)
    {
        Node::addEdgeInDir( static_cast< Edge*> (edge), dir);
    }

    /**
     * Add predecessor edge
     */
    inline void addPred( EdgeAux *edge)
    {
        addEdgeInDir( edge, GRAPH_DIR_UP);
    }

    /**
     * Add successor edge
     */
    inline void addSucc( EdgeAux *edge) 
    {
        addEdgeInDir( edge, GRAPH_DIR_DOWN);
    }

     /**
     *  Iteration through edges routines.
     *
     *  Set iterator to beginning of edge list and return first edge
     */
    inline EdgeAux* firstEdgeInDir( GraphDir dir)
    {
        return static_cast< EdgeAux*>( Node::firstEdgeInDir( dir));
    }
    /**
     * Advance iterator and return next edge in specified direction
     * NOTE: If end of list is reached we return NULL for first time and fail if called once again
     */
    inline EdgeAux* nextEdgeInDir( GraphDir dir)
    {
        return static_cast< EdgeAux*>( Node::nextEdgeInDir( dir));
    }

    /** 
     * Corresponding iterators for successors/predeccessors.
     * NOTE: See firstEdgeInDir and other ...InDir routines for details
     */
    inline EdgeAux* firstSucc()
    {
        return firstEdgeInDir( GRAPH_DIR_DOWN);
    }
    inline EdgeAux* nextSucc()
    {
        return nextEdgeInDir( GRAPH_DIR_DOWN);
    }
    inline EdgeAux* firstPred()
    {
        return firstEdgeInDir( GRAPH_DIR_UP);
    }
    inline EdgeAux* nextPred()
    {
        return nextEdgeInDir( GRAPH_DIR_UP);
    }
};

#endif
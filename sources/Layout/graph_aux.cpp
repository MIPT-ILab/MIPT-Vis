/** 
* File: graph_aux.cpp - Auxiliary graph class implementation, part of
* layout library, internal graph placement component in MiptVis tool.
* Copyright (C) 2009 MiptVis
*/

#include <Qt/QQueue.h>
#include <Qt/QStack.h>
#include <math.h>
#include "layout_iface.h"


//-----------------------------------------------------------------------------
/**
* Ranking algorithm. Divide nodes to layers
*/
bool GraphAux::ranking()
{
	NodeAux* root = makeAcyclic();
	if (root == 0) return false;

	max_rank = 0;

	clearRanks();
	Marker passed = newMarker();
	rankImp (root, 0, passed);
	freeMarker (passed);
	
	return true;
}
//-----------------------------------------------------------------------------
/**
* Here must be an implementation of ordering in layers
*/
bool GraphAux::ordering()
{
	/** Add virtuals and initialize rank structure */
	addVirtualChains();
	rank.clear();
	rank.resize( max_rank + 1);
	for( NodeAux* iter = firstNode(); iter != NULL; iter = iter->nextNode())
	{
		rank[ iter->rang()].addNode( iter);
	}
	/** Initial ordering */
	Numeration dfs_num = newNum();
	DFS( dfs_num);
	setDfsNum( dfs_num);
	for( QVector<AdjRank>::iterator iter = rank.begin(); iter != rank.end(); iter++)
	{
		iter->sortByNum();
		iter->setInitX();
	}
	freeNum( dfs_num);
	/** Ordering */
	const int max_iter = 24;
	for( int i = 0; i < max_iter; i++)
	{
		for( int j = 1; j < max_rank; j++)
		{
			rank[j].doOrder( &rank[j - 1]);
		}
		for( int j = max_rank - 1; j > 0; j--)
		{
			rank[j].doOrder( &rank[j + 1]);
		}
	}
	return true;
}
//-----------------------------------------------------------------------------
/**
* Position. Includes vertical arangement, posiotion of nodes within layers depend on order.
*/
bool GraphAux::position()
{
	arrangeHorizontal();
	arrangeVertical();
	applayPositions();
	return true;
}
//-----------------------------------------------------------------------------
/**
* Here might be laying a edges.
*/
bool GraphAux::make_splines()
{
	return true;
}
//-----------------------------------------------------------------------------
/**
* The main function of our component
*/
bool GraphAux::doLayout()
{
	delVirtualNodes();
	if (!ranking())			return false;
	if (!ordering())		return false;
	if (!position())		return false;
	if (!make_splines())	return false;
	if (!applayLayout())	return false;
	
	debugPrint();

	return true;
}
//-----------------------------------------------------------------------------
qreal Lenght (QPoint& what)
{
	return sqrt(qreal (what.x()*what.x() + what.y()*what.y()));
}
qreal lenSq (QPoint& what)
{
	return (what.x()*what.x() + what.y()*what.y());
}
qreal lenSq (QPointF& what)
{
	return (what.x()*what.x() + what.y()*what.y());
}
/**
* The main function of our component
*/
QPointF attractForce (NodeAux* from, int frmass, NodeAux* to);
QPointF repulseForce (NodeAux* from, int frmass, NodeAux* to);
const qreal attrK = 1400;
const qreal repuK = 400000000000;
const qreal minDist = 50;
const qreal timestep = 2;
const qreal resistance = 0.7;
const qreal max_force = 1;
void GraphAux::iterateGravity()
{
	int m = 0;
//	for (NodeAux* iter = firstNode(); iter; iter = iter->nextNode())
//		iter->shedA();

	for (NodeAux* iter = firstNode(); iter; iter = iter->nextNode())
	{
		m = 2500;//iter->width()*iter->height();

		Edge* e;
		ForEdges(iter, e, Succ)
		{
			addAux(e->succ())->addA( attractForce( iter, m, addAux( e->succ())));
		}
		ForEdges(iter, e, Pred)
		{
			addAux(e->pred())->addA( attractForce( iter, m, addAux( e->pred())));
		}
		for (NodeAux* cur = firstNode(); cur; cur = cur->nextNode())
			if (iter != cur)
				cur->addA( repulseForce( iter, m, cur));
	}


	for (NodeAux* iter = firstNode(); iter; iter = iter->nextNode())
	{
		iter->applA (timestep, resistance);
		iter->applV (timestep);
		iter->shedA();
	}
	applayPositions();
}
QPointF attractForce (NodeAux* from, int frmass, NodeAux* to)
{
	QPoint dsti = from->dist (to->coor());
	QPointF dst(dsti.x(), dsti.y());
	qreal lensq = lenSq (dsti);
	if (lensq < minDist*minDist) lensq = minDist*minDist;

	QPointF f = frmass*attrK*dst/lensq/lensq;

	if (lenSq (f) > max_force) f *= max_force/sqrt (lenSq (f));

	return f;
}
QPointF repulseForce (NodeAux* from, int frmass, NodeAux* to)
{
	QPoint dsti = from->dist (to->coor());
	QPointF dst(dsti.x(), dsti.y());
	qreal lensq = lenSq (dsti);
	if (lensq < minDist*minDist) lensq = minDist*minDist;

	QPointF f = -frmass*repuK*dst/lensq/lensq/lensq/lensq;
	if (lenSq (f) > max_force) f *= max_force/sqrt (lenSq (f));

	return f;
}
//-----------------------------------------------------------------------------
/**
* Deletes virtual roots, translate nodes, returns false if there are any
* virtuals state in graph
*/
bool GraphAux::delVirtualNodes()
{
	bool rez = true;
	Node* to_detach = 0;
	for (Node* cur = firstNode(); cur != 0; cur = cur->nextNode())
	{
		if (to_detach)
		{
			removeNode (to_detach);
			//delete to_detach;
			to_detach = 0;
		}

		if (!addAux(cur)->real())
		{
			int num_preds = 0;
			int num_succs = 0;
			Edge* iter;
			ForEdges(cur, iter, Succ) ++num_succs;
			ForEdges(cur, iter, Pred) ++num_preds;

			if (num_preds == 0 && num_succs == 0) to_detach = cur; //lonely node
			else if (num_preds == 0)				//virtual root
			{
//				ForEdges(cur, iter, Succ) removeEdge (iter);
				to_detach = cur;
			}
			else if (num_preds == 1 && num_succs == 1)//translation node(edge point)
			{
				Edge* one = cur->firstSucc();
				Edge* two = cur->firstPred();
				one->setPred (two->pred());
//				removeEdge (two);
				to_detach = cur;
			}
			else rez = false;//it node can't be deleted
		}
	}
	if (to_detach)//for the last
	{
		removeNode (to_detach);
//		delete to_detach;
		to_detach = 0;
	}
	return rez;
}
//-----------------------------------------------------------------------------
/**
* Finding a node like root
*/
NodeAux* GraphAux::findRoot()
{
	QList <Node*> roots;
	for (Node* cur = firstNode(); cur != 0;cur = cur->nextNode())
		if (cur->firstPred() == 0)
			roots.push_back (cur);

	int num_reach = 0;
	Marker reachable = newMarker();
	foreach(Node* cur_root, roots)
	{
		num_reach += markReachable(cur_root, reachable);
	}
	while (num_reach != getNodeCount())//these roots are not grab all graph
	{
		for (Node* cur = firstNode(); cur != 0;cur = cur->nextNode())
			if (!cur->isMarked(reachable))
				if (cur->firstSucc() != 0 || cur->firstPred() == 0)//needs advanced improvement
				{
					roots.push_back (cur);
					break;
				}
		num_reach += markReachable (roots.last(), reachable);//!!!&&& do not forget to correct
	}
	freeMarker (reachable);

	if (roots.size() >  1) return makeVirtualRoot (roots);
	if (roots.size() == 1) return addAux (roots.back());
						   return 0; //it's impossible
}
//-----------------------------------------------------------------------------
/**
* Marks all nodes, reachable from root by by_what, and returns theis number
*/
int GraphAux::markReachable (Node* root, Marker by_what)
{
	if (root->isMarked (by_what)) return 0;
	root->mark (by_what);
	int number_marked = 0;
	QQueue<Node*> to_process;
	to_process.enqueue (root);

	while (to_process.size() != 0)
	{
		++number_marked;
		Node* cur = to_process.dequeue();
		for (Edge* csucc = cur->firstSucc(); csucc != 0; csucc = csucc->nextSucc())
			if (!csucc->succ()->isMarked (by_what))
			{
				csucc->succ()->mark (by_what);
				to_process.enqueue (csucc->succ());
			}
	}
	return number_marked;
}
//-----------------------------------------------------------------------------
/**
* Adds virtual node - predesessor of all roots, 
*/
NodeAux* GraphAux::makeVirtualRoot (QList <Node*>& roots)
{
	if (roots.size() < 1) return 0;

	NodeAux* root = newNode();
	root->setReal (false);

	foreach(Node* iter, roots)
	{
		newEdge (root, addAux(iter));
	}
	return root;
}
//-----------------------------------------------------------------------------
/**
* Make graph acyclic and set to all edges corresponding type
*/
NodeAux* GraphAux::makeAcyclic()
{
	NodeAux* first = findRoot();
	if (first == 0) return 0;
	
	Marker passed = newMarker();
	Marker ret = newMarker();

	QStack<NodeAux*> to_process;
	to_process.push (first);

	while (to_process.size() > 0)
	{
		NodeAux* from = to_process.pop();
		
		if (from->isMarked (passed))//the second pass of node
		{
			from->mark (ret);
			continue;
		}
		from->mark (passed);

		to_process.push(from);//Need to pass this repeatedly, to mark as returned

		for (EdgeAux* cur = from->firstSucc(); !from->endOfSuccs();
			          cur = from->nextSucc())
			if (cur->succ() == from) cur->type = EdgeAux::mesh; //mesh
			else
			{
				if (cur->succ()->isMarked (passed))
				{
					if (cur->succ()->isMarked (ret))
						 cur->type = EdgeAux::forward;			//forward
					else cur->type = EdgeAux::back;				//back
				}
				else
				{
					cur->type = EdgeAux::tree;					//tree
					to_process.push (cur->succ());//process childs
				}
			}
	}


	freeMarker (passed);
	freeMarker (ret);

	return first;
}
//-----------------------------------------------------------------------------
/*
 * Internal implementation, ranks nodes by the longest path for it
 */
void GraphAux::clearRanks()
{
	for (Node* iter = firstNode(); iter != 0; iter = iter->nextNode())
		addAux(iter)->setRang (0);
}
//-----------------------------------------------------------------------------
/*
 * Internal implementation, ranks nodes by the longest path for it
 */
void GraphAux::rankImp (NodeAux* from, int cur_rank, Marker passed)
{
	if (from->rang_priv <= cur_rank)//Choose maximal lenght
		from->rang_priv =  cur_rank;

	if (cur_rank > max_rank) max_rank = cur_rank;

	if (passedAllPred (from, passed))
		passAllSucc (from, from->rang_priv, passed);//Wait for pass all previous nodes
}
//-----------------------------------------------------------------------------
/*
 * Make ranking for all successors
 */
void GraphAux::passAllSucc (NodeAux* from, int cur_rank, Marker passed)
{
	EdgeAux* cur;

	ForEdges(from, cur, Succ)
	{
		if (cur->ahead())
		{
			cur->mark (passed);
			rankImp (addAux (cur->succ()), cur_rank + 1, passed);//climb to a max-lenght path
		}
	}	
	ForEdges(from, cur, Pred)
	{
		if (cur->backward())
		{
			cur->mark (passed);
			rankImp (addAux (cur->pred()), cur_rank + 1, passed);//climb to a max-lenght path
		}
	}
}
//-----------------------------------------------------------------------------
/*
 * returns true only when all "pred" edges of from are marked "passed"
 */
bool GraphAux::passedAllPred (NodeAux* from, Marker passed)
{
	EdgeAux* cur;

	ForEdges(from, cur, Succ)
		if (cur->backward() && !cur->isMarked (passed))
			return false;

	ForEdges(from, cur, Pred)
		if (cur->ahead() && !cur->isMarked (passed))
			return false;
	return true;//all incoming edges are processed
}
//-----------------------------------------------------------------------------
void GraphAux::addVirtualChains()
{
	bool added = true;
	while (added)// number of edges increases
	{
		added = false;
		for (EdgeAux* iter = firstEdge(); iter != NULL; iter = iter->nextEdge())
		{
			int rang_pred = iter->pred()->rang_priv, rang_succ = iter->succ()->rang_priv;
			if( abs(rang_pred - rang_succ) > 1)
			{
				added = true;
				NodeAux* n = insertNodeOnEdge( iter);
				n->setReal (false);
				n->setWidth(20);
				if( rang_pred > rang_succ)
					n->rang_priv = rang_pred - 1;
				else
					n->rang_priv = rang_pred + 1;
			}
		}
	}
}
//-----------------------------------------------------------------------------
/*
 * Arrnages nodes' vertical positions by their ranks
 */
void GraphAux::arrangeVertical()
{
	int starty = 0;
	int stepy = 80;

	for (NodeAux* iter = addAux(firstNode()); iter != 0; iter = addAux(iter->nextNode()))
	{
		iter->setY (iter->rang()*stepy + starty);
	}
}
//-----------------------------------------------------------------------------
/**
*
*/
void GraphAux::arrangeHorizontal()
{
	forceDirectedPosition();
	/* Set minimal coordinate to zero */
	int min_x = 1000000000;
	for (NodeAux* iter = addAux(firstNode()); iter != 0; iter = addAux(iter->nextNode()))
	{
		if( iter->x() < min_x) min_x = iter->x();
	}
	for (NodeAux* iter = addAux(firstNode()); iter != 0; iter = addAux(iter->nextNode()))
	{
		iter->setX( iter->x() - min_x);
	}
}
//-----------------------------------------------------------------------------
/**
* Force caused by attached spring. Calculates with Hooke's law.
*/
QPointF springForce( NodeAux* from, NodeAux* to, int k, int k1, int min_dist)
{
	QPointF r( to->coor() - from->coor());
	if( lenSq( r) < min_dist * min_dist) return r / sqrt( lenSq( r)) * k * ( lenSq( r) - min_dist);
	return r / sqrt( lenSq( r)) * (k1) * ( sqrt( lenSq( r)) - min_dist);
}
/**
* Save minimal distance between nodes by balancing forces.
*/
void GraphAux::saveMinDist( int rank_num)
{
	bool balanced = true;
	NodeAux* iter = rank[rank_num].first();
	int dir = 1;
	int min_dist;
	int offset = 50;
	float eps = 10; // Minimal difference between forces to avoid infinite balancing.
	do
	{
		balanced = true;
		for(iter = rank[rank_num].first();  iter->nextInLayer() != NULL; iter = iter->nextInLayer())
		{
			min_dist = iter->width() / 2 + offset + iter->nextInLayer()->width() / 2;
			if ((iter->nextInLayer()->x() - iter->x()) < min_dist) // Treat nodes only closer than min_dist
			{
				if ((iter->nextInLayer()->getA().x() - iter->getA().x()) < - eps) // Compare forces exerted on nodes
				{
					iter->setA((iter->getA() + iter->nextInLayer()->getA()) / 2); // Balance them
					iter->nextInLayer()->setA(iter->getA());
					balanced = false;
				}
			}
		}
	}
	while(!balanced);
}
/**
* Decompact nodes to have affordable minimal distance between them.
* Can be run in both directions to obtain symetry.
*/
void GraphAux::decompact( int rank_num, int dir)
{
	int offset = 49;
	if( dir == 1)
	{
		for( NodeAux* iter = rank[rank_num].first()->nextInLayer(); iter!=NULL; iter = iter->nextInLayer())
		{
			int min_dist = iter->prevInLayer()->width() / 2 + offset + iter->width() / 2;
			int dist = iter->x() -  iter->prevInLayer()->x();
			if( dist < min_dist)
			{
				iter->setX( iter->x() + min_dist - dist);
			}
		}
	}
	else
	{
		for( NodeAux* iter = rank[rank_num].last()->prevInLayer(); iter!=NULL; iter = iter->prevInLayer())
		{
			int min_dist = iter->width() / 2 + offset + iter->nextInLayer()->width() / 2;
			int dist = - iter->x() + iter->nextInLayer()->x();
			if( dist < min_dist)
			{
				iter->setX( iter->x() - min_dist + dist);
			}
		}
	}
}
/**
* Get resultant forces for all nodes of the layer
*/
void GraphAux::resultantForce( int rank_num)
{
	const int step = 30;
	for( NodeAux* iter = rank[rank_num].first(); iter != NULL; iter = iter->nextInLayer())
		{
			QPointF resultant(0,0);
			/**
			* Get attract forces from attached edges.
			* Too small coefficient may affect in too small displacement,
			* and too big coefficient may affect forces to be never balanced.
			*/
			for( EdgeAux* itere = iter->firstPred(); itere != NULL; itere = iter->nextPred())
			{
				resultant += springForce( iter, itere->pred(), 0, 10, 0);
			}
			for( EdgeAux* itere = iter->firstSucc(); itere != NULL; itere = iter->nextSucc())
			{
				resultant += springForce( iter, itere->succ(), 0, 10, 0);
			}
			/**
			* Get repulse force of adjacent nodes.
			* Both coefficient are zero, because saveMinDist and decompact methods are used instead.
			*/
			if( iter->nextInLayer() != NULL)
				resultant += springForce( iter, iter->nextInLayer(), 0, 0, 150);
			if( iter->prevInLayer() != NULL)
				resultant += springForce( iter, iter->prevInLayer(), 0, 0, 150);
			resultant.setY( 0);
			iter->addA( resultant);
		}
}
/**
* Force directed horizontal positioning algorithm. 
* This algorithm does not attempt to place edges vertically (at least not yet).
*/
void GraphAux::forceDirectedPosition()
{
	const int max_iter = 256;
	const int mass = 1;
	const QPointF max_force(5,0); ///Set maximum force to avoid too big displacement
	for( int i = 0; i < max_iter; i++)
	{
		for( int j = 0; j <= max_rank; j++)
		{
			resultantForce( j);
			saveMinDist( j);
		}
		for( int j = max_rank; j >= 0; j--)
		{
			resultantForce( j);
			saveMinDist( j);
		}
		for (NodeAux* iter = firstNode(); iter; iter = iter->nextNode())
		{
			/* Normalize force */
			iter->setA(iter->getA() / mass);
			if( abs( iter->getA().x()) > max_force.x()) iter->setA( iter->getA().x() / abs( iter->getA().x()) * max_force);
			/**
			* Apply forces.
			* Theese parameters strongly influence behaviour of the whole algorithm.
			* Unfortunately they may not be graph-independent.
			*/
			iter->applA( 1, 0.3);
			iter->applV( 1);
			iter->shedA();
		}
		for( int j = max_rank; j >= 0; j--)
		{
			decompact( j, i % 2);
		}
	}
}
//-----------------------------------------------------------------------------
void GraphAux::applayPositions()
{
	for (NodeAux* iter = addAux(firstNode()); iter != 0;
		          iter = addAux(iter->nextNode()))
	{
		iter->commitPos (iter->x(), iter->y());
		if (!iter->real())
			iter->superscribe (Qt::gray, "unreal");
	}
}
//-----------------------------------------------------------------------------